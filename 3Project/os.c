/**
 * File: os.c
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 02
 * 04/25/2014
 */

#include "os.h"
#include "globals.h"

volatile struct system_t *system;
volatile uint32_t tenMillisecondCounter = 0;
volatile uint32_t oneSecondCounter = 0;

void yield() {
   switchNextThread();
}

/**
 * Initializes the operating system. Reserves space from the heap for the
 * system data structure and initializes the system data structure.
 */
void os_init(void) {
   system = calloc(1, sizeof(struct system_t));
   system->currentThreadId = 0;
   system->numberOfThreads = 0;
   system->systemTime = getSystemTime();
}

void thread_sleep(uint16_t ticks) {
   system->threads[system->currentThreadId].state = THREAD_SLEEPING;
   system->threads[system->currentThreadId].sleepingTicksLeft = ticks;

   switchNextThread();
}

void mutex_init(struct mutex_t* m) {
   cli();
   m->ownerId = 0;
   m->lock = 0;
   m->startIndex = 0;
   m->endIndex = 0;
   sei();
}

void mutex_lock(struct mutex_t* m) {
   cli();
   if (!m->lock) {
      m->ownerId = system->currentThreadId;
      m->lock = 1;
   }
   else {
      m->waitingThreadsIds[m->endIndex] = system->currentThreadId; 
      m->endIndex = (m->endIndex + 1) % MAX_NUMBER_OF_THREADS;
      system->threads[system->currentThreadId].state = THREAD_WAITING;
      sei();
      switchNextThread();
      cli();
      //have question here
      mutex_lock(m);
   }
   sei();
}

void mutex_unlock(struct mutex_t* m) {
   cli();
   uint8_t nextThreadId = 0;

   if (m->ownerId == system->currentThreadId) {
      system->threads[m->waitingThreadsIds[m->startIndex]].state = THREAD_READY;
   
      nextThreadId = m->waitingThreadsIds[m->startIndex];

      m->startIndex = (m->startIndex + 1) % MAX_NUMBER_OF_THREADS;
      m->lock = 0;

      sei();
      switchThreads(nextThreadId);
      cli();
   }
   sei();
}

void sem_init(struct semaphore_t* s, int8_t value) {
   cli();
   s->value = value;
   s->startIndex = 0;
   s->endIndex = 0;
   sei();
}

void sem_wait(struct semaphore_t* s) {
   cli();
   if (s->value <= 0) {
      s->waitingThreadsIds[s->endIndex] = system->currentThreadId;
      s->endIndex = (s->endIndex + 1) % MAX_NUMBER_OF_THREADS;
      system->threads[system->currentThreadId].state = THREAD_WAITING;
      sei();
      switchNextThread();
      cli();
   }
   else {
      s->value--;
   }
   sei();
}

void sem_signal(struct semaphore_t* s) {
   cli();
   s->value++;

   if (s->startIndex != s->endIndex) {
      system->threads[s->waitingThreadsIds[s->startIndex]].state = THREAD_READY;
      s->startIndex = (s->startIndex + 1) % MAX_NUMBER_OF_THREADS;
   }

   sei();
}

void sem_signal_swap(struct semaphore_t* s) {
   cli();
   uint8_t nextThreadId = 0;

   s->value++;

   if (s->startIndex != s->endIndex) {
      nextThreadId = s->waitingThreadsIds[s->startIndex];
      system->threads[nextThreadId].state = THREAD_READY;
      s->startIndex = (s->startIndex + 1) % MAX_NUMBER_OF_THREADS;

      sei();
      switchNextThread(nextThreadId);
      cli();
   }
   sei(); 
}

/**
 * Adds a new thread to the system data structure. The new thread allocates
 * space for its stack, and sets its stack bounds, its stack pointer, and
 * its program counter.
 *
 * @param address The address the program counter will assume upon thread start
 * @param args A pointer to a list of arguments passed into the thread
 * @param stackSize The stack size in bytes available to the thread
 */
void create_thread(uint16_t address, void *args, uint16_t stackSize) {
   volatile struct thread_t *newThread =
    &system->threads[system->numberOfThreads];

   newThread->threadId = system->numberOfThreads++;

   newThread->stackSize = stackSize + sizeof(struct regs_interrupt) +
    sizeof(struct regs_context_switch);
   newThread->lowestStackAddress =
    malloc(newThread->stackSize * sizeof(uint8_t));
   newThread->highestStackAddress = newThread->lowestStackAddress +
    newThread->stackSize;
   newThread->stackPointer = newThread->highestStackAddress;
   newThread->functionAddress = address;
   newThread->state = THREAD_READY;
   newThread->sleepingTicksLeft = 0;

   struct regs_context_switch *registers =
    (struct regs_context_switch *) newThread->stackPointer - 1;

   // thread_start address low byte
   registers->pcl = 0x00FF & (uint16_t) thread_start;
   // thread_start address high byte
   registers->pch = 0x00FF & ((uint16_t) thread_start >> 8);
   registers->r2 = 0;
   registers->r3 = 0;
   registers->r4 = 0;
   registers->r5 = 0;
   registers->r6 = 0;
   registers->r7 = 0;
   registers->r8 = 0;
   registers->r9 = 0;
   registers->r10 = 0;
   registers->r11 = 0;
   registers->r12 = 0;
   registers->r13 = 0;
   // args address low byte
   registers->r14 = 0x00FF & (uint16_t) args; 
   // args address high byte
   registers->r15 = 0x00FF & ((uint16_t) args >> 8);
   // function address low byte
   registers->r16 = 0x00FF & address;
   // function address high byte
   registers->r17 = 0x00FF & (address >> 8);
   registers->r28 = 0;
   registers->r29 = 0;

   newThread->stackPointer = (uint8_t *) registers;

   return;
}

