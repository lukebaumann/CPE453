#include "globals.h"

void main(void) {
   volatile uint8_t inputByte = 0;
   volatile uint8_t numDelays = 1;
   volatile uint8_t i = 0;

   STATE state = CONTINUE;

   serial_init();

   
   while (1) {
      switch (state) {
      case CONTINUE:
         inputByte = read_byte();

         if (inputByte != 255) {
            if (inputByte == 'a' && numDelays < 100) {
               numDelays++;
            }
            else if (inputByte == 'z' && numDelays > 0) {
               numDelays--;
            }
            else if (inputByte == 'q') {
               state = FINISH;
               break;
            }
         }

         led_off();
         for (i = 0; i < numDelays; i++) {
            _delay_ms(50);
         }

         led_on();
         for (i = 0; i < numDelays; i++) {
            _delay_ms(50);
         }

         state = CONTINUE;
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
   asm volatile("ldi r31, 0x0");
   asm volatile("ldi r30, 0x25");
   asm volatile ("ld r17, z");
   asm volatile ("andi r17, 0x10");
   asm volatile ("st z, r17");

   asm volatile("ldi r31, 0x0");
   asm volatile("ldi r30, 0x24");
   asm volatile ("ld r17, z");
   asm volatile ("andi r17, 0x10");
   asm volatile ("st z, r17");
}

void led_off() {
   asm volatile("ldi r31, 0x0");
   asm volatile("ldi r30, 0x25");
   asm volatile ("ld r17, z");
   asm volatile ("ori r17, 0xef");
   asm volatile ("st z, r17");

   asm volatile("ldi r31, 0x0");
   asm volatile("ldi r30, 0x24");
   asm volatile ("ld r17, z");
   asm volatile ("ori r17, 0xef");
   asm volatile ("st z, r17");
}


