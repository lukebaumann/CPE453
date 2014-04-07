#include "globals.h"

void main(void) {
   uint8_t row = 0;
   uint8_t col = 0;
   uint8_t color = BLACK;
   uint8_t inputByte = 0;
   char *myName = "Luke Baumann";
   uint8_t nameLength = 12;

   serial_init();

   print_string("Luke Baumann");
   while (1) {
      
      while (!byte_available());
      inputByte = read_byte();

      if (inputByte == 'a') {
         if (row < MAX_ROW - nameLength) {
            row++;
         }
      }
      else if (inputByte == 'd') {
         if (row > 0) {
            row--;
         }
      }
      else if (inputByte == 'd') {
         if (col < MAX_COL) {
            col++;
         }
      }
      else if (inputByte == 'd') {
         if (col > 0) {
            col--;
         }
      }
      else if (inputByte = 'c') {
         color = color == WHITE ? BLACK : color++;
      }
      else {
         assert(0);
      }

      clear_screen();
      set_cursor(row, col);
      set_color(color);

      print_string("Luke Baumann");
   }
}



