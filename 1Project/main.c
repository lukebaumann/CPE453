// According to Kyle Corio, there is no allocated space in the heap until one of the *alloc calls is made
void *programBreak = 0;
void *nextAddressToAllocated = 0;
uint8_t heapSize = 0;

typedef struct {
   uint8_t freeFlag,
   uint32_t size,
} header;

void *calloc(size_t nmemb, size_t size) {
   return 0;
}

void *malloc(size_t size) {
   if (nextAddressToAllocated + sizeof(header) + size > programBreak) {
      sblk(...);
   }

   
   

   return 0;
}

void free(void ptr) {

}

void *realloc(void *ptr, size_t size) {
   return 0;
}


