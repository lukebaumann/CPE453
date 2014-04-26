/**
 * File: os.h
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 02
 * 04/25/2014
 */

#ifndef OS_H
#define OS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

//This structure defines the register order pushed to the stack on a
//system context switch.
struct regs_context_switch {
   //stack pointer is pointing to 1 byte below the top of the stack
   uint8_t padding;

   //Registers that will be managed by the context switch function (via gcc)
   uint8_t r29;
   uint8_t r28;
   uint8_t r17;
   uint8_t r16;
   uint8_t r15;
   uint8_t r14;
   uint8_t r13;
   uint8_t r12;
   uint8_t r11;
   uint8_t r10;
   uint8_t r9;
   uint8_t r8;
   uint8_t r7;
   uint8_t r6;
   uint8_t r5;
   uint8_t r4;
   uint8_t r3;
   uint8_t r2;
   uint8_t pch;
   uint8_t pcl;
};

//This structure defines how registers are pushed to the stack when
//the system tick interrupt occurs.  This struct is never directly 
//used, but instead be sure to account for the size of this struct 
//when allocating initial stack space
struct regs_interrupt {
   uint8_t padding; //stack pointer is pointing to 1 byte below the top
                    //of the stack

   //Registers that are pushed to the stack during an interrupt service routine
   uint8_t r31;
   uint8_t r30;
   uint8_t r27;
   uint8_t r26;
   uint8_t r25;
   uint8_t r24;
   uint8_t r23;
   uint8_t r22;
   uint8_t r21;
   uint8_t r20;
   uint8_t r19;
   uint8_t r18;
   uint8_t sreg; //status register
   uint8_t r0;
   uint8_t r1;
   uint8_t pch;
   uint8_t pcl;
};

#define MAX_NUMBER_OF_THREADS 8

// This structure holds thread specific information
struct thread_t {
   uint8_t threadId;
   uint16_t stackSize;
   uint16_t functionAddress;
   uint8_t *lowestStackAddress;
   uint8_t *highestStackAddress;
   uint8_t *stackPointer;
};

// This structure holds system information
struct system_t {
   struct thread_t threads[MAX_NUMBER_OF_THREADS];
   uint8_t currentThreadId;
   uint8_t numberOfThreads;
   uint32_t systemTime;
};


void os_init(void);
void create_thread(uint16_t address, void *args, uint16_t stackSize);
ISR(TIMER0_COMPA_vect);
void start_system_timer();
__attribute__((naked)) void context_switch(uint16_t* newStackPointer,
    uint16_t* oldStackPointer);
__attribute__((naked)) void thread_start(void);
uint8_t get_next_thread(void);
uint32_t getSystemTime(void);
uint8_t getNumberOfThreads(void);
void printSystemInfo(void);

#endif
