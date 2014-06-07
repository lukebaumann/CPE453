/**
 * File: program5.c
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 05
 * 06/03/2014
 */

#include "globals.h"
#include "os.h"
#include "SdReader.h"

extern volatile struct system_t *system;
//extern volatile uint32_t tenMillisecondCounter;
//extern volatile uint32_t oneSecondCounter;

static struct mutex_t buffer1Mutex;
static struct mutex_t buffer2Mutex;

static uint8_t *buffer[NUMBER_OF_BUFFERS];
uint8_t playBuffer = 0;
uint8_t playBufferIndex = 0;
uint8_t readBuffer = 1;
uint8_t buffer1[BUFFER_SIZE] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};
uint8_t buffer2[BUFFER_SIZE] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

struct ext2_dir_entry *entries[MAX_NUMBER_OF_ENTRIES];
static uint8_t entriesIndex = 0;
static uint32_t numberOfEntries = 0;
uint8_t readComplete = 0;

//static uint32_t block = 0;
//static uint8_t offset = 0;

void idle_thread(void) {
   while (1);
}

/**
 * Ivokes the operating system and is the idle thread.
 */
int main() {
   serial_init();

   buffer[0] = buffer1;
   buffer[1] = buffer2;

   //print_string("Before sdinit()\n\r");

   // if (!sdInit(0)) {
   //    if (!sdInit(1)) {
   //       exit(-1);
   //    }
   // }

   sdInit(0);

   //print_string("Before ext2_init\n\r");

   numberOfEntries = ext2_init(entries);
   print_string("Right Before create_thread()s\n\r");
   os_init();

   create_thread((uint16_t) playback, 0, 50);
   create_thread((uint16_t) reader, 0, 500);
   //create_thread((uint16_t) display_stats, 0, 52);
   create_thread((uint16_t) idle_thread, 0, 53);

   print_string("After create_thread()s\n\r");

   mutex_init(&buffer1Mutex);
   mutex_init(&buffer2Mutex);

   print_string("Before startaudio\n\r");
   start_audio_pwm();

   //clear_screen();
   print_string("Right before os_start\n\r");
   os_start();
   print_string("Right after os_start. Should not ever see this\n\r");
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
      yield();
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
   set_cursor(50, 0);
   print_string("In playback\n\r");
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
   set_cursor(51, 0);
   print_string("In reader\n\r");
   struct mutex_t *readMutex;
   uint8_t timesRead = 4; //4 is the value at which we request a new block
   uint32_t blockToRead = -1;
   struct ext2_inode songInode;

   findInode(&songInode, entries[entriesIndex]->inode);

   while (1) {
      if (readComplete)
         yield();
      else {
         if (0 == readBuffer) {
            readMutex = &buffer1Mutex;
         }
         else if (1 == readBuffer) {
            readMutex = &buffer2Mutex;
         }

         if (4 == timesRead) {
            timesRead = 0;

            set_cursor(52,0);
            print_string("About to getNextBlockNumber\n\r");
            blockToRead = getNextBlockNumber(&songInode);

            if (!blockToRead) {
               entriesIndex = (entriesIndex + 1) % numberOfEntries;

               //Get next song
               findInode(&songInode, entries[entriesIndex]->inode);
               blockToRead = getNextBlockNumber(&songInode);
            }
            print_int(blockToRead);
            print_string("\n\n\r");
         }

         mutex_lock(readMutex);

         sdReadData(blockToRead, BUFFER_SIZE * timesRead, buffer[readBuffer],
            BUFFER_SIZE);

         mutex_unlock(readMutex);

         readComplete = 1;
         timesRead++;
      }
   }
}
