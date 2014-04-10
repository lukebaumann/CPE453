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
   sprintf(buffer, "nextHeader: %p\nnextHeader->allocatedBlock: %p\nsbrk(0): %p\n", nextHeader, (void *) (nextHeader + 1), sbrk(0));
   puts(buffer);
   nextHeader->allocatedBlock =
    (void *) (nextHeader + 1); 
   nextHeader->freeFlag = TRUE;
   nextHeader->size = oldSize - headerSize - size;
   sprintf(buffer, "headerPointer->size: %d\noldSize: %d\nheaderSize: %d\nsize: %d\nnextHeader->size: %d\n", headerPointer->size, oldSize, headerSize, size, nextHeader->size);
   puts(buffer);
   nextHeader->next = temp;
}

void mallocForTailHeader(header *headerPointer, size_t size) {
      // If it allocates the exact size of the heap,
      // it grows the heap to allow for the final free header

      sprintf(buffer, "headerPointer->size: %d\nsize: %d\nheaderSize: %d\nheaderPointer->size - size <= headerSize + 10: %d\n\n", headerPointer->size, size, headerSize, headerPointer->size <= headerSize + 10 + size);
      puts(buffer);
      while (headerPointer->size <= headerSize + 10 + size) {
         putChar('a');
         if (sbrk(BREAK_INCREMENT) < 0) {
         }
         sprintf(buffer, "old headerPointer->size: %d\n", headerPointer->size);
         puts(buffer);
         headerPointer->size += BREAK_INCREMENT;
         sprintf(buffer, "new headerPointer->size: %d\n", headerPointer->size);
         puts(buffer);
      }

      sprintf(buffer, "headerPointer: %p\nheaderPointer->allocatedBlock: %p\nheaderPointer->size: %d\nsize: %d\nnextHeader: %p\nnextHeader->allocatedBlock: %p\n\nsbrk(0): %p\n", headerPointer, headerPointer->allocatedBlock, headerPointer->size, size, headerPointer->next, (void *) (headerPointer->next + 1), sbrk(0));
      puts(buffer);

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
      while ((uint8_t *) headerBefore->next->next > (uint8_t * ) ptr) {
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

void *realloc(void *ptr, size_t size) {
   if (ptr == NULL) {
      return malloc(size);
   }
   if (size == 0) {
      free(ptr);
      return NULL;
   }

   header *headerBefore = getBeforePointerFromPointer(ptr);
   header *headerPointer = getHeaderPointerFromBefore(headerBefore);

   return NULL;
}


