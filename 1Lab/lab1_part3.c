#include "globals.h"

void main(void) {
   uint8_t row = 1;
   uint8_t col = 1;
   uint8_t color = BLACK;
   uint8_t inputByte = 0;
   uint8_t delayTime = 100;

   STATE state = CONTINUE;

   serial_init();

   
   while (1) {
      switch (state) {
      case CONTINUE:
         inputByte = read_byte();

         if (inputByte != 255 && delayTime <= 65530) {
            if (inputByte == 'a') {
               delayTime += 5;
            }
            else if (inputByte == 'z' && delayTime >= 5) {
               delayTime -= 5;
            }
            else if (inputByte == 'q') {
               state = FINISH;
               break;
            }
         }

         led_on();

         _delay_ms(100);

         led_off();


         state = CONTINUE;

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

void led_on() {
   DDRB |= 0x10;
   PORTB |= 0x10;
}

void led_off() {
   DDRB |= 0x10;
   PORTB |= 0xEF;
}


