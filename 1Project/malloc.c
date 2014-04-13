#include "globals.h"

void *programBreak = 0;
header *head = NULL;
static uint32_t headerSize = sizeof(header) + 16 - sizeof(header) % 16;
char buffer[10000];

// Wrapper to call my malloc function
void *malloc(size_t size) {
   if (size <= 0) {
      return NULL;
   }

   return myMalloc(size + PADDING);
}

// Wrapper to call my calloc function
void *calloc(size_t nmemb, size_t size) {
   return myCalloc(nmemb, size);
}

// Wrapper to call my free function
void free(void *ptr) {
   myFree(ptr);
}

// Wrapper to call my free function
void *realloc(void *ptr, size_t size) {
   // If ptr is NULL, realloc behaves as malloc
   if (ptr == NULL) {
      return malloc(size);
   }

   // If size is 0, realloc behaves as free
   if (size == 0) {
      myFree(ptr);
      return NULL;
   }

   return myRealloc(ptr, size + PADDING);
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
      // While my header is not the last header and I am either not free
      // or too small, continue down the linked list
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

   // Align the pointer that is returned from malloc. This is safe to
   // do because the extra padding that is given is large enough to
   // allow the first bytes that block point to to be unused
   return (void *) ceil16((intptr_t) (headerPointer + 1));
}

// Calls my malloc function with a pad and sets only the 
// data that I am focused on using to be equal to 0
void *myCalloc(size_t nmemb, size_t size) {
   void * block = myMalloc(nmemb * (size + PADDING));
   memset(block, 0, nmemb * size);
   return block;
}

// Frees the block of memory that the ptr is a part of.
// If ptr is not within a previously allocated block,
// the behavior is undefined.
void myFree(void *ptr) {
   if (ptr == NULL) {
      return;
   }

   header *headerBefore = getBeforePointerFromPointer(ptr);
   header *headerPointer = getHeaderPointerFromBefore(headerBefore);

   if (headerPointer == head) {
      freeHead(headerPointer);
   }
   else {
      if (headerBefore->freeFlag == TRUE) {
         // If the previous and next blocks are free, I need to
         // combine all of them
         if (headerPointer->next->freeFlag == TRUE) {
            headerBefore->size += headerSize + headerPointer->size +
             headerSize + headerPointer->next->size;
            headerBefore->next = headerPointer->next->next;
         }
         // If only the previous block is free, I need to combine it
         // and the current block
         else {
            headerBefore->size += headerSize + headerPointer->size;
            headerBefore->next = headerPointer->next;
         }
      }
      else {
         // If only the next block is free, I need to combine it
         // and the current block
         if (headerPointer->next->freeFlag == TRUE) {
            headerPointer->size += headerSize + headerPointer->next->size;
            headerPointer->freeFlag = TRUE;
            headerPointer->next = headerPointer->next->next;
         }
         // If the previous and next blocks are not free, I do not
         // need to combine
         else {
            headerPointer->freeFlag = TRUE;
         }
      }
   }
}

// Reallocates the block pointed to
void *myRealloc(void *ptr, size_t size) {
   void *block = NULL;

   header *headerBefore = getBeforePointerFromPointer(ptr);
   header *headerPointer = getHeaderPointerFromBefore(headerBefore);

   if (headerPointer->next->next == NULL) {
      block = reallocateSecondToLastBlock(headerPointer, headerBefore, size);
   }
   else if (headerPointer == head) {
      block = reallocFirstBlock(headerPointer, size);
   }
   else {
      block = reallocMiddleBlock(headerPointer, headerBefore, size);
   }

   block = block == NULL ?
      (void *) ceil16((intptr_t) (headerPointer + 1)) : block;

   return block;
}

// Ceiling function used for aligning pointer addresses
void *ceil16(intptr_t i) {
   return (void *) (i % 16 ? i + 16 - i % 16 : i);
}

// Does the first sbrk, makes the first header, and mallocs the
// first block
header *firstMalloc(size_t size) {
   header *headerPointer = (header *) sbrk(BREAK_INCREMENT);
   makeHeader(headerPointer);
   head = headerPointer;

   mallocForTailHeader(headerPointer, size);

   return headerPointer;
}

// Sets the head header
void makeHeader(header *headerPointer) {
   headerPointer->freeFlag = TRUE;
   headerPointer->size = BREAK_INCREMENT - headerSize;
   headerPointer->next = NULL;
}

// Does a single malloc on a headerPointer that is ready
// free and large enough. It breaks the current free block
// and adds a header in the middle
void doMalloc(header *headerPointer, size_t size) {
   header *nextHeader;
   header *temp = headerPointer->next;
   size_t oldSize = headerPointer->size;
   
   headerPointer->freeFlag = FALSE;
   headerPointer->size = size;
   headerPointer->next =
    (header *) (((uint8_t *) (headerPointer + 1))
    + headerPointer->size);

   nextHeader = headerPointer->next;
   nextHeader->freeFlag = TRUE;
   nextHeader->size = oldSize - headerSize - size;
   nextHeader->next = temp;
}

// Does the malloc for the last block in the linked list.
// Ensures that there is a final block that is free and fills
// the rest of the the heap
void mallocForTailHeader(header *headerPointer, size_t size) {
   while (headerPointer->size <= headerSize + size) {
      if (sbrk(BREAK_INCREMENT) < 0) {
      }
      headerPointer->size += BREAK_INCREMENT;
   }

   doMalloc(headerPointer, size);
}

