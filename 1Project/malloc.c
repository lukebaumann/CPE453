#include "globals.h"

void *programBreak = 0;
header *head = NULL;
static uint32_t headerSize = sizeof(header) + 16 - sizeof(header) % 16;
char buffer[10000];

// Wrapper to call my malloc function
void *malloc(size_t size) {
   return myMalloc(size + PADDING);
}

// Wrapper to call my calloc function
void *calloc(size_t nmemb, size_t size) {
   myCalloc(nmemb, size + PADDING);
}

// Wrapper to call my free function
void free(void *ptr) {
   myFree(ptr);
}

// Wrapper to call my free function
void *realloc(void *ptr, size_t size) {
   myRealloc(ptr, size + PADDING);
}

// Allocates a section of memory in the heap and returns
// the pointer to the beginning of the block. There is always
// a header describing the space remaining in the heap as free
// at the end of the linked list of headers. 
void *myMalloc(size_t size) {
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

   return (void *) ceil16((uint32_t) headerPointer->allocatedBlock) ;
}

void *myCalloc(size_t nmemb, size_t size) {
   void *block = myMalloc(nmemb * size);
   memset(block, 0, nmemb * size);
   return block;
}

void myFree(void *ptr) {
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

void *myRealloc(void *ptr, size_t size) {
   if (ptr == NULL) {
      return myMalloc(size);
   }
   if (size == 0) {
      myFree(ptr);
      return NULL;
   }

   void *block = NULL;

   header *headerBefore = getBeforePointerFromPointer(ptr);
   header *headerPointer = getHeaderPointerFromBefore(headerBefore);

   if (headerPointer->next->next == NULL) {
      if (headerPointer->size + headerSize + headerPointer->next->size
       > size + headerSize) {
         headerPointer->size += headerSize + headerPointer->next->size;
         headerPointer->next = NULL;

         reallocFromTooBig(headerPointer, size);
      }
      else {
         headerPointer->size += headerSize + headerPointer->next->size;
         headerPointer->next = NULL;
         while (headerPointer->size <= headerSize + 10 + size) {
            if (sbrk(BREAK_INCREMENT) < 0) {
            }
            headerPointer->size += BREAK_INCREMENT;
         }

         reallocFromTooBig(headerPointer, size);
      }
   }
   else if (headerPointer == head) {
      if (headerPointer->next->freeFlag == TRUE &&
       headerPointer->size + headerSize + headerPointer->next->size > size) {
         reallocIntoNextHeader(headerPointer, size);
      }
      else {
         block = reallocIntoCompletelyNewBlock(headerPointer, size);
      }
   }
   else {
      // If after is free and big enough...
      if (headerPointer->next->freeFlag == TRUE &&
       headerPointer->size + headerSize + headerPointer->next->size > size) {
         reallocIntoNextHeader(headerPointer, size);
      }
      // If both before and after are free and big enough...
      else if (headerBefore->freeFlag == TRUE
       && headerPointer->next->freeFlag == TRUE
       && headerBefore->size + headerSize + headerPointer->size
       + headerSize + headerPointer->next->size > size) {
         reallocIntoPreviousAndNextHeader(headerBefore, size);
         headerPointer = headerBefore;
      }
      // If only before is free and big enough...
      else if (headerBefore->freeFlag == TRUE &&
       headerBefore->size + headerSize + headerPointer->size > size) {
         reallocIntoPreviousHeader(headerBefore, size);
         headerPointer = headerBefore;
      }
      // If neither before or after are free or big enough...
      else {
         block = reallocIntoCompletelyNewBlock(headerPointer, size);
      }
   }


   block = block == NULL ?
      (void *) ceil16((uint32_t) headerPointer->allocatedBlock) : block;

   return block;
}

void putChar(char c) {
   putchar(c);
   putchar('\n');
}

uint32_t ceil16(uint32_t i) {
   return i;
   //return i % 16 ? i + 16 - i % 16 : i;
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

void reallocFromTooBig(header *headerPointer, uint32_t size) {
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
   headerPointer->size += headerSize + headerPointer->next->size;
   headerPointer->next = headerPointer->next->next;

   // If there is enough room for another header, insert it.
   // Otherwise, live with the extra space
   if (headerPointer->size > size + headerSize) {
      reallocFromTooBig(headerPointer, size);
   }
}

void reallocIntoPreviousAndNextHeader(header *headerBefore, uint32_t size) {
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
   // To account for malloc's padding, we do not want to double pad
   void *block = myMalloc(size - 16);
   memmove(block, headerPointer->allocatedBlock, headerPointer->size);
   myFree(headerPointer->allocatedBlock);

   return block;
}
