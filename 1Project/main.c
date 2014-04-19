#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void printPointer(int num, void *ptr);
void printNumber(uint32_t num);
uint32_t min(uint32_t n1, uint32_t n2);
int main(void) {
   uint32_t i = 0;
   uint32_t j = 0;
   uint32_t temp1 = 0;
   uint32_t temp2 = 0;
   void *ptr;
   uint8_t *arraym[100000];
   uint8_t *arrayr[100000];
   uint8_t test[100000];

   srand(5);

   for (i = 0; i < 100000; i++) {
      test[i] = i;
   }

   for (i = 0; i < 100000; i++) {
      temp1 = rand() % 100000;
      arraym[i] = malloc(temp1);

      printNumber(i);
      arraym[i][0] = 4;
      arraym[i][temp1 / 2] = 6;
      arraym[i][temp1 - 1] = 8;
      printNumber(i);
   }


   for (i = 0; i < 100000; i++) {
      printNumber(i);
      free(arraym[i]);
      printNumber(i);
   }

   return 0;
}

uint32_t min(uint32_t n1, uint32_t n2) {
   return n1 < n2 ? n1 : n2;
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
