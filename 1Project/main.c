#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(void) {
   uint32_t i = 0;
   char buffer[1000];
   void *ptr;
   uint8_t *array[10000];
   uint8_t test[8193];

   for (i = 0; i < 8193; i++) {
      test[i] = i;
   }

   for (i = 1; i < 8193; i++) {
      
      array[i] = malloc(i * sizeof(uint8_t));
      sprintf(buffer, "array[%d] (malloc): %p\n", i, array[i]);
   //   puts(buffer);

      if (i %2) {
      memcpy(array[i], test, i);

      array[i] = realloc(array[i], 100 * sizeof(uint8_t));
      sprintf(buffer, "array[%d] (realloc): %p\n", i, array[i]);
     // puts(buffer);

      if (memcmp(array[i], test, i > 100 ? 100 : i)) {
         sprintf(buffer, "i: %d\n", i);
         puts(buffer);
         sprintf(buffer, "Different\n");
         puts(buffer);

         int j = 0;
         for (j = 0; j < (i > 100 ? 100 : i); j++) {
            sprintf(buffer, "%d %d", test[j], array[i][j]);
            puts(buffer);
         }
         putchar('\n');
      }
      }
   }

   for (i = 0; i < 1000; i++) {
      free(array[i]);
   }

   return 0;
}

