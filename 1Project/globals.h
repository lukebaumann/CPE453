#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#define BREAK_INCREMENT 65536
#define PADDING 16
#define TRUE 1
#define FALSE 0

typedef struct header {
   uint8_t freeFlag;
   uint32_t size;
   struct header *next;
} header;

void *myMalloc(size_t size);
void *myCalloc(size_t nmemb, size_t size);
void myFree(void *ptr);
void *myRealloc(void *ptr, size_t size);
void putChar(char c); 
uint32_t ceil16(uint32_t i);
void makeHeader(header *headerPointer);
void doMalloc(header *headerPointer, size_t size);
void mallocForTailHeader(header *headerPointer, size_t size);
header *firstMalloc(size_t size);
header *getBeforePointerFromPointer(void *ptr);
header *getHeaderPointerFromBefore(header *headerBefore);
void reallocFromTooBig(header *headerPointer, uint32_t size);
void reallocIntoNextHeader(header *headerPointer, uint32_t size);
void reallocIntoPreviousAndNextHeader(header *headerBefore, uint32_t size);
void reallocIntoPreviousHeader(header *headerBefore, uint32_t size);
void *reallocIntoCompletelyNewBlock(header *headerPointer, uint32_t size);
void *reallocFirstBlock(header *headerPointer, size_t size);
void *reallocMiddleBlock(header *headerPointer,
 header *headerBefore, size_t size);
void *reallocateSecondToLastBlock(header *headerPointer,
 header *headerBefore, size_t size);
void freeHead(header *headerPointer);

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
