#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void printPointer(int num, void *ptr);
void printNumber(uint32_t num);
int main(void) {
   uint32_t i = 0;
   uint32_t temp = 0;
   void *ptr;
   uint8_t *arraym[10000];
   uint8_t *arrayr[10000];
   uint8_t test[8193];

   srand(3);

   for (i = 0; i < 10000; i = i + 2) {
      temp = rand() % 10000;
      printNumber(temp);
      arraym[i] = malloc(temp);
      printPointer(0, arraym[i]);

      temp = rand() % 10000;
      printNumber(temp);
      arraym[i + 1] = malloc(temp);
      printPointer(1, arraym[i + 1]);
   
      temp = rand() % 10000;
      printNumber(temp);
      arrayr[i] = realloc(arraym[i], temp);
      printPointer(2, arrayr[i]);
   }


   return 0;
}

void printPointer(int num, void *ptr) {
   char buffer[1000];
   sprintf(buffer, "%d: %p\n", num, ptr);
   puts(buffer);
}

void printNumber(uint32_t num) {
   char buffer[1000];
   sprintf(buffer, "%u\n", num);
   puts(buffer);
}
