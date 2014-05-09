/**
 * File: program2.c
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 02
 * 04/25/2014
 */

#include "globals.h"
#include "os.h"

extern volatile struct system_t *system;
extern volatile uint32_t tenMillisecondCounter;
extern volatile uint32_t oneSecondCounter;

static struct semaphore_t bufferSemaphore;
static struct mutex_t bufferMutex;
static uint16_t bufferSize = 0;
static uint16_t consumeTime = DEFAULT_CONSUME_TIME;
static uint16_t produceTime = DEFAULT_PRODUCE_TIME;

/**
 * Ivokes the operating system and is the idle thread.
 */
void main() {
   serial_init();
   os_init();

   create_thread((uint16_t) producer, 0, 50);
   create_thread((uint16_t) consumer, 0, 51);
   create_thread((uint16_t) display_stats, 0, 52);
   create_thread((uint16_t) display_bounded_buffer, 0, 53);
   create_thread((uint16_t) blink, 0, 54);

   sem_init(&bufferSemaphore, 1);

   clear_screen();
   os_start();
   sei();

   while(1) {}
}

/**
 * This thread sleeps for a given number of interrupts,
 * grabs a mutex and semaphore on the bufferMutex to show
 * that both are working, increments the buffer, releases
 * the locks, and repeats.
 */
void producer() {
   while(1) {
      thread_sleep(produceTime);

      if (bufferSize < MAX_BUFFER_SIZE) {

         mutex_lock(&bufferMutex);
         sem_wait(&bufferSemaphore);

         bufferSize++;

         sem_signal(&bufferSemaphore);
         mutex_unlock(&bufferMutex);
      }
   }
}

/**
 * This thread sleeps for a given number of interrupts,
 * grabs a mutex and semaphore on the bufferMutex to show
 * that both are working, decrements the buffer, releases
 * the locks, and repeats.
 */
void consumer() {
   while(1) {
      thread_sleep(consumeTime);

      if (bufferSize > 0) {

         mutex_lock(&bufferMutex);
         sem_wait(&bufferSemaphore);

         bufferSize--;

         sem_signal(&bufferSemaphore);
         mutex_unlock(&bufferMutex);
      }
   }
}

/**
 * Displays a visual representation of the buffer and stats
 * on how fast items are being produced and consumed. This
 * thread is responsible for handling key presses to change
 * produce and consume times.
 */
void display_bounded_buffer() {
   while (1) {
      handleKeys();

      set_cursor(1, 81);
      set_color(MAGENTA);
      print_string("Producing 1 item per ");
      print_int(produceTime * 10);
      print_string(" ms   ");

      set_cursor(2, 81);
      print_string("Consuming 1 item per ");
      print_int(consumeTime * 10);
      print_string(" ms   ");

      uint8_t i = 0;
      for (i = 0; i < MAX_BUFFER_SIZE; i++) {
         set_color(BLACK + (i % (WHITE - BLACK)));
         set_cursor(3 + MAX_BUFFER_SIZE - i, 91);

         if (i < bufferSize) {
            print_string("XXX");
         }
         else {
            print_string("   ");
         }
      }

      thread_sleep(5);
   }
}

/**
 * Handles key presses
 * 'a' and 'z' control produce time
 * 'k' and 'm' control consume time
 * 'e' and 'f' set the buffer to 0 or filled
 */
void handleKeys() {
   uint8_t key = read_byte();

   if (key != 255) {
      if (key == 'a') {
         produceTime++;
      }
      else if (key == 'z' && produceTime > 0) {
         produceTime--;
      }
      else if (key == 'k') {
         consumeTime++;
      }
      else if (key == 'm' && consumeTime > 0) {
         consumeTime--;
      }
      else if (key == 'e') {
         bufferSize = 0;
      }
      else if (key == 'f') {
         bufferSize = MAX_BUFFER_SIZE;
      }
   }
}

/**
 * Prints the following information:
 * 1. System time in seconds
 * 2. Interrupts per second (number of OS interrupts per second)
 * 3. Number of threads in the system not including the main thread
 * 4. Per-thread information
 */