/**
 * Fetches the next thread to run and then invokes context-switching
 * between threads every 10 ms. The ISR is based off of the system
 * timer.
 */
ISR(TIMER0_COMPA_vect) {
   //The following statement tells GCC that it can use registers r18-r27, 
   //and r30-31 for this interrupt routine.  These registers (along with
   //r0 and r1) will automatically be pushed and popped by
   //this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");                        

   tenMillisecondCounter++;
   notifySleepingThreads();
   switchNextThread();
}

void notifySleepingThreads() {
   uint8_t i = 0;
   for (i = 0; i < system->numberOfThreads; i++) {
      if (system->threads[i].state == THREAD_SLEEPING) {
         system->threads[i].sleepingTicksLeft--;
         if (system->threads[i].sleepingTicksLeft == 0) {
            system->threads[i].state = THREAD_READY;
         }
      }  
   }
}

void switchNextThread() {
   sei();
   switchThreads(get_next_thread());
   cli();
}

//Maybe set sei() at the beginning
void switchThreads(uint8_t nextThreadId) {
   sei();
   uint8_t currentThreadId = system->currentThreadId;
   
   //if the current thread was interrupted and not changed 
   //into a waiting or sleeping state, change its state to THREAD_READY
   if (system->threads[system->currentThreadId].state == THREAD_RUNNING) {
      system->threads[system->currentThreadId].state = THREAD_READY;
   }
   system->currentThreadId = nextThreadId;

   system->threads[nextThreadId].state = THREAD_RUNNING;

   //Call context switch here to switch to that next thread
   context_switch((uint16_t *) &system->threads[nextThreadId].stackPointer,
    (uint16_t *) &system->threads[currentThreadId].stackPointer);

   cli();
}

ISR(TIMER1_COMPA_vect) {
   //This interrupt routine is run once a second
   //The 2 interrupt routines will not interrupt each other
   oneSecondCounter++;
   uint8_t i = 0;
   
   for (i = 0; i < system->numberOfThreads; i++) {
      //system->threads[i] 
   }
}

/**
 * Configures the system timer to trigger an interrupt approximately every
 * 10 ms and 1 s.
 */
void start_system_timer() {
   //start timer 0 for OS system interrupt
   TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
   TCCR0A |= _BV(WGM01);   //clear timer on compare match

   //Generate timer interrupt every ~10 milliseconds
   TCCR0B |= _BV(CS02) | _BV(CS00);    //prescalar /1024
   OCR0A = 156;             //generate interrupt every 9.98 milliseconds

   //start timer 1 to generate interrupt every 1 second
   OCR1A = 15625;
   TIMSK1 |= _BV(OCIE1A);  //interrupt on compare
   TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS10); //slowest prescalar /1024
}

// Context switch will pop off the manually saved registers,
// then ret to thread_start. ret will pop off the automatically
// saved registers and thread_start will pop off the
// function address and then ijmp to the function.
__attribute__((naked)) void context_switch(uint16_t* newStackPointer,
 uint16_t* oldStackPointer) {

   // Manually save registers
   asm volatile("push r2");
   asm volatile("push r3");
   asm volatile("push r4");
   asm volatile("push r5");
   asm volatile("push r6");
   asm volatile("push r7");
   asm volatile("push r8");
   asm volatile("push r9");
   asm volatile("push r10");
   asm volatile("push r11");
   asm volatile("push r12");
   asm volatile("push r13");
   asm volatile("push r14");
   asm volatile("push r15");
   asm volatile("push r16");
   asm volatile("push r17");
   asm volatile("push r28");
   asm volatile("push r29");

   // Changing stack pointer!
   {
      
      // Load current stack pointer into r16/r17
      asm volatile("in r16, __SP_L__");
      asm volatile("in r17, __SP_H__");

      // Load the oldStackPointer into z
      asm volatile("mov r30, r22");
      asm volatile("mov r31, r23");

      // Save current stack pointer into oldStackPointer
      asm volatile("st z+, r16");
      asm volatile("st z, r17");
 
      // Load newStackPointer into z
      asm volatile("mov r30, r24");
      asm volatile("mov r31, r25");

      // Load newStackPointer into r16/r17
      asm volatile("ld r16, z+");
      asm volatile("ld r17, z");

      // Load newStackPointer into current statck pointer
      asm volatile("out __SP_L__, r16");
      asm volatile("out __SP_H__, r17");
   }

   // Manually load registers!
   asm volatile("pop r29");
   asm volatile("pop r28");
   asm volatile("pop r17");
   asm volatile("pop r16");
   asm volatile("pop r15");
   asm volatile("pop r14");
   asm volatile("pop r13");
   asm volatile("pop r12");
   asm volatile("pop r11");
   asm volatile("pop r10");
   asm volatile("pop r9");
   asm volatile("pop r8");
   asm volatile("pop r7");
   asm volatile("pop r6");
   asm volatile("pop r5");
   asm volatile("pop r4");
   asm volatile("pop r3");
   asm volatile("pop r2");
   asm volatile("ret");
}

