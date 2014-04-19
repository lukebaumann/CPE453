#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"

void os_init(void) {
   system.numberOfThreads = 0;
   system.systemTime = getSystemTime();
}

void create_thread(uint16_t address, void *args, uint16_t stackSize) {
   struct thread_t newThread = system.threads[system.numberOfThreads];
   newThread.threadId = system.numberOfThreads++;
   newThread.stackSize = stackSize + STACK_SIZE_PADDING;
   newThread.lowestStackAddress = malloc(newThread.stackSize * sizeof(uint8_t));
   newThread.highestStackAddress = lowestStackAddress + newThread.stackSize;
   newThread.stackPointer = highestStackAddress;

   // PC low byte
   *newThread.stackPointer++ = 0x00FF & address;
   // PC high byte
   *newThread.stackPointer++ = 0x00FF & (address >> 8);
   // r2
   *newThread.stackPointer++ = 0;
   // r3
   *newThread.stackPointer++ = 0;
   // r4
   *newThread.stackPointer++ = 0;
   // r5
   *newThread.stackPointer++ = 0;
   // r6
   *newThread.stackPointer++ = 0;
   // r7
   *newThread.stackPointer++ = 0;
   // r8
   *newThread.stackPointer++ = 0;
   // r9
   *newThread.stackPointer++ = 0;
   // r10
   *newThread.stackPointer++ = 0;
   // r11
   *newThread.stackPointer++ = 0;
   // r12
   *newThread.stackPointer++ = 0;
   // r13
   *newThread.stackPointer++ = 0;
   // r14
   *newThread.stackPointer++ = 0;
   // r15
   *newThread.stackPointer++ = 0;
   // r16
   *newThread.stackPointer++ = 0;
   // r17
   *newThread.stackPointer++ = 0;
   // r28
   *newThread.stackPointer++ = 0;
   // r29
   *newThread.stackPointer++ = 0;

}

//This interrupt routine is automatically run every 10 milliseconds
ISR(TIMER0_COMPA_vect) {
   //The following statement tells GCC that it can use registers r18-r27, 
   //and r30-31 for this interrupt routine.  These registers (along with
   //r0 and r1) will automatically be pushed and popped by this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");                        


   //Call get_next_thread to get the thread id of the next thread to run
   //Call context switch here to switch to that next thread
}

//Call this to start the system timer interrupt
void start_system_timer() {
   TIMSK0 |= _BV(OCIE0A);  //interrupt on compare match
   TCCR0A |= _BV(WGM01);   //clear timer on compare match

   //Generate timer interrupt every ~10 milliseconds
   TCCR0B |= _BV(CS02) | _BV(CS00);    //prescalar /1024
   OCR0A = 156;             //generate interrupt every 9.98 milliseconds
}

__attribute__((naked)) void context_switch(uint16_t* newStackPointer, uint16_t* oldStackPointer) {
   volatile asm("push r2");
   volatile asm("push r3");
   volatile asm("push r4");
   volatile asm("push r5");
   volatile asm("push r6");
   volatile asm("push r7");
   volatile asm("push r8");
   volatile asm("push r9");
   volatile asm("push r10");
   volatile asm("push r11");
   volatile asm("push r12");
   volatile asm("push r13");
   volatile asm("push r14");
   volatile asm("push r15");
   volatile asm("push r16");
   volatile asm("push r17");
   volatile asm("push r28");
   volatile asm("push r29");


}

__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave this as the first statement in thread_start()
}
