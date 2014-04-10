#include <unistd.h>
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
header *head;
uint8_t heapSize = 0;
static uint8_t headerSize = sizeof(header);
#define BREAK_INCREMENT 65536
#define TRUE 1
#define FALSE 0


// The last header is always free and fills the rest of the space in the heap
void *calloc(size_t nmemb, size_t size) {
   return 0;
}

uint32_t ceil16(uint32_t i) {
   return i % 16 ? i + 16 - i % 16: i;
}

void makeHeader(header *headerPointer) {
   headerPointer->allocatedBlock =
    (void *) (((uint8_t *) headerPointer) + ceil16(headerSize)); 
   headerPointer->freeFlag = TRUE;
   headerPointer->size = BREAK_INCREMENT - headerSize;
   headerPointer-> next = NULL;
}

void doMalloc(header *headerPointer, size_t size) {
   header *nextHeader;
   header *temp = headerPointer;
   size_t currentSize = headerPointer->size;
   
   headerPointer->freeFlag = FALSE;
   headerPointer->size = (size);
   headerPointer->next =
    (header *) (((uint8_t *) headerPointer->allocatedBlock)
    + headerPointer->size);

   nextHeader = headerPointer->next;
   nextHeader->allocatedBlock =
    (void *) (((uint8_t *) nextHeader) + ceil16(headerSize)); 
   nextHeader->freeFlag = TRUE;
   nextHeader->size = currentSize - headerSize - headerPointer->size;
   nextHeader->next = temp->next;
}

void *malloc(size_t size) {
   header *headerPointer = head;
   header *temp;

   if (headerPointer == NULL) {
      headerPointer = (header *) sbrk(0);
      sbrk(BREAK_INCREMENT);
      makeHeader(headerPointer);
      head = headerPointer;

      doMalloc(headerPointer, size);
   }
   else {
      while (headerPointer->next != NULL && 
         !(headerPointer->freeFlag == TRUE && headerPointer->size >= size)) {
         headerPointer = headerPointer->next;
      }

      if (headerPointer->next == NULL) { 
         // If it allocates the exact size of the heap,
         // it grows the heap to allow for the final free header
         if (headerPointer->size - size > headerSize) {
            doMalloc(headerPointer, size);
         }
         else {
            sbrk(BREAK_INCREMENT);
            headerPointer->size += BREAK_INCREMENT;
            doMalloc(headerPointer, size);
         }
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

void free(void *ptr) {
   header *headerPointer = head;
   header *headerBefore;

   while (((uint8_t *) &headerPointer->allocatedBlock)
    + headerPointer->size < (uint8_t * ) ptr) {
      headerBefore = headerPointer;
      headerPointer = headerPointer->next;
   }

   if (headerBefore == NULL && headerPointer->next == NULL) {
      headerPointer->freeFlag = TRUE;
   }
   else if (headerBefore == NULL && headerPointer->next != NULL) {
      if (headerPointer->next->freeFlag == FALSE) {
         headerPointer->freeFlag = TRUE;
      }
      else if (headerPointer->next->freeFlag == TRUE) {
         headerPointer->next = headerPointer->next->next;
         headerPointer->size += headerSize + headerPointer->next->size;
         headerPointer->freeFlag = TRUE;
      }
      else {
         assert(0);
      }
   }
   else if (headerBefore != NULL && headerPointer->next == NULL) {
      if (headerBefore->freeFlag == FALSE) {
         headerPointer->freeFlag = TRUE;
      }
      else if (headerBefore->freeFlag == TRUE) {
         headerBefore->next = headerPointer->next;
         headerBefore->size += headerSize + headerPointer->next->size;
      }
      else {
         assert(0);
      }
   }
   else if (headerBefore != NULL && headerPointer->next != NULL) {
      if (headerBefore->freeFlag == TRUE
       && headerPointer->next->freeFlag == TRUE) {
         headerBefore->next = headerPointer->next->next;
         headerBefore->size += headerSize + headerPointer->size +
            headerSize + headerPointer->next->size;
      }
      else if (headerBefore->freeFlag == FALSE
       && headerPointer->next->freeFlag == TRUE) {
         headerPointer->next = headerPointer->next->next;
         headerPointer->size += headerSize + headerPointer->next->size;
         headerPointer->freeFlag = TRUE;
      }
      else if (headerBefore->freeFlag == TRUE
       && headerPointer->next->freeFlag == FALSE) {
         headerBefore->next = headerPointer->next;
         headerBefore->size += headerSize + headerPointer->size;
      }
      else if (headerBefore->freeFlag == FALSE
       && headerPointer->next->freeFlag == FALSE) {
         headerPointer->freeFlag = TRUE;
      }
      else {
         assert(0);
      }
   }
   else {
      assert(0);
   }
}

void *realloc(void *ptr, size_t size) {
   return 0;
}


