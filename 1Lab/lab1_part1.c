#include "globals.h"

void printCoordinates(uint8_t row, uint8_t col) {
   set_cursor(1, 1);
   print_string("(");
   print_int(row);
   print_string(", ");
   print_int(col);
   print_string(")");
}

void printName(uint8_t row, uint8_t col) {
   set_cursor(row, col);
   print_string("Luke Baumann");
   set_cursor(row, col);
}

void main(void) {
   uint8_t row = 2;
   uint8_t col = 1;
   uint8_t color = BLACK;
   uint8_t inputByte = 0;
   char *myName = "Luke Baumann";
   uint8_t nameLength = 12;

   serial_init();

   printCoordinates(row, col);
   printName(row, col);

   while (1) {
      
      //while (!byte_available());
      inputByte = read_byte();

      if (inputByte != 255) {
         if (inputByte == 's') {
            if (row < MAX_ROW) {
               row++;
            }
         }
         else if (inputByte == 'w') {
            if (row > 2) {
               row--;
            }
         }
         else if (inputByte == 'd') {
            if (col < MAX_COL - nameLength + 1) {
               col++;
            }
         }
         else if (inputByte == 'a') {
            if (col > 1) {
               col--;
            }
         }
         else if (inputByte == 'c') {
            color = color == WHITE ? BLACK : color + 1;
         }

         clear_screen();
         set_color(color);
         printCoordinates(row, col);
         printName(row, col);
      }
   }
}



