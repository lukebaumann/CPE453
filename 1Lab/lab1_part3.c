#include "globals.h"

void main(void) {
   uint8_t row = 1;
   uint8_t col = 1;
   uint8_t color = BLACK;
   uint8_t inputByte = 0;
   uint8_t delayTime = 100;

   STATE state = CONTINUE;

   serial_init();

   setXRegister(PORTB);
   setYRegister(DDRB);
   
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

void setXRegister(uint16_t value) {
   char buffer[100];

   snprintf(buffer, "ldi r27 0x%x", value >> 8, 100);
   asm volatile (buffer);
   snprintf(buffer, "ldi r26 0x%x", value && 0xFF, 100);
   asm volatile (buffer);
}

void setYRegister(uint16_t value) {
   char buffer[100];

   snprintf(buffer, "ldi r29 0x%x", value >> 8, 100);
   asm volatile (buffer);
   snprintf(buffer, "ldi r28 0x%x", value && 0xFF, 100);
   asm volatile (buffer);
}

void led_on() {
   asm volatile ("ld r1 (x)");
   asm volatile ("andi r1 0x10");
   asm volatile ("st (x) r1");

   asm volatile ("ld r1 (y)");
   asm volatile ("andi r1 0x10");
   asm volatile ("st (y) r1");
}

void led_off() {
   asm volatile ("ld r1 (x)");
   asm volatile ("ori r1 0xef");
   asm volatile ("st (x) r1");

   asm volatile ("ld r1 (y)");
   asm volatile ("ori r1 0xef");
   asm volatile ("st (y) r1");
}


