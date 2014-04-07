#include "globals.h"

void main(void) {
   uint8_t row = 1;
   uint8_t col = 1;
   uint8_t color = BLACK;
   uint8_t inputByte = 0;
   char *myName = "Luke Baumann";
   uint8_t nameLength = 12;

   serial_init();

   print_string("Luke Baumann");
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
            if (row > 1) {
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
         set_cursor(row, col);
         set_color(color);

         print_string("Luke Baumann");
      }
   }
}



