#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
   uint32_t i = 0;
   char buffer[1000];
   void *ptr;
   uint8_t *array[8192];

   for (i = 0; i < 8192; i++) {
      sprintf(buffer, "i: %d\n", i);
      puts(buffer);
      
      array[i] = malloc(i * sizeof(uint8_t));
      sprintf(buffer, "array[%d] (realloc): %p\n", i, array[i]);
      puts(buffer);

   }

   for (i = 0; i < 8192; i++) {
      free(array[i]);
   }

   return 0;
}
