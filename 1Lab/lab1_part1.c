#include "globals.h"

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
      // Read input. If there is input, check the value. If s, move the row up.
      // If w, move the row to the down. If d, move the col to the
      // right. If a, move the col to the left. If c, cycle to the next color.
      // It checks to make sure the text will not fall off the edge of the screen.
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

         // Do the actual printing:
         // First clear the screen
         // Second set the color
         // Third print the coordinates in the upper left corner 
         // Fourth print my name
         clear_screen();
         set_color(color);
         printCoordinates(row, col);
         printName(row, col);
      }
   }
}

// Prints the coordinates in the upper left corner 
void printCoordinates(uint8_t row, uint8_t col) {
   set_cursor(1, 1);
   print_string("(");
   print_int(row);
   print_string(", ");
   print_int(col);
   print_string(")");
}

// Sets the cursor to the correct position and then prints my name
void printName(uint8_t row, uint8_t col) {
   set_cursor(row, col);
   print_string("Luke Baumann");
   set_cursor(row, col);
}
