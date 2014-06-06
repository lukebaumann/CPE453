/**
 * File: synchro.c
 * Authors: Luke Baumann, Chao Chen, (program 2 contribution: Tyler Kowallis)
 * CPE 453 Program 03
 * 05/09/2014
 */

#include "os.h"
#include "globals.h"

extern volatile struct system_t *system;

/**
 * A thread can yield the CPU to other threads by calling this function.
 * All that is necessary is to switch to the next thread.
 */
void yield() {
   cli();
   switchNextThread();
   sei();
}

/**
 * This puts a thread to sleep by setting its sleepingTicksLeft which is a
 * down counter for time remaining while sleeping. Then the state is set to
 * THREAD_SLEEPING and the next thread is switched in.
 *
 * @param ticks The number of ten millisecond interrupts before the
 * thread wakes
 */
void thread_sleep(uint16_t ticks) {
   //cli();
   // If ticks is 0, this will behave exactly as yield()
   if (ticks > 0) {
      system->threads[system->currentThreadId].state = THREAD_SLEEPING;
      system->threads[system->currentThreadId].sleepingTicksLeft = ticks;
   }

   switchNextThread();
   //sei();
}

/**
 * Initializes a mutex
 *
 * @param m The address of the mutex_t 
 */
void mutex_init(struct mutex_t* m) {
   // Above the max thread id and main thread id
   m->ownerId = MAX_NUMBER_OF_THREADS + 2;
   m->lock = 0;
   m->startIndex = 0;
   m->endIndex = 0;
}

/**
 * If the mutex is not locked, the thread locks the mutex and becomes the
 * owner. Otherwise, the thread is placed on the waiting list, put in the
 * waiting state, and the next thread is switched in. Once the thread is
 * no longer waiting, it locks the mutex and becomes the owner.
 *
 * @param m The address of the mutex_t 
 */
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

      switchNextThread();

      m->ownerId = system->currentThreadId;
      m->lock = 1;
   }
   sei();
}

/**
 * If the thread is not the owner, do nothing. Otherwise, unlock the mutex
 * and if there is a thread waiting, remove the first one from the waiting
 * list, change its state to ready, and switch to it.
 *
 * @param m The address of the mutex_t 
 */
void mutex_unlock(struct mutex_t* m) {
   cli();
   uint8_t nextThreadId = 0;

   if (m->ownerId == system->currentThreadId) {
      m->lock = 0;

      if (m->startIndex != m->endIndex) {
         system->threads[m->waitingThreadsIds[m->startIndex]].state = THREAD_READY;
      
         nextThreadId = m->waitingThreadsIds[m->startIndex];

         m->startIndex = (m->startIndex + 1) % MAX_NUMBER_OF_THREADS;

         switchThreads(nextThreadId);
      }
   }
   sei();
}

/**
 * Initiatizes a semaphore
 *
 * @param s The address of the semaphore_t 
 * @param value The number of threads that can concurrently access the resource 
 */
void sem_init(struct semaphore_t* s, int8_t value) {
   cli();
   s->value = value;
   s->startIndex = 0;
   s->endIndex = 0;
   sei();
}

/**
 * If the semaphore has an available space, decrement the number of free
 * spaces. Otherwise, put the thread on the waiting list, change its state,
 * switch to the next thread, and then decrement the number of free spaces.
 *
 * It seems kind of unfair if I am finally at the front of the waiting line,
 * someone signals, I am ready, someone else grabs my spot before I run, and
 * now I am at the end of the waitlist again.
 *
 * @param s The address of the semaphore_t 
 */
void sem_wait(struct semaphore_t* s) {
   cli();
   while (s->value <= 0) {
      s->waitingThreadsIds[s->endIndex] = system->currentThreadId;
      s->endIndex = (s->endIndex + 1) % MAX_NUMBER_OF_THREADS;
      system->threads[system->currentThreadId].state = THREAD_WAITING;

      switchNextThread();
   }

   s->value--;
   sei();
}

/**
 * Increment the number of free spaces. Then, check to see if there are
 * threads waiting. If there are, remove the first one and change its state.
 * Do not immediately switch to it.
 *
 * Here is a question: How do I prevent a thread that has never
 * called sem_wait() from calling sem_signal()?
 *
 * @param s The address of the semaphore_t 
 */
void sem_signal(struct semaphore_t* s) {
   cli();
   s->value++;

   if (s->startIndex != s->endIndex) {
      system->threads[s->waitingThreadsIds[s->startIndex]].state = THREAD_READY;
      s->startIndex = (s->startIndex + 1) % MAX_NUMBER_OF_THREADS;
   }

   sei();
}

/**
 * Same as sem_signal() but it immediately switches to the first waiting
 * thread if there is one.

 * @param s The address of the semaphore_t 
 */
void sem_signal_swap(struct semaphore_t* s) {
   cli();
   uint8_t nextThreadId = 0;

   s->value++;

   if (s->startIndex != s->endIndex) {
      nextThreadId = s->waitingThreadsIds[s->startIndex];
      system->threads[nextThreadId].state = THREAD_READY;
      s->startIndex = (s->startIndex + 1) % MAX_NUMBER_OF_THREADS;

      switchNextThread(nextThreadId);
   }
   sei(); 
}

/**
 * Runs through all the threads and looks for sleeping threads to
 * decrement their tick counters.
 */
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
