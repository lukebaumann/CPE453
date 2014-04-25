#include "os.h"

volatile struct system_t *system;
volatile uint32_t isrCounter = 0;

void os_init(void) {
   //thread = calloc(1, sizeof(struct thread_t));
   
   system = calloc(1, sizeof(struct system_t));
   system->currentThreadId = 0;
   system->numberOfThreads = 0;
   system->systemTime = getSystemTime();
   
}

// Context switch will pop off the manually saved registers,
// then ret to thread_start. ret will pop off the automatically
// saved registers and thread_start will pop off the
// function address and then ijmp to the function.
// I am not sure how the args address plays into everything.
// thread_start address low byte
void create_thread(uint16_t address, void *args, uint16_t stackSize) {
   volatile struct thread_t *newThread = &system->threads[system->numberOfThreads];
   //volatile struct thread_t *newThread = thread; 

   newThread->threadId = system->numberOfThreads++;

   newThread->stackSize = stackSize + sizeof(struct regs_interrupt) +
    sizeof(struct regs_context_switch);
   newThread->lowestStackAddress = malloc(newThread->stackSize * sizeof(uint8_t));
   newThread->highestStackAddress = newThread->lowestStackAddress +
    newThread->stackSize;
   newThread->stackPointer = newThread->highestStackAddress;

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

   newThread->stackPointer = (uint16_t *) registers;

   return;
}

//This interrupt routine is automatically run every 10 milliseconds
ISR(TIMER0_COMPA_vect) {
   //The following statement tells GCC that it can use registers r18-r27, 
   //and r30-31 for this interrupt routine.  These registers (along with
   //r0 and r1) will automatically be pushed and popped by this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");                        

   isrCounter++;

   //Call get_next_thread to get the thread id of the next thread to run
   uint8_t nextThreadId = get_next_thread();
   uint8_t currentThreadId = system->currentThreadId;
   system->currentThreadId = nextThreadId;

   //Call context switch here to switch to that next thread
   context_switch((uint16_t *) &system->threads[nextThreadId].stackPointer,
    (uint16_t *) &system->threads[currentThreadId].stackPointer);
}

//Call this to start the system timer interrupt
void start_system_timer() {
   TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
   TCCR0A |= _BV(WGM01);   //clear timer on compare match

   //Generate timer interrupt every ~10 milliseconds
   TCCR0B |= _BV(CS02) | _BV(CS00);    //prescalar /1024
   OCR0A = 156;             //generate interrupt every 9.98 milliseconds
}

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

// Pop off the function address into the z register and then jump to it
__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave this as the first statement in thread_start()
   print_string("\n\rI am really here!\n\r");
   asm volatile("mov r30, r16");
   asm volatile("mov r31, r17");
   asm volatile("mov r24, r14");
   asm volatile("mov r25, r15");
   asm volatile("ijmp");
}

void os_start(void) {
   uint16_t mainStackPointer = 0;
   //system->currentThreadId = 0;
   start_system_timer();

   context_switch((uint16_t *) (&system->threads[0].stackPointer), &mainStackPointer);
   //context_switch((uint16_t *) (&thread->stackPointer), &mainStackPointer);
}

uint8_t get_next_thread(void) {
   return (system->currentThreadId + 1) % system->numberOfThreads;
}

uint32_t getSystemTime(void) {
   return isrCounter * 10;
}