/**
 * Move the function address contained in the thread into the Z register for
 * dereferencing and execution.
 */
__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave this as the first
          //statement in thread_start()
   asm volatile("mov r30, r16");
   asm volatile("mov r31, r17");
   asm volatile("mov r24, r14");
   asm volatile("mov r25, r15");
   asm volatile("ijmp");
}

/**
 * Starts the operating system by starting the system timer and performing the
 * very first invocation of context_switch().
 */
void os_start(void) {

   start_system_timer();

   context_switch((uint16_t *) (&system->threads[0].stackPointer), 
    (uint16_t *) (&system->threads[7].stackPointer));
}

/**
 * Returns the thread following the currently-executing thread in the system's
 * array list of threads.
 *
 * @return The thread following the thread that is currently executing.
 */
uint8_t get_next_thread(void) {
   int i = 0;
   for (i = (system->currentThreadId + 1) % system->numberOfThreads;
    i != system->currentThreadId; i = (i + 1) % system->numberOfThreads) {
      if (system->threads[i].state == THREAD_READY) {
         break;
      }
   }

   return i; 
}

/**
 * Returns the system time in seconds.
 *
 * @return The system time, in seconds.
 */
uint32_t getSystemTime(void) {
   return oneSecondCounter;
}

/**
 * Returns the total number of threads registered in the system.
 *
 * @return The total number of threads registered in the system.
 */
uint8_t getNumberOfThreads(void) {
   return system->numberOfThreads;
}

/**
 * Returns the number of interrupts occuring per second. The result is an
 * integer.
 *
 * @return the number of interrupts occuring per second.
 */
uint32_t getInterruptsPerSecond(void) {
   uint32_t sysTime = getSystemTime();
   return sysTime ? tenMillisecondCounter / sysTime : tenMillisecondCounter;
}

/**
 * Prints the following information:
 * 1. System time in seconds
 * 2. Interrupts per second (number of OS interrupts per second)
 * 3. Number of threads in the system
 * 4. Per-thread information
 *    thread id
 *    thread pc (starting pc)
 *    stack usage (number of bytes used by the stack)
 *    total stack size (number of bytes allocated for the stack)
 *    current top of stack (current top of stack address)
 *    stack base (lowest possible stack address)
 *    stack end (highest possible stack address)
 */
void printSystemInfo() {
   while (1) {
      //_delay_ms(100);
      set_cursor(1, 1);

      set_color(MAGENTA);

      //System time
      print_string("System time: ");
      print_int32(getSystemTime());
      print_string("\n\r");

      //Interrupts per second
      print_string("Interrupts per second: ");
      print_int32(getInterruptsPerSecond());
      print_string("\n\r");

      //Number of threads in the system
      print_string("Thread count: ");
      print_int(getNumberOfThreads());
      print_string("\n\n\r");

      //Per-thread information



      // CHAO: Hello, Ni Hao. No \n\r. Set all of them to be set_cursor()
      // Set all end of line print_string() to have 3 extra spaces at the end of them.
      // Add other specs from spec.
      // Move this function to program3.c
      // Make system and extern variable in other file
      // Change color of display_bounded_buffer to Magenta
      // Test it on your arduino
      // Add sei() and cli() to switchThreads/switchNextThread
      // Delete old comments
      // Add 'a' 'z' for consumer and producer in this function
      // Add wait and signal into producer and consumer and display_bounded_buffer
      int i = 0;
      for (; i < system->numberOfThreads; i++) {
         set_color(BLACK + i);
         print_string("Thread ");
         print_int(system->threads[i].threadId);
         print_string("\n\r");

         print_string("Thread PC: 0x");
         print_hex(system->threads[i].functionAddress);
         print_string("\n\r");

         print_string("Stack usage: ");
         print_int((uint16_t) (system->threads[i].highestStackAddress
            - system->threads[i].stackPointer));
         print_string("\n\r");

         print_string("Total stack size: ");
         print_int(system->threads[i].stackSize);
         print_string("\n\r");

         print_string("Current top of stack: 0x");
         print_hex((uint16_t) system->threads[i].stackPointer);
         print_string("\n\r");

         print_string("Stack base: 0x");
         print_hex((uint16_t) system->threads[i].highestStackAddress);
         print_string("\n\r");

         print_string("Stack end: 0x");
         print_hex((uint16_t) system->threads[i].lowestStackAddress);
         print_string("\n\n\r");
      }
   }
}
