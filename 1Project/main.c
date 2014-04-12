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

   for (i = 0; i < 100000; i = i + 2) {
      temp1 = rand() % 100000;
     // printNumber(temp1);
      arraym[i] = malloc(temp1);
     // printPointer(0, arraym[i]);
      memcpy(arraym[i], test, temp1);

      temp2 = rand() % 100000;
     // printNumber(temp2);
      arrayr[i] = realloc(arraym[i], temp2);
     // printPointer(2, arrayr[i]);

      
      if (memcmp(arrayr[i], test, min(temp1, temp2))) {
         for (j = 0; j < min(temp1, temp2); j++) {
            if (arrayr[i][j] != test[j]) {
               printNumber(i);
               putchar(arrayr[i][j]);
               putchar(test[j]);
               putchar('\n');
            }
         }
         break;
      }


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
