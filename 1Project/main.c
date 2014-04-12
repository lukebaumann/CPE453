#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void printPointer(int num, void *ptr);
int main(void) {
   uint32_t i = 0;
   void *ptr;
   uint8_t *arraym[100];
   uint8_t *arrayr[100];
   uint8_t test[8193];

      arraym[0] = malloc(1000);
      printPointer(0, arraym[0]);
      arraym[1] = malloc(1000);
      printPointer(1, arraym[1]);
      arraym[2] = malloc(1000);
      printPointer(2, arraym[2]);
      arraym[3] = malloc(1000);
      printPointer(3, arraym[3]);
      arraym[4] = malloc(1000);
      printPointer(4, arraym[4]);

      arrayr[0] = realloc(arraym[3], 1000);
      printPointer(0, arrayr[0]);

      assert(arrayr[0] == arraym[3]);

      arraym[6] = malloc(1000);
      printPointer(6, arraym[6]);
      assert(arraym[6] > arraym[4]);

      arrayr[1] = realloc(arrayr[0], 1001);
      printPointer(1, arrayr[1]);
      assert(arrayr[1] > arraym[6]);

      arraym[7] = malloc(1000);
      printPointer(7, arraym[7]);
      assert(arraym[7] == arraym[3]);


   return 0;
}

void printPointer(int num, void *ptr) {
   char buffer[1000];
   sprintf(buffer, "%d: %p\n", num, ptr);
   puts(buffer);
}
