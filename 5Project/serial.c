#include <avr/io.h>
#include "globals.h"

/*
 * Initialize the serial port.
 */
void serial_init(void) {
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
uint8_t byte_available(void) {
   return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}

/*
 * Unbuffered read
 * Return 255 if no character is available otherwise return available character.
 */
uint8_t read_byte(void) {
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

/*
 * Prints a string.
 *
 * s string to print
 */
void print_string(char* s) {
   while (*s)
      write_byte(*s++);
}

/*
 * Print an 8-bit or 16-bit unsigned int
 *
 * i integer to print
 */
void print_int(uint16_t i) {
   uint16_t c = 10000;

   while (c && i / c == 0) {
      i %= c;
      c /= 10;
   }

   if (!c)
      write_byte('0');

   while (c) {
      write_byte( i / c + ASCII_0);
      i %= c;
      c /= 10;
   }
}

/*
 * Print a 32-bit unsigned int
 *
 * i integer to print
 */
void print_int32(uint32_t i) {
      uint32_t c = 1000000000;

   while (c && i / c == 0) {
      i %= c;
      c /= 10;
   }

   if (!c)
      write_byte('0');

   while (c) {
      write_byte( i / c + ASCII_0);
      i %= c;
      c /= 10;
   }
}

/*
 * Print an 8-bit or 16-bit unsigned int in hex
 *
 * i integer to print
 */
void print_hex(uint16_t i) {
   uint8_t nibble = 0;
   int8_t c = 12;

   while (!(i >> c & 0xF) && c >= 0)
      c -= 4;

   for (; c >= 0; c -= 4) {
      nibble = i >> c & 0xF;

      if (nibble <= 9)  //is nibble 0-9?
         write_byte(nibble + ASCII_0);
      else              //nibble is A-F
         write_byte(nibble + ASCII_7);
   }
}

/*
 * Print a 32-bit unsigned int in hex
 *
 * i integer to print
 */
void print_hex32(uint32_t i) {
   uint8_t nibble = 0;
   int8_t c = 28;

   while (!(i >> c & 0xF) && c >= 0)
      c -= 4;

   for (; c >= 0; c -= 4) {
      nibble = i >> c & 0xF;

      if (nibble <= 9)  //is nibble 0-9?
         write_byte(nibble + ASCII_0);
      else              //nibble is A-F
         write_byte(nibble + ASCII_7);
   }
}

/*
 * Set the console cursor position 
 *
 * row The cursor row
 * col The cursor column
 */
void set_cursor(uint8_t row, uint8_t col) {
   print_string("\033[");
   print_int(row);
   write_byte(';');
   print_int(col);
   write_byte('H');
}

/*
 * Sets the console color
 *
 * color A VT100 color value
 */
void set_color(uint8_t color) {
   if (color < BLACK || color > WHITE)
      return;

   print_string("\033[");
   print_int(color);
   write_byte('m');
}

/*
 * Clears the screen
 */
void clear_screen(void) {
   print_string("\033[2J");
   print_string("\033[H");
}