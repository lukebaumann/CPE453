#include <avr/io.h>

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

void print_string(char* s) {
   while (*s) {
      write_byte(*s++);
   }
}

void print_int(uint16_t i) {
   char integerString[30];

   itoa(integerString, i, 10);
   

   print_string(integerString);
}

void print_int32(uint32_t i) {
   char integerString[30];

   itoa(integerString, i, 10);

   print_string(integerString);
}

void print_hex(uint16_t i) {
   char integerString[30];

   itoa(integerString, i, 16);

   print_string(integerString);
}

void print_hex32(uint32_t i) {
   char integerString[30];

   itoa(integerString, i, 16);

   print_string(integerString);
}

void set_cursor(uint8_t row, uint8_t col) {
   char cursorCommand[30];

   cursorCommand[0] = '';
//   cursorCommand[1] = row;
   cursorCommand[2] = ';';
//   cursorCommand[3] = col;
   cursorCommand[4] = 'f';
   cursorCommand[5] = '\0';

   print_string(cursorCommand);
}

void set_color(uint8_t color) {
   char cursorCommand[30];
   if (color >= BLACK && color <= WHITE) {
      cursorCommand[0] = '';
//      cursorCommand[1] = color;
      cursorCommand[2] = 'm';
      cursorCommand[3] = '\0';

      print_string(cursorCommand);
   }
}

void clear_screen(void) {
   char cursorCommand[30];

   cursorCommand[0] = '';
   cursorCommand[1] = '2';
   cursorCommand[2] = 'J';
   cursorCommand[3] = '\0';

   print_string(cursorCommand);
}
