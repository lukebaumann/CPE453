#include <avr/io.h>
#include <avr/interrupt.h>
#include "globals.h"
#include "os.h"

volatile struct system_t *system;

void os_init(void) {
   system = calloc(sizeof(system_t));
   system->systemTime = getSystemTime();
}

// Not sure what to do with args
void create_thread(uint16_t address, void *args, uint16_t stackSize) {
   struct thread_t newThread = system->threads[system->numberOfThreads];
   newThread.threadId = system->numberOfThreads++;
   newThread.stackSize = stackSize + sizeof(regs_interrup);
   newThread.lowestStackAddress = malloc(newThread.stackSize * sizeof(uint8_t));
   newThread.highestStackAddress = lowestStackAddress + newThread.stackSize;
   newThread.stackPointer = highestStackAddress ;

   // r31
   *newThread.stackPointer-- = 0;
   // r30
   *newThread.stackPointer-- = 0;
   // r27
   *newThread.stackPointer-- = 0;
   // r26
   *newThread.stackPointer-- = 0;
   // r25
   *newThread.stackPointer-- = 0;
   // r24
   *newThread.stackPointer-- = 0;
   // r23
   *newThread.stackPointer-- = 0;
   // r22
   *newThread.stackPointer-- = 0;
   // r21
   *newThread.stackPointer-- = 0;
   // r20
   *newThread.stackPointer-- = 0;
   // r19
   *newThread.stackPointer-- = 0;
   // r18
   *newThread.stackPointer-- = 0;
   // sreg
   *newThread.stackPointer-- = 0;
   // r0
   *newThread.stackPointer-- = 0;
   // r1
   *newThread.stackPointer-- = 0;
   // PC low byte
   *newThread.stackPointer-- = 0x00FF & address;
   // PC high byte
   *newThread.stackPointer-- = 0x00FF & (address >> 8);
   // r2
   *newThread.stackPointer-- = 0;
   // r3
   *newThread.stackPointer-- = 0;
   // r4
   *newThread.stackPointer-- = 0;
   // r5
   *newThread.stackPointer-- = 0;
   // r6
   *newThread.stackPointer-- = 0;
   // r7
   *newThread.stackPointer-- = 0;
   // r8
   *newThread.stackPointer-- = 0;
   // r9
   *newThread.stackPointer-- = 0;
   // r10
   *newThread.stackPointer-- = 0;
   // r11
   *newThread.stackPointer-- = 0;
   // r12
   *newThread.stackPointer-- = 0;
   // r13
   *newThread.stackPointer-- = 0;
   // r14
   *newThread.stackPointer-- = 0;
   // r15
   *newThread.stackPointer-- = 0;
   // r16
   *newThread.stackPointer-- = 0;
   // r17
   *newThread.stackPointer-- = 0;
   // r28
   *newThread.stackPointer-- = 0;
   // r29
   *newThread.stackPointer-- = 0;
}

//This interrupt routine is automatically run every 10 milliseconds
ISR(TIMER0_COMPA_vect) {
   //The following statement tells GCC that it can use registers r18-r27, 
   //and r30-31 for this interrupt routine.  These registers (along with
   //r0 and r1) will automatically be pushed and popped by this interrupt routine.
   asm volatile ("" : : : "r18", "r19", "r20", "r21", "r22", "r23", "r24", \
                 "r25", "r26", "r27", "r30", "r31");                        


   //Call get_next_thread to get the thread id of the next thread to run
   uint8_t nextThreadId = get_next_thread();
   uint8_t currentThreadId = system->currentThreadId;
   system->currentThreadId = nextThreadId;
   //Call context switch here to switch to that next thread
   context_switch(&system->threads[nextThreadId].stackPointer, &system->threads[currentThread].stackPointer);
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
   asm volatile("push pc");

   // Load the oldStackPointer into z
   asm volatile("movw r30, r22")
   asm volatile("movw r31, r23")

   // Save sp intp oldStackPointer
   asm volatile("st z, sp");
   
   // Load newStackPointer into z
   asm volatile("movw r30, r24")
   asm volatile("movw r31, r25")

   // Load newStackPointer into sp
   asm volatile("ld sp, z");
   
   // Manually load registers
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

__attribute__((naked)) void thread_start(void) {
   sei(); //enable interrupts - leave this as the first statement in thread_start()
}

void os_start(void) {
   

uint8_t get_next_thread(void) {
   return (system->currentThreadId + 1) % system->numberOfThreads;
}