// Goes through the linked list of allocated and free blocks
// looking for the header for the block before the pointer's
// block. If the pointer is not in the list, behavior is undefined
header *getBeforePointerFromPointer(void *ptr) {
   header *headerBefore = head;
   
   // If the pointer is part of the head header's block,
   // there is no header before it
   if ((uint8_t *) ptr < ((uint8_t *) (head + 1)) + head->size) {
      return NULL;
   }
   else {
      while ((uint8_t *) headerBefore->next->next < (uint8_t * ) ptr) {
         headerBefore = headerBefore->next;
      }
   }

   return headerBefore;
}

// If the header before does not exist, the current is the head.
// Otherwise, it is the next header
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

// Make a new header if there is enough room to. Otherwise, do nothing
void reallocFromTooBig(header *headerPointer, uint32_t size) {
   header *temp;
   header *nextHeader;
   uint32_t oldSize;

   if (headerPointer->size > size + headerSize) {
      temp = headerPointer->next;
      oldSize = headerPointer->size;

      headerPointer->size = size;
      headerPointer->next =
       (header *) (((uint8_t *) (headerPointer + 1))
       + headerPointer->size);

      nextHeader = headerPointer->next;
      nextHeader->freeFlag = TRUE;
      nextHeader->size = oldSize - headerSize - size;
      nextHeader->next = temp;
   }
}

// Used for the senario where the next block is free. It will add it
// to itself and then call reallocFromTooBig(...)
void reallocIntoNextHeader(header *headerPointer, uint32_t size) {
   headerPointer->size += headerSize + headerPointer->next->size;
   headerPointer->next = headerPointer->next->next;

   reallocFromTooBig(headerPointer, size);
}

// Used for the senario where the previous and next block is free.
// It will add itself and the next block to the previous block,
// move the data to the previous block, and then call reallocFromTooBig(...)
void reallocIntoPreviousAndNextHeader(header *headerBefore, uint32_t size) {
   header *temp = headerBefore->next->next->next;

   headerBefore->size += headerSize + headerBefore->next->size + headerSize
    + headerBefore->next->next->size;
   
   myMemmove(headerBefore + 1, 
    headerBefore->next + 1, headerBefore->next->size);
   
   headerBefore->next = temp;

   reallocFromTooBig(headerBefore, size);
}

// Used for the senario where the previous block is free.
// It will add itself to the previous block, move the data
// to the previous block, and then call reallocFromTooBig(...)
void reallocIntoPreviousHeader(header *headerBefore, uint32_t size) {
   header *temp = headerBefore->next->next;

   headerBefore->size += headerSize + headerBefore->next->size;
   
   myMemmove(headerBefore + 1, 
    headerBefore->next + 1, headerBefore->next->size);
   
   headerBefore->next = temp;

   reallocFromTooBig(headerBefore, size);
}

// Used when the current block needs to grow and there are no
// adjacent free blocks large enough to grow into. Instead, a new block 
// will be allocated and moved into. Then the orginal block will be freed
void *reallocIntoCompletelyNewBlock(header *headerPointer, uint32_t size) {
   void *block = myMalloc(size);
   myMemmove(block, headerPointer + 1, headerPointer->size);
   myFree(headerPointer + 1);

   return block;
}

// If I am the first block
void *reallocFirstBlock(header *headerPointer, size_t size) {
   void *block = NULL;

   // and the next block is free and big enough, realloc into it
   if (headerPointer->next->freeFlag == TRUE &&
    headerPointer->size + headerSize + headerPointer->next->size > size) {
      reallocIntoNextHeader(headerPointer, size);
   }
   // Otherwise, realloc into a brand new place and free the old block
   else {
      block = reallocIntoCompletelyNewBlock(headerPointer, size);
   }     

   return block;
}

// If I am a middle block
void *reallocMiddleBlock(header *headerPointer,
 header *headerBefore, size_t size) {
   void *block = NULL;

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
      // and I am downsizing
      if (headerPointer->size >= size) {
         reallocFromTooBig(headerPointer, size);
      }
      // If I am expanding, realloc into a new block
      else {
         block = reallocIntoCompletelyNewBlock(headerPointer, size);
      }
   }

   return block;
}

// If I am the last used block, merge with the last block,
// and possibly call sbrk until the block is big enough, then
// reallocate from the block now that it is too big
void *reallocateSecondToLastBlock(header *headerPointer,
 header *headerBefore, size_t size) {
   void *block = NULL;

   headerPointer->size += headerSize + headerPointer->next->size;
   headerPointer->next = NULL;

   if (headerPointer->size <= size + headerSize) {
      while (headerPointer->size <= headerSize + size) {
         if (sbrk(BREAK_INCREMENT) < 0) {
         }
         headerPointer->size += BREAK_INCREMENT;
      }
   }

   reallocFromTooBig(headerPointer, size);
   
   return block;
}

void freeHead(header *headerPointer) {
   // If the next header is used, no need to combine blocks
   if (headerPointer->next->freeFlag == FALSE) {
      headerPointer->freeFlag = TRUE;
   }
   // If the next header is free, I need to combined myself with
   // the header in front of me.
   else {
      headerPointer->size += headerSize + headerPointer->next->size;
      headerPointer->freeFlag = TRUE;
      headerPointer->next = headerPointer->next->next;
   }
}

// This is my version of memmove. It is necessary because the
// allocated blocks might not be aligned with respect to each other
void myMemmove(void * destination, void *source, uint32_t size) {
   return memmove((void *) ceil16((intptr_t) destination),
    (void *) ceil16((intptr_t) source), size - PADDING);
}
