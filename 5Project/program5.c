/**
 * File: program5.c
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 05
 * 04/25/2014
 */

#include "globals.h"
#include "os.h"
#include "SdReader.h"

extern volatile struct system_t *system;
extern volatile uint32_t tenMillisecondCounter;
extern volatile uint32_t oneSecondCounter;

static struct mutex_t buffer1Mutex;
static struct mutex_t buffer2Mutex;

static uint8_t *buffer[NUMBER_OF_BUFFERS];
uint8_t playBuffer = 0;
uint8_t playBufferIndex = 0;
uint8_t readBuffer = 0;
uint8_t readBufferIndex = 0;
static uint8_t readComplete = 0;

static uint32_t block = 0;
static uint8_t offset = 0;

/**
 * Ivokes the operating system and is the idle thread.
 */
void main() {
   uint8_t i = 0;
   for (i = 0; i < NUMBER_OF_BUFFERS; i++) {
      buffer[i] = malloc(BUFFER_SIZE);
   }

   if (!sdInit(0)) {
      if (!sdInit(1)) {
         exit(-1);
      }
   }

   start_audio_pwm();

   ext2_init();
   serial_init();
   os_init();


   create_thread((uint16_t) playback, 0, 50);
   create_thread((uint16_t) reader, 0, 51);
   create_thread((uint16_t) display_stats, 0, 52);

   mutex_init(&buffer1Mutex);
   mutex_init(&buffer2Mutex);

   clear_screen();
   os_start();
   sei();

   while(1) {}
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
      if (key == 'n') {
      }
      else if (key == 'p') {
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
 * Writes a byte to the digital output pin (3) and then yields.
 */
void playback(void) {
   struct mutex_t *playMutex; 
   while (1) {
      if (playBuffer == 0) {
         playMutex = &buffer1Mutex;
      }
      else if (playBuffer == 1) {
         playMutex = &buffer2Mutex;
      }

      mutex_lock(playMutex);

      OCR2B = buffer[playBuffer][playBufferIndex];

      mutex_unlock(playMutex);

      yield();
   }
 }

 /**
  *
  */
void reader(void) {
   struct mutex_t *readMutex; 
   while (1) {
      if (readComplete)
         yield();
      else {
         if (readBuffer == 0) {
            readMutex = &buffer1Mutex;
         }
         else if (readBuffer == 1) {
            readMutex = &buffer2Mutex;
         }

         mutex_lock(readMutex);


         // Do read here

         mutex_unlock(readMutex);
      }
   }
}

// These functions will go in their own file later but for now will be right here
int inodesPerGroup = 0;
int sectorsPerGroup = 0;

void ext2_init() {
   struct ext2_super_block sb;

   findSuperBlock(&sb);
   inodesPerGroup = sb.s_inodes_per_group;
   sectorsPerGroup = 2 * sb.s_blocks_per_group;

}
void findSuperBlock(struct ext2_super_block *sb) {
   read_data(2 * SUPER_BLOCK_INDEX, 0, (uint8_t *) sb,
         sizeof(struct ext2_super_block));

   return;
}
