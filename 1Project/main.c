#include <stdlib.h>

extern void *myMalloc(size_t size);
extern void myFree(void *ptr);

int main(void) {
   uint32_t i = 0;

   uint8_t *array = myMalloc(100 * sizeof(uint8_t));
   for (i = 0; i < 100; i++) {
      array[i] = i;
   }

   myFree(array);

   return 0;
}
