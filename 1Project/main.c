#include <stdlib.h>
#include <stdint.h>

int main(void) {
   uint32_t i = 0;

   uint8_t *array = malloc(100 * sizeof(uint8_t));
   for (i = 0; i < 100; i++) {
      array[i] = i;
   }

   free(array);

   return 0;
}
