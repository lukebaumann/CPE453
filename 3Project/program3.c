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
static uint16_t bufferSize = 0;
static uint16_t consumeTime = DEFAULT_CONSUME_TIME;
static uint16_t produceTime = DEFAULT_PRODUCE_TIME;

void test() {
   while(1) {}
}

/**
 * Ivokes the operating system.
 */
int main(void) {
   serial_init();
   os_init();

   create_thread((uint16_t) test, 0, 50);
   create_thread((uint16_t) consumer, 0, 50);
   create_thread((uint16_t) producer, 0, 50);
   create_thread((uint16_t) display_stats, 0, 50);
   create_thread((uint16_t) display_bounded_buffer, 0, 50);
   create_thread((uint16_t) blink, 0, 50);
   //create_main

   sem_init(&bufferSemaphore, 1);

   os_start();
   sei();
   while(1) {}
   
   return 0;
}

void producer() {
   while(1) {
      thread_sleep(produceTime);

      if (bufferSize < MAX_BUFFER_SIZE) {
         set_cursor(20, 50);
         print_string("produce wait              ");
         sem_wait(&bufferSemaphore);
         set_cursor(20, 50);
         print_string("             produce ready");
         bufferSize++;
         sem_signal(&bufferSemaphore);
      }
   }
}

void consumer() {
   while(1) {
      thread_sleep(consumeTime);

      if (bufferSize > 0) {
         set_cursor(21, 50);
         print_string("consume wait              ");
         sem_wait(&bufferSemaphore);
         set_cursor(21, 50);
         print_string("             consume ready");
         bufferSize--;
         sem_signal(&bufferSemaphore);
      }
   }
}

void display_bounded_buffer() {
   while (1) {
      handleKeys();

      set_cursor(1, 40);
      set_color(MAGENTA);
      print_string("Producing 1 item per ");
      print_int(produceTime * 10);
      print_string(" ms   ");

      set_cursor(2, 40);
      print_string("Consuming 1 item per ");
      print_int(consumeTime * 10);
      print_string(" ms   ");

      uint8_t i = 0;
      for (i = 0; i < MAX_BUFFER_SIZE; i++) {
         set_cursor(3 + MAX_BUFFER_SIZE - i, 50);
         if (i < bufferSize) {
            print_string("X");
         }
         else {
            print_string(" ");
         }
      }

      thread_sleep(5);
   }
}

void handleKeys() {
   uint8_t key = read_byte();

   if (key != 255) {
      if (key == 'a') {
         produceTime++;
      }
      if (key == 'z') {
         produceTime--;
      }
      if (key == 'k') {
         consumeTime++;
      }
      if (key == 'm') {
         consumeTime--;
      }
   }
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
         print_string("   ");
         set_cursor(5 + i * STAT_DISPLAY_HEIGHT, 1);

         print_string("Thread PC: 0x");
         print_hex(system->threads[i].functionAddress);
         print_string("   ");
         set_cursor(6 + i * STAT_DISPLAY_HEIGHT, 1);

         print_string("Stack usage: ");
         print_int((uint16_t) (system->threads[i].highestStackAddress -
          system->threads[i].stackPointer));
         print_string("   ");
         set_cursor(7 + i * STAT_DISPLAY_HEIGHT, 1);

         print_string("Total stack size: ");
         print_int(system->threads[i].stackSize);
         print_string("   ");
         set_cursor(8 + i * STAT_DISPLAY_HEIGHT, 1);

         print_string("Current top of stack: 0x");
         print_hex((uint16_t) system->threads[i].stackPointer);
         print_string("   ");
         set_cursor(9 + i * STAT_DISPLAY_HEIGHT, 1);

         print_string("Stack base: 0x");
         print_hex((uint16_t) system->threads[i].highestStackAddress);
         print_string("   ");
         set_cursor(10 + i * STAT_DISPLAY_HEIGHT, 1);

         print_string("Stack end: 0x");
         print_hex((uint16_t) system->threads[i].lowestStackAddress);
         print_string("   ");
         set_cursor(11 + i * STAT_DISPLAY_HEIGHT, 1);
      }
   }
}

/**
 * Flashes the LED on and then off with a frequency of 1 second.
 */
void blink(void) {
   while (1) {
      if (bufferSize < MAX_BUFFER_SIZE && produceTime > 0) {
         led_on();
      }
      else {
         led_off();
      }

      yield();
   }
}

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
