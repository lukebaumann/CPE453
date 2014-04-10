#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

void putChar(char c) {
   putchar(c);
   putchar('\n');
}

typedef struct header {
   uint8_t freeFlag;
   uint32_t size;
   void *allocatedBlock;
   struct header *next;
} header;

void *programBreak = 0;
header *head = NULL;
static uint32_t headerSize = sizeof(header);
char buffer[10000];
#define BREAK_INCREMENT 0x100
#define TRUE 1
#define FALSE 0


// The last header is always free and fills the rest of the space in the heap
void *calloc(size_t nmemb, size_t size) {
   void *block = malloc(nmemb * size);
   memset(block, 0, nmemb * size);
   return block;
}

void makeHeader(header *headerPointer) {
   headerPointer->allocatedBlock =
    (void *) (headerPointer + 1); 
   headerPointer->freeFlag = TRUE;
   headerPointer->size = BREAK_INCREMENT - headerSize;
   headerPointer->next = NULL;
}

void doMalloc(header *headerPointer, size_t size) {
   header *nextHeader;
   header *temp = headerPointer->next;
   size_t oldSize = headerPointer->size;
   
   headerPointer->freeFlag = FALSE;
   headerPointer->size = size;
   headerPointer->next =
    (header *) (((uint8_t *) headerPointer->allocatedBlock)
    + headerPointer->size);

   nextHeader = headerPointer->next;
   //sprintf(buffer, "nextHeader: %p\nnextHeader->allocatedBlock: 
   //%p\nsbrk(0): %p\n", nextHeader, (void *) (nextHeader + 1), sbrk(0));
   //puts(buffer);
   nextHeader->allocatedBlock =
    (void *) (nextHeader + 1); 
   nextHeader->freeFlag = TRUE;
   nextHeader->size = oldSize - headerSize - size;
   //sprintf(buffer, "headerPointer->size: %d\noldSize:
   //%d\nheaderSize: %d\nsize: %d\nnextHeader->size: %d\n",
   //headerPointer->size, oldSize, headerSize, size, nextHeader->size);
   //puts(buffer);
   nextHeader->next = temp;
}

void mallocForTailHeader(header *headerPointer, size_t size) {
      // If it allocates the exact size of the heap,
      // it grows the heap to allow for the final free header

      //sprintf(buffer, "headerPointer->size: %d\nsize: %d\n
      //headerSize: %d\nheaderPointer->size - size <=
      //headerSize + 10: %d\n\n", headerPointer->size, size,
      //headerSize, headerPointer->size <= headerSize + 10 + size);
      //puts(buffer);
      while (headerPointer->size <= headerSize + 10 + size) {
         if (sbrk(BREAK_INCREMENT) < 0) {
         }
         headerPointer->size += BREAK_INCREMENT;
      }

      doMalloc(headerPointer, size);
}

header *firstMalloc(size_t size) {
   header *headerPointer = (header *) sbrk(BREAK_INCREMENT);
   makeHeader(headerPointer);
   head = headerPointer;

   mallocForTailHeader(headerPointer, size);

   return headerPointer;
}

void *malloc(size_t size) {
   if (size <= 0) {
      return NULL;
   }

   header *headerPointer = head;
   header *temp;

   if (headerPointer == NULL) {
      headerPointer = firstMalloc(size);
   }
   else {
      while (headerPointer->next != NULL && 
         !(headerPointer->freeFlag == TRUE && headerPointer->size >= size)) {
         headerPointer = headerPointer->next;
      }

      if (headerPointer->next == NULL) { 
         mallocForTailHeader(headerPointer, size);
      }
      else {
         // If there is enough space for my size and the size of the next
         // header I need to make, use the space and make a new header to
         // divide the block 
         if (headerPointer->size - size > headerSize) {
            doMalloc(headerPointer, size);
         }
         // Else use the original block with its size that might be a
         // little bigger than what was asked for
         else {
            headerPointer->freeFlag = FALSE;
         }
      }
   }

   return headerPointer->allocatedBlock;
}

