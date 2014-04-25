#include <avr/io.h>
#include "globals.h"

/*
 * Initialize the serial port.
 */
void serial_init() {
   uint16_t baud_setting;

   UCSR0A = _BV(U2X0);
   baud_setting = 16; //115200 baud

   // assign the baud_setting
   UBRR0H = baud_setting >> 8;
   UBRR0L = baud_setting;

   // enable transmit and receive
   UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
}

/*
 * Return 1 if a character is available else return 0.
 */
uint8_t byte_available() {
   return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

/*
 * Unbuffered read
 * Return 255 if no character is available otherwise return available character.
 */
uint8_t read_byte() {
   if (UCSR0A & (1 << RXC0)) return UDR0;
   return 255;
}

/*
 * Unbuffered write
 *
 * b byte to write.
 */
uint8_t write_byte(uint8_t b) {
   //loop until the send buffer is empty
   while (((1 << UDRIE0) & UCSR0B) || !(UCSR0A & (1 << UDRE0))) {}

   //write out the byte
   UDR0 = b;
   return 1;
}

// Send the string, character by character to write_byte
void print_string(char* s) {
   while (*s) {
      write_byte(*s++);
   }
}

// Find each decimal place value and store it in a string,
// then reverse the string to be in the correct order and
// return the length
uint8_t myITOA10(char *string, uint32_t i) {
   uint8_t stringIndex = 0;
   uint8_t remainder = 0;

   do {
      remainder = i % 10;
      string[stringIndex++] = '0' + remainder;

      i /= 10;
   } while (i > 0);

   string[stringIndex] = '\0';

   reverseString(string, stringIndex);
   
   return stringIndex;
}


// Find each hexidecimal place value and store it in a string,
// then reverse the string to be in the correct order and
// return the length
uint8_t myITOA16(char *string, uint32_t i) {
   uint8_t stringIndex = 0;
   uint8_t remainder = 0;

   do {
      remainder = i & 0xF;
      string[stringIndex++] = remainder < 10 ? '0' + remainder : 'A' + remainder - 10;

      i >>= 4;
   } while (i > 0);

   string[stringIndex] = '\0';

   reverseString(string, stringIndex);

   return stringIndex;
}

// Reverses the string in place
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

// Prints a 16 bit int in dec
void print_int(uint16_t i) {
   print_int32(i);
}

// Prints a 32 bit int in dec
void print_int32(uint32_t i) {
   char integerString[MAX_STRING_LENGTH];

   myITOA10(integerString, i);

   print_string(integerString);
}

// Prints a 16 bit int in hex
void print_hex(uint16_t i) {
   char integerString[MAX_STRING_LENGTH];

   myITOA16(integerString, i);

   print_string(integerString);
}

// Prints a 32 bit int in hex
void print_hex32(uint32_t i) {
   char integerString[MAX_STRING_LENGTH];

   myITOA16(integerString, i);

   print_string(integerString);
}

// Sends the VT100 command to move the cursor to the right place
void set_cursor(uint8_t row, uint8_t col) {
   char string[MAX_STRING_LENGTH];
   uint8_t stringIndex = 0;

   string[stringIndex++] = '';
   string[stringIndex++] = '[';
   stringIndex += myITOA10(&string[stringIndex], row);
   string[stringIndex++] = ';';
   stringIndex += myITOA10(&string[stringIndex], col);
   string[stringIndex++] = 'f';
   string[stringIndex++] = '\0';

   print_string(string);
}

// Sends the VT100 command to change the color
void set_color(uint8_t color) {
   char string[MAX_STRING_LENGTH];
   uint8_t stringIndex = 0;

   if (color >= BLACK && color <= WHITE) {
      string[stringIndex++] = '';
      string[stringIndex++] = '[';
      stringIndex += myITOA10(&string[stringIndex], color);
      string[stringIndex++] = 'm';
      string[stringIndex++] = '\0';

      print_string(string);
   }
}

// Sends the VT100 command to clear the screen
// void clear_screen(void) {
//    char string[MAX_STRING_LENGTH];
//    uint8_t stringIndex = 0;

//    string[stringIndex++] = '';
//    string[stringIndex++] = '[';
//    string[stringIndex++] = '2';
//    string[stringIndex++] = 'J';
//    string[stringIndex++] = '\0';

//    print_string(string);
// }

void clear_screen(void) {
   print_string("\033[2J");
   print_string("\033[H");
}
