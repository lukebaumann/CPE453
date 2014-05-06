/**
 * File: program2.c
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 02
 * 04/25/2014
 */

#include "globals.h"
#include "os.h"

extern uint32_t oneSecondCounter;
extern struct system_t *system;

extern uint32_t tenMillisecondCounter = 0;
extern uint32_t oneSecondCounter = 0;

uint16_t bufferSize = 0;
uint16_t consumeTime = DEFAULT_CONSUME_TIME;
uint16_t produceTime = DEFAULT_PRODUCE_TIME;


/**
 * Ivokes the operating system.
 */
int main(void) {
   serial_init();
   os_init();

   create_thread((uint16_t) blink, 0, 50);
   create_thread((uint16_t) printSystemInfo, 0, 50);

   os_start();
   sei();
   while(1) {}
   
   return 0;
}

//global variables for producer and consumer
struct semaphore_t *full;
struct semaphore_t *empty;
struct mutex_t *mutex;
int buffCounter;
int buffer[MAX_BUFFER_SIZE];

//initial global variables
void initData() {
   sem_init(empty, MAX_BUFFER_SIZE);
   sem_init(full, 0);
   mutex_init(mutex);
   buffCounter = 0;
}

//insert item to the buffer
int insert(int item) {
   if (buffCounter < MAX_BUFFER_SIZE) {
      buffer[buffCounter] = item;
	  buffCounter++;
	  return 0;
   }
   else
      return -1;
}

void producer(int item) {
   while(1) {
      thread_sleep(produceTime);
	  sem_wait(empty);
	  mutex_lock(mutex);
	  insert(item);
	  mutex_unlock(mutex);
	  sem_signal(full);
	  led_on();
	  //might not need it since add a counter
      if (bufferSize < MAX_BUFFER_SIZE) {
         bufferSize++;
      }
   }
}

//remove the item from the buffer
int remove(int item) {
   if(buffCounter > 0) {
      item = buffer[(buffCounter-1)];
      buffCounter--;
      return 0;
   }
   else 
      return -1;
}

void consumer(int item) {
   while(1) {
      thread_sleep(consumeTime);
      sem_wait(full);
	  mutex_lock(mutex);
	  remove(item);
	  mutex_unlock(mutex);
	  sem_signal(empty);
	  led_off();
	  //might not need it since add a counter
	  if (bufferSize > 0) {
         bufferSize--;
      }
   }
}

void display_bounded_buffer() {
   set_cursor(0, 40);
   set_color(MAGENTA);
   print_string("Producing 1 item per ");
   print_int(produceTime * 10);
   print_string(" ms");

   set_cursor(1, 40);
   print_string("Consuming 1 item per ");
   print_int(consumeTime * 10);
   print_string(" ms");

   uint8_t i = 0;
   for (i = 0; i < bufferSize; i++) {
      set_cursor(2 + MAX_BUFFER_SIZE - i, 50);
      print_string("X");
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
void printSystemInfo() {
   while (1) {
      //_delay_ms(100);
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
      
	  uint8_t key = read_byte();
	  int item = 400;
	  //a 
	  if (key == 97) 
         producer(++item);
	  //z
      if (key == 122)	
         producer(--item);
	  //k
      if (key == 107)
         consumer(++item);
	  //m
      if (key == 109)
         consumer(--item);
		 
	  int i = 0;
      for (; i < system->numberOfThreads; i++) {
         set_color(BLACK + i);
         print_string("Thread ");
         print_int(system->threads[i].threadId);
         print_string("   ");
	     set_cursor(5, 1);

         print_string("Thread PC: 0x");
         print_hex(system->threads[i].functionAddress);
         print_string("   ");
	     set_cursor(6, 1);

         print_string("Stack usage: ");
         print_int((uint16_t) (system->threads[i].highestStackAddress
            - system->threads[i].stackPointer));
         print_string("   ");
	     set_cursor(7, 1);

         print_string("Total stack size: ");
         print_int(system->threads[i].stackSize);
         print_string("   ");
	     set_cursor(8, 1);

         print_string("Current top of stack: 0x");
         print_hex((uint16_t) system->threads[i].stackPointer);
         print_string("   ");
	     set_cursor(9, 1);

         print_string("Stack base: 0x");
         print_hex((uint16_t) system->threads[i].highestStackAddress);
         print_string("   ");
	     set_cursor(10, 1);

         print_string("Stack end: 0x");
         print_hex((uint16_t) system->threads[i].lowestStackAddress);
         print_string("   ");
	     set_cursor(11, 1);
      }
   }
}

/**
 * Flashes the LED on and then off with a frequency of 1 second.
 */
void blink(void) {
   volatile uint8_t numDelays = 5;
   volatile uint8_t i = 0;

   while (1) {
      // Then it turns off the LED and delays
      led_off();
      for (i = 0; i < numDelays; i++) {
         thread_sleep(5);
      }

      // Then it turns on the LED and delays
      led_on();
      for (i = 0; i < numDelays; i++) {
         thread_sleep(5);
      }
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
