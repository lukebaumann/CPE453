#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
   uint32_t i = 0;
   char buffer[1000];
   void *ptr;
   uint8_t *array[1000];

   for (i = 1; i < 1000; i++) {
      sprintf(buffer, "i: %d\n", i);
      puts(buffer);
      
      array[i] = malloc(0x1000 * sizeof(uint8_t));
      sprintf(buffer, "array[%d] (malloc): %p\n", i, array[i]);
      puts(buffer);

      ((uint8_t *)array[i])[0] = 0xFF;
      ((uint8_t *)array[i])[i % 100] = 0xAA;
      sprintf(buffer, "array[%d][0] (malloc): 0x%X\n", i, ((uint8_t *) array[i])[0]);
      puts(buffer);
      sprintf(buffer, "array[%d][%d] (malloc): 0x%X\n", i, i, ((uint8_t *) array[i])[i]);
      puts(buffer);

      array[i] = realloc(array[i], (i + 1) * sizeof(uint8_t));
      sprintf(buffer, "array[%d] (realloc): %p\n", i, array[i]);
      puts(buffer);

      sprintf(buffer, "array[%d][0] (realloc): 0x%X\n", i, ((uint8_t *) array[i])[0]);
      puts(buffer);
      sprintf(buffer, "array[%d][%d %% 100] (realloc): 0x%X\n", i, i, ((uint8_t *) array[i])[i % 100]);
      puts(buffer);
   }

   for (i = 0; i < 1000; i++) {
      free(array[i]);
   }

   return 0;
}
