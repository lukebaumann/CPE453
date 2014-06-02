/**
 * File: os.c
 * Authors: Luke Baumann, Tyler Kowallis)
 * CPE 453 Program 05
 * 05/09/2014
 */

#include "os.h"
#include "globals.h"

volatile struct system_t *system;
volatile uint32_t tenMillisecondCounter = 0;
volatile uint32_t oneSecondCounter = 0;

extern uint8_t playBufferIndex;
extern static uint8_t playBuffer;

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
   newThread->runsCurrentSecond = 0; 
   newThread->runsLastSecond = 0;

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
 * Fetches the next thread to run, decrements sleeping thread's tick
 * counters, increments the ten millisecond counter and then invokes
 * context-switching between threads every 10 ms. The ISR is based
 * off of the system timer.
 */
ISR(TIMER0_COMPA_vect) {
   //The following statement tells GCC that it can use registers r18-r27, 
   //and r30-31 for this interrupt routine.  These registers (along with
   //r0 and r1) will automatically be pushed and popped by
   //this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");                        

   tenMillisecondCounter++;
   
   if (!++playBufferIndex) {
      playBuffer = readBuffer;
      readBuffer = (readBuffer + 1) % NUMBER_OF_BUFFERS;
   }

   notifySleepingThreads();
   switchNextThread();
}

/**
 * Calls switchThreads() with the threadId retrieved from get_next_thread()
 */
void switchNextThread() {
   sei();
   switchThreads(get_next_thread());
   cli();
}

/**
 * Switches in a new thread. If the current thread's state is THREAD_RUNNING,
 * then it needs to be THREAD_READY. Otherwise, I do not want to change the
 * state; keep it as THREAD_WAITING or THREAD_SLEEPING. For the next thread, 
 * increment the runsCurrentSecond counter, and change the state to
 * THREAD_RUNNING. Change the currentThreadId to the nextThreadId and perform
 * the context_switch()
 *
 * @param nextThreadId The threadId that will be run next
 */
void switchThreads(uint8_t nextThreadId) {
   sei();

   uint8_t currentThreadId = system->currentThreadId;
   
   //if the current thread was interrupted and not changed 
   //into a waiting or sleeping state, change its state to THREAD_READY
   if (system->threads[system->currentThreadId].state == THREAD_RUNNING) {
      system->threads[system->currentThreadId].state = THREAD_READY;
   }

   system->threads[nextThreadId].runsCurrentSecond++;
   system->threads[nextThreadId].state = THREAD_RUNNING;
   
   system->currentThreadId = nextThreadId;

   //Call context switch here to switch to that next thread
   context_switch((uint16_t *) &system->threads[nextThreadId].stackPointer,
    (uint16_t *) &system->threads[currentThreadId].stackPointer);

   cli();
}

/**
 * This is the interrupt that gets runs every second. It resets the
 * runsCurrentSecond and sets runsLastSecond for all the threads.
 */
ISR(TIMER1_COMPA_vect) {
   //This interrupt routine is run once a second
   //The 2 interrupt routines will not interrupt each other
   oneSecondCounter++;
   uint8_t i = 0;
   
   for (i = 0; i < system->numberOfThreads; i++) {
      system->threads[i].runsLastSecond = system->threads[i].runsCurrentSecond;
      system->threads[i].runsCurrentSecond = 0;
   }

   system->threads[MAX_NUMBER_OF_THREADS].runsLastSecond = system->threads[MAX_NUMBER_OF_THREADS].runsCurrentSecond;
   system->threads[MAX_NUMBER_OF_THREADS].runsCurrentSecond = 0;
}

/**
 * Configures the system timer to trigger an interrupt approximately every
 * 10 ms and 1 s.
 */
// void start_system_timer() {
//    //start timer 0 for OS system interrupt
//    TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
//    TCCR0A |= _BV(WGM01);   //clear timer on compare match

//    //Generate timer interrupt every ~10 milliseconds
//    TCCR0B |= _BV(CS02) | _BV(CS00);    //prescalar /1024
//    OCR0A = 156;             //generate interrupt every 9.98 milliseconds

//    //start timer 1 to generate interrupt every 1 second
//    OCR1A = 15625;
//    TIMSK1 |= _BV(OCIE1A);  //interrupt on compare
//    TCCR1B |= _BV(WGM12) | _BV(CS12) | _BV(CS10); //slowest prescalar /1024
// }

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
 * Starts the operating system by starting the system timer, creating the main thread,
 * and performing the very first invocation of context_switch().
 */
void os_start(void) {
   createMainThread();

   start_system_timer();
   context_switch((uint16_t *) (&system->threads[0].stackPointer), 
    (uint16_t *) (&system->threads[MAX_NUMBER_OF_THREADS].stackPointer));
}

/**
 * Initializes the main thread which is located at the end of the array of threads. 
 */
void createMainThread() {
   volatile struct thread_t *mainThread = &system->threads[MAX_NUMBER_OF_THREADS];
 
   mainThread->highestStackAddress = (uint8_t *) 0x8FF;
   mainThread->functionAddress = (uint16_t) main;
   mainThread->threadId = MAX_NUMBER_OF_THREADS;

   mainThread->state = THREAD_RUNNING;
   mainThread->sleepingTicksLeft = 0;
   
   mainThread->runsCurrentSecond = 1;
   mainThread->runsLastSecond = 0;
}

/**
 * Returns the thread following the currently-executing thread in the system's
 * array list of threads.
 *
 * @return The thread following the thread that is currently executing.
 */
uint8_t get_next_thread(void) {
   int i = 0;

   // If the main thread is the current thread and no thread is ready yet,
   // the main thread should run again
   if (system->currentThreadId == MAX_NUMBER_OF_THREADS) {
      for (i = 0; i < system->numberOfThreads; i++) {
         if (system->threads[i].state == THREAD_READY) {
            return i;
         }
      }

      return MAX_NUMBER_OF_THREADS;
   }

   else {
      // Otherwise, if a thread is ready, return its index
      for (i = (system->currentThreadId + 1) % system->numberOfThreads;
       i != system->currentThreadId; i = (i + 1) % system->numberOfThreads) {
         if (system->threads[i].state == THREAD_READY) {
            return i;
         }
      }
      
      // Need to check to make sure that the current thread was not put into
      // a waiting or sleeping state. It would not have been put into a ready
      // state yet because that happens in switchThreads()
      if (system->threads[system->currentThreadId].state == THREAD_RUNNING) {
         return system->currentThreadId;
      }
      else {
         return MAX_NUMBER_OF_THREADS;
      }
   }
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
