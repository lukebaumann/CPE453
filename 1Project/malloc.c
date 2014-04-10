#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef struct header {
   uint8_t freeFlag;
   size_t size;
   void *allocatedBlock;
   struct header *next;
} header;

void *programBreak = 0;
header *head = NULL;
uint8_t heapSize = 0;
static uint8_t headerSize = sizeof(header);
#define BREAK_INCREMENT 10000000
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
   nextHeader->allocatedBlock =
    (void *) (nextHeader + 1); 
   nextHeader->freeFlag = TRUE;
   nextHeader->size = oldSize - headerSize - size;
   nextHeader->next = temp;
}

void mallocForTailHeader(header *headerPointer, size_t size) {
      // If it allocates the exact size of the heap,
      // it grows the heap to allow for the final free header
      while (headerPointer->size - size <= headerSize) {
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