void display_stats() {
   while (1) {
      thread_sleep(5);
      set_cursor(1, 1);

      set_color(MAGENTA);

      //System time
      print_string("System time: ");
      print_int32(getSystemTime());
      print_string("   ");
      set_cursor(2, 1);
	  
      //Interrupts per second
      print_string("Interrupts per second: ");
      print_int32(getInterruptsPerSecond());
      print_string("   ");
      set_cursor(3, 1);

      //Number of threads in the system
      print_string("Thread count: ");
      print_int(getNumberOfThreads());
      print_string("   ");
      set_cursor(4, 1);

      int i = 0;
      for (i = 0; i < system->numberOfThreads; i++) {
         printThreadStats(i, i);
      }

      // Main thread stats
      printThreadStats(MAX_NUMBER_OF_THREADS, i);
   }
}

/**
 * Prints per thread information:
 *    thread id
 *    thread pc (starting pc)
 *    runs per second, based on last second
 *    stack usage (number of bytes used by the stack)
 *    total stack size (number of bytes allocated for the stack)
 *    current top of stack (current top of stack address)
 *    stack base (lowest possible stack address)
 *    stack end (highest possible stack address)
 */
void printThreadStats(uint8_t threadIndex, uint8_t threadCount) {
   uint8_t lineNumber = 0;
   uint8_t rowOffset = 5 + (threadCount / 2) * STAT_DISPLAY_HEIGHT;
   uint8_t colOffset = threadCount % 2 ? 41 : 1;

   set_cursor(rowOffset + lineNumber++, colOffset);
   set_color(BLACK + threadIndex);
   print_string("Thread ");
   print_int(system->threads[threadIndex].threadId);
   print_string("   ");

   set_cursor(rowOffset + lineNumber++, colOffset);
   print_string("Thread PC: 0x");
   print_hex(system->threads[threadIndex].functionAddress);
   print_string("   ");

   set_cursor(rowOffset + lineNumber++, colOffset);
   print_string("Runs per second: ");
   print_int(system->threads[threadIndex].runsLastSecond);
   print_string("   ");

   set_cursor(rowOffset + lineNumber++, colOffset);
   print_string("Stack usage: ");
   print_int((uint16_t) (system->threads[threadIndex].highestStackAddress -
    system->threads[threadIndex].stackPointer));
   print_string("   ");

   set_cursor(rowOffset + lineNumber++, colOffset);
   print_string("Total stack size: ");
   print_int(system->threads[threadIndex].stackSize);
   print_string("   ");

   set_cursor(rowOffset + lineNumber++, colOffset);
   print_string("Current top of stack: 0x");
   print_hex((uint16_t) system->threads[threadIndex].stackPointer);
   print_string("   ");

   set_cursor(rowOffset + lineNumber++, colOffset);
   print_string("Stack base: 0x");
   print_hex((uint16_t) system->threads[threadIndex].highestStackAddress);
   print_string("   ");

   set_cursor(rowOffset + lineNumber++, colOffset);
   print_string("Stack end: 0x");
   print_hex((uint16_t) system->threads[threadIndex].lowestStackAddress);
   print_string("   ");
}

/**
 * Turns LED on if it is producing, off otherwise 
 */
void blink(void) {
   while (1) {
      if (bufferSize < MAX_BUFFER_SIZE && produceTime > 0) {
         led_on();
      }
      else {
         led_off();
      }

      thread_sleep(1);
   }
}

/**
 * Turns LED on
 */
void led_on() {
   //Set data direction to OUTPUT
   //Clear Z high byte
   __asm__ volatile ("clr r31");
   //Set Z low byte to DDRB
   __asm__ volatile ("ldi r30,0x24");
   //Set 0x20 (DDRB bit 5 to 1)
   __asm__ volatile ("ldi r18,0x20");
   //Write 0x20 to location 0x24 (set LED pin as output)
   __asm__ volatile ("st Z, r18");

   //Set LED to ON
   //Set Z low byte to LED register
   __asm__ volatile ("ldi r30,0x25");
   
   __asm__ volatile ("ldi r18,0x20");
   __asm__ volatile ("st Z, r18");
}

/**
 * Turns LED off
 */
void led_off() {
   //Set data direction to OUTPUT
   //Clear Z high byte
   __asm__ volatile ("clr r31");
   //Set Z low byte to DDRB
   __asm__ volatile ("ldi r30,0x24");
   //Set 0x20 (DDRB bit 5 to 1)
   __asm__ volatile ("ldi r18,0x20");
   //Write 0x20 to location 0x24 (set LED pin as output)
   __asm__ volatile ("st Z, r18");

   //Set LED to OFF
   __asm__ volatile ("ldi r30,0x25");
   
   __asm__ volatile ("ldi r18,0x00");
   __asm__ volatile ("st Z, r18");
}
