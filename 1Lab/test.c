#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "globals.h"
#include <stdint.h>

/*
 * Unbuffered write
 *
 * b byte to write.
 */
/*uint8_t write_byte(uint8_t b) {
   //loop until the send buffer is empty
   while (((1 << UDRIE0) & UCSR0B) || !(UCSR0A & (1 << UDRE0))) {}

   //write out the byte
   UDR0 = b;
   return 1;
}*/

// For testing
uint8_t write_byte(uint8_t b) {
   putchar(b);
   return 1;
}

void print_string(char* s) {
   while (*s) {
      write_byte(*s++);
   }
}

uint8_t myITOA10(char *string, uint32_t i) {
   uint8_t stringIndex = 0;
   uint8_t remainder = 0;

   do {
      if (stringIndex > MAX_STRING_LENGTH) {
         assert(0);
      }

      remainder = i % 10;
      string[stringIndex++] = '0' + remainder;

      i /= 10;
   } while (i > 0);

   string[stringIndex] = '\0';

   reverseString(string, stringIndex);
   
   return stringIndex;
}

uint8_t myITOA16(char *string, uint32_t i) {
   uint8_t stringIndex = 0;
   uint8_t remainder = 0;

   do {
      if (stringIndex > MAX_STRING_LENGTH) {
         assert(0);
      }

      remainder = i & 0xF;
      string[stringIndex++] = remainder < 10 ? '0' + remainder : 'A' + remainder - 10;

      i >>= 4;
   } while (i > 0);

   string[stringIndex] = '\0';

   reverseString(string, stringIndex);

   return stringIndex;
}

void reverseString(char *string, uint8_t stringSize) {
   uint8_t stringIndex = 0;
   char temp = 0;

   while (stringIndex * 2 < stringSize - 1) {
      temp = string[stringIndex];
      string[stringIndex] = string[stringSize - stringIndex - 1];
      string[stringSize - stringIndex - 1] = temp;

      stringIndex++;
   }
}

// How is this different than print_int32(uint32_t i)?
void print_int(uint16_t i) {
   print_int32(i);
}

void print_int32(uint32_t i) {
   char integerString[MAX_STRING_LENGTH];

   myITOA10(integerString, i);

   print_string(integerString);
}

void print_hex(uint16_t i) {
   char integerString[MAX_STRING_LENGTH];

   myITOA16(integerString, i);

   print_string(integerString);
}

void print_hex32(uint32_t i) {
   char integerString[MAX_STRING_LENGTH];

   myITOA16(integerString, i);

   print_string(integerString);
}

void set_cursor(uint8_t row, uint8_t col) {
   char string[MAX_STRING_LENGTH];
   uint8_t stringIndex = 0;

   string[stringIndex++] = '';
   stringIndex += myITOA10(&string[stringIndex], row);
   string[stringIndex++] = ';';
   stringIndex += myITOA10(&string[stringIndex], col);
   string[stringIndex++] = 'f';
   string[stringIndex++] = '\0';

   if (stringIndex > MAX_STRING_LENGTH) {
      assert(0);
   }

   print_string(string);
}

void set_color(uint8_t color) {
   char string[MAX_STRING_LENGTH];
   uint8_t stringIndex = 0;

   if (color >= BLACK && color <= WHITE) {
      string[stringIndex++] = '[';
      stringIndex += myITOA10(&string[stringIndex], color);
      string[stringIndex++] = 'm';
      string[stringIndex++] = '\0';

      print_string(string);
   }
   else {
      assert(0);
   }
}

void clear_screen(void) {
   char string[MAX_STRING_LENGTH];
   uint8_t stringIndex = 0;

   string[stringIndex++] = '';
   string[stringIndex++] = '2';
   string[stringIndex++] = 'J';
   string[stringIndex++] = '\0';

   print_string(string);
}

void testDec8(uint8_t tested) {
   printf("Expecting: %u\tActual: ", tested);
   print_int(tested);
   printf("\n");
}
void testDec16(uint16_t tested) {
   printf("Expecting: %u\tActual: ", tested);
   print_int(tested);
   printf("\n");
}
void testDec32(uint32_t tested) {
   printf("Expecting: %u\tActual: ", tested);
   print_int32(tested);
   printf("\n");
}

