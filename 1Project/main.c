#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
   uint32_t i = 0;
   char buffer[1000];

   sprintf(buffer, "1\n"); 
   puts(buffer);
   uint8_t *array1 = malloc(1000 * sizeof(uint8_t));
   sprintf(buffer, "2\n"); 
   puts(buffer);
   uint8_t *array2 = malloc(1000 * sizeof(uint8_t));
   sprintf(buffer, "3\n"); 
   puts(buffer);
   uint8_t *array3 = malloc(1000 * sizeof(uint8_t));
   sprintf(buffer, "4\n"); 
   puts(buffer);
   uint8_t *array4 = malloc(1000 * sizeof(uint8_t));
   sprintf(buffer, "5\n"); 
   puts(buffer);

   for (i = 0; i < 1000; i++) {
      array1[i] = i;
   }
   for (i = 0; i < 1000; i++) {
      if (!(i % 50)) {
         printf("%d\n", array1[i]); 
      }
   }

   free(array1);
   free(array2);
   free(array3);
   free(array4);

   return 0;
}