header *getBeforePointerFromPointer(void *ptr) {
   header *headerBefore = head;
   if ((uint8_t *) ptr < (uint8_t *) headerBefore->next) {
      return NULL;
   }
   else {
      while ((uint8_t *) headerBefore->next->next < (uint8_t * ) ptr) {
         headerBefore = headerBefore->next;
      }
   }

   return headerBefore;
}

header *getHeaderPointerFromBefore(header *headerBefore) {
   header *headerPointer;

   if (headerBefore == NULL) {
      headerPointer = head;
   }
   else {
      headerPointer = headerBefore->next;
   }

   return headerPointer;
}

void free(void *ptr) {
   if (ptr == NULL) {
      return;
   }

   header *headerBefore = getBeforePointerFromPointer(ptr);
   header *headerPointer = getHeaderPointerFromBefore(headerBefore);


   if (headerPointer == head) {
      if (headerPointer->next == NULL) {
         headerPointer->freeFlag = TRUE;
      }
      else {
         if (headerPointer->next->freeFlag == FALSE) {
            headerPointer->freeFlag = TRUE;
         }
         else {
            headerPointer->size += headerSize + headerPointer->next->size;
            headerPointer->freeFlag = TRUE;
            headerPointer->next = headerPointer->next->next;
         }
      }
   }
   else {
      if (headerPointer->next == NULL) {
         if (headerBefore->freeFlag == FALSE) {
            headerPointer->freeFlag = TRUE;
         }
         else {
            headerBefore->size += headerSize + headerPointer->size;
            headerBefore->next = NULL;
         }
      }
      else {
         if (headerBefore->freeFlag == TRUE) {
            if (headerPointer->next->freeFlag == TRUE) {
               headerBefore->size += headerSize + headerPointer->size +
                headerSize + headerPointer->next->size;
               headerBefore->next = headerPointer->next->next;
            }
            else {
               headerBefore->size += headerSize + headerPointer->size;
               headerBefore->next = headerPointer->next;
            }
         }
         else {
            if (headerPointer->next->freeFlag == TRUE) {
               headerPointer->size += headerSize + headerPointer->next->size;
               headerPointer->freeFlag = TRUE;
               headerPointer->next = headerPointer->next->next;
            }
            else {
               headerPointer->freeFlag = TRUE;
            }
         }
      }
   }
}

void reallocFromTooBig(header *headerPointer, uint32_t size) {
   putChar('L');
   header *temp;
   header *nextHeader;
   uint32_t oldSize;

   temp = headerPointer->next;
   oldSize = headerPointer->size;

   headerPointer->size = size;
   headerPointer->next =
    (header *) (((uint8_t *) headerPointer->allocatedBlock)
    + headerPointer->size);

   nextHeader = headerPointer->next;
   nextHeader->allocatedBlock =
    (void *) (nextHeader + 1); 
   nextHeader->freeFlag = TRUE;
   nextHeader->size = oldSize - headerSize - size;
   nextHeader->next = temp;
}

void reallocIntoNextHeader(header *headerPointer, uint32_t size) {
   putChar('N');
   headerPointer->size += headerSize + headerPointer->next->size;
   headerPointer->next = headerPointer->next->next;

   // If there is enough room for another header, insert it.
   // Otherwise, live with the extra space
   if (headerPointer->size > size + headerSize) {
      reallocFromTooBig(headerPointer, size);
   }
}

void reallocIntoPreviousAndNextHeader(header *headerBefore, uint32_t size) {
   putChar('B');
   header *temp = headerBefore->next->next->next;

   headerBefore->size += headerSize + headerBefore->next->size + headerSize
    + headerBefore->next->next->size;
   
   memmove(headerBefore->allocatedBlock, 
    headerBefore->next->allocatedBlock, headerBefore->next->size);
   
   headerBefore->next = temp;

   if (headerBefore->size > size + headerSize) {
      reallocFromTooBig(headerBefore, size);
   }
}

void reallocIntoPreviousHeader(header *headerBefore, uint32_t size) {
   putChar('P');
   header *temp = headerBefore->next->next;

   headerBefore->size += headerSize + headerBefore->next->size;
   
   memmove(headerBefore->allocatedBlock, 
    headerBefore->next->allocatedBlock, headerBefore->next->size);
   
   headerBefore->next = temp;

   if (headerBefore->size > size + headerSize) {
      reallocFromTooBig(headerBefore, size);
   }
}