void testHex8(int8_t tested) {
   printf("Expecting: %u\t%X\tActual: ", tested, tested);
   print_hex(tested);
   printf("\n");
}
void testHex16(uint16_t tested) {
   printf("Expecting: %u\t%X\tActual: ", tested, tested);
   print_hex(tested);
   printf("\n");
}
void testHex32(uint32_t tested) {
   printf("Expecting: %u\t%X\tActual: ", tested, tested);
   print_hex32(tested);
   printf("\n");
}

int main(void) {
   uint8_t u8 = 0;
   uint16_t u16 = 0;
   uint32_t u32 = 0;

   set_color(CYAN);

   printf("Testing print_int u8: \n");
   u8 = 14;
   testDec8(u8);
   u8 = 0;
   testDec8(u8);
   u8 = 1;
   testDec8(u8);
   u8 = 14124;
   testDec8(u8);
   u8 = 255;
   testDec8(u8);
   u8 = 256;
   testDec8(u8);
   u8 = 12;
   testDec8(u8);
   u8 = 124;
   testDec8(u8);
   u8 = 64;
   testDec8(u8);
   u8 = 34;
   testDec8(u8);
   u8 = -3;
   testDec8(u8);
   u8 = 123;
   testDec8(u8);
   u8 = 147;
   testDec8(u8);

   printf("Testing print_int u16: \n");
   u16 = 13435;
   testDec16(u16);
   u16 = 1345;
   testDec16(u16);
   u16 = 0;
   testDec16(u16);
   u16 = 23;
   testDec16(u16);
   u16 = -123;
   testDec16(u16);
   u16 = 65535;
   testDec16(u16);
   u16 = 65536;
   testDec16(u16);
   u16 = -1;
   testDec16(u16);
   u16 = 234;
   testDec16(u16);
   u16 = 23462;
   testDec16(u16);
   u16 = 32;
   testDec16(u16);
   u16 = 64;
   testDec16(u16);
   u16 = 14;
   testDec16(u16);

   printf("Testing print_int32 u32: \n");
   u32 = 14;
   testDec32(u32);
   u32 = 0;
   testDec32(u32);
   u32 = 54;
   testDec32(u32);
   u32 = 23;
   testDec32(u32);
   u32 = 65535;
   testDec32(u32);
   u32 = 65536;
   testDec32(u32);
   u32 = 4294967295;
   testDec32(u32);
   u32 = 4294967296;
   testDec32(u32);
   u32 = -1;
   testDec32(u32);
   u32 = 123;
   testDec32(u32);
   u32 = 125126;
   testDec32(u32);
   u32 = 345736547;
   testDec32(u32);

   printf("Testing print_hex u8: \n");
   u8 = 14;
   testHex8(u8);
   u8 = 0;
   testHex8(u8);
   u8 = 1;
   testHex8(u8);
   u8 = 14124;
   testHex8(u8);
   u8 = 255;
   testHex8(u8);
   u8 = 256;
   testHex8(u8);
   u8 = 12;
   testHex8(u8);
   u8 = 124;
   testHex8(u8);
   u8 = 64;
   testHex8(u8);
   u8 = 34;
   testHex8(u8);
   u8 = -3;
   testHex8(u8);
   u8 = 123;
   testHex8(u8);
   u8 = 147;
   testHex8(u8);

   printf("Testing print_hex u16: \n");
   u16 = 13435;
   testHex16(u16);
   u16 = 1345;
   testHex16(u16);
   u16 = 0;
   testHex16(u16);
   u16 = 23;
   testHex16(u16);
   u16 = -123;
   testHex16(u16);
   u16 = 65535;
   testHex16(u16);
   u16 = 65536;
   testHex16(u16);
   u16 = -1;
   testHex16(u16);
   u16 = 234;
   testHex16(u16);
   u16 = 23462;
   testHex16(u16);
   u16 = 32;
   testHex16(u16);
   u16 = 64;
   testHex16(u16);
   u16 = 14;
   testHex16(u16);

   printf("Testing print_hex32 u32: \n");
   u32 = 14;
   testHex32(u32);
   u32 = 0;
   testHex32(u32);
   u32 = 54;
   testHex32(u32);
   u32 = 23;
   testHex32(u32);
   u32 = 65535;
   testHex32(u32);
   u32 = 65536;
   testHex32(u32);
   u32 = 4294967295;
   testHex32(u32);
   u32 = 4294967296;
   testHex32(u32);
   u32 = -1;
   testHex32(u32);
   u32 = 123;
   testHex32(u32);
   u32 = 125126;
   testHex32(u32);
   u32 = 345736547;
   testHex32(u32);
   return 0;
}
