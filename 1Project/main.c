#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
   uint32_t i = 0;
   char buffer[1000];
   void *ptr;
   void *array[1000];

   for (i = 0; i < 1000; i++) {
      sprintf(buffer, "i: %d\n", i);
      puts(buffer);
      array[i] = malloc(100);
      sprintf(buffer, "array[%d] (malloc): %p\n", i, array[i]);
      puts(buffer);
      array[i] = realloc(array[i], i);
      sprintf(buffer, "array[%d] (realloc): %p\n", i, array[i]);
      puts(buffer);
   }

   for (i = 0; i < 1000; i++) {
      free(array[i]);
   }

   return 0;
}