void *reallocIntoCompletelyNewBlock(header *headerPointer, uint32_t size) {
   putChar('C');
   void *block = malloc(size);
   memmove(block, headerPointer->allocatedBlock, headerPointer->size);
   free(headerPointer->allocatedBlock);

   return block;
}

void *realloc(void *ptr, size_t size) {
   if (ptr == NULL) {
      return malloc(size);
   }
   if (size == 0) {
      free(ptr);
      return NULL;
   }

   void *block = NULL;

   header *headerBefore = getBeforePointerFromPointer(ptr);
   header *headerPointer = getHeaderPointerFromBefore(headerBefore);

   putChar('5');
   sprintf(buffer, "ptr: %p\n", ptr);
   puts(buffer);
   sprintf(buffer, "headerPointer: %p\n", headerPointer);
   puts(buffer);
   sprintf(buffer, "headerPointer->next: %p\n", headerPointer->next);
   puts(buffer);
   sprintf(buffer, "headerPointer->next->next: %p\n", headerPointer->next->next);
   puts(buffer);


   if (headerPointer->next->next == NULL) {
      putChar('a');
      if (headerPointer->size + headerSize + headerPointer->next->size
       > size + headerSize) {
      putChar('b');
         headerPointer->size += headerSize + headerPointer->next->size;
         headerPointer->next = NULL;

      putChar('c');
         reallocFromTooBig(headerPointer, size);
      putChar('d');
      }
      else {
      putChar('e');
         headerPointer->size += headerSize + headerPointer->next->size;
         headerPointer->next = NULL;
      putChar('f');
         while (headerPointer->size <= headerSize + 10 + size) {
            if (sbrk(BREAK_INCREMENT) < 0) {
            }
      putChar('g');
            headerPointer->size += BREAK_INCREMENT;
         }

      putChar('h');
         reallocFromTooBig(headerPointer, size);
      putChar('i');
      }
   }
   else if (headerPointer == head) {
   putChar('9');
      // Never will happen
      //if (headerPointer->next == NULL) {
      //   headerPointer->freeFlag = TRUE;
      //}
      //else {
         if (headerPointer->next->freeFlag == TRUE &&
          headerPointer->size + headerSize + headerPointer->next->size > size) {
   putChar('8');
            reallocIntoNextHeader(headerPointer, size);
   putChar('8');
         }
         else {
   putChar('1');
            block = reallocIntoCompletelyNewBlock(headerPointer, size);
   putChar('1');
         }
      //}
   }
   else {
      // Never will happen
      //if (headerPointer->next == NULL) {
      //   if (headerBefore->freeFlag == FALSE) {
      //      headerPointer->freeFlag = TRUE;
      //   }
      //   else {
      //      headerBefore->size += headerSize + headerPointer->size;
      //      headerBefore->next = NULL;
      //   }
      //}
      //else {
         // If after is free and big enough...
   putChar('3');
         if (headerPointer->next->freeFlag == TRUE &&
          headerPointer->size + headerSize + headerPointer->next->size > size) {
   putChar('4');
            reallocIntoNextHeader(headerPointer, size);
   putChar('4');
         }
         // If both before and after are free and big enough...
         else if (headerBefore->freeFlag == TRUE
          && headerPointer->next->freeFlag == TRUE
          && headerBefore->size + headerSize + headerPointer->size
          + headerSize + headerPointer->next->size > size) {
   putChar('5');
            reallocIntoPreviousAndNextHeader(headerBefore, size);
            headerPointer = headerBefore;
   putChar('5');
         }
         // If only before is free and big enough...
         else if (headerBefore->freeFlag == TRUE &&
          headerBefore->size + headerSize + headerPointer->size > size) {
   putChar('6');
            reallocIntoPreviousHeader(headerBefore, size);
            headerPointer = headerBefore;
   putChar('6');
         }
         // If neither before or after are free or big enough...
         else {
   putChar('7');
            block = reallocIntoCompletelyNewBlock(headerPointer, size);
   putChar('7');
         }
      //}
   }

   return block == NULL ? headerPointer->allocatedBlock : block;
}


