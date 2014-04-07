#include "globals.h"

void main(void) {
   uint8_t row = 1;
   uint8_t col = 1;
   uint8_t color = BLACK;
   uint8_t inputByte = 0;
   uint8_t num1 = 0;
   uint32_t num2 = 2154;
   uint8_t num3 = 0x4F;
   uint32_t num4 = 0xFFAA0000;

   STATE state = INCREMENT;

   serial_init();

   
   while (1) {
      switch (state) {
      case INCREMENT:
         inputByte = read_byte();

         if (inputByte != 255) {
            state = FINISH;
            break;
         }

         clear_screen();
         set_cursor(1, 1);
         print_int(++num1);
         set_cursor(2, 1);
         print_int32(++num2);
         set_cursor(3, 1);
         print_hex(++num3);
         set_cursor(4, 1);
         print_hex32(++num4);

         state = INCREMENT;
         _delay_ms(100);
         break;
         
      case FINISH:
         state = FINISH;
         _delay_ms(1000);
         break;
      
      default:
         state = FINISH;
         break;
      }
   }
}



