#include "globals.h"

void main(void) {
   volatile uint8_t inputByte = 0;
   volatile uint8_t numDelays = 1;
   volatile uint8_t i = 0;

   STATE state = CONTINUE;

   serial_init();
   
   while (1) {
      switch (state) {
      // This is the main state in my program
      case CONTINUE:
         // First it handles any possible input
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

         // Then it turns off the LED and delays
         led_off();
         for (i = 0; i < numDelays; i++) {
            _delay_ms(50);
         }

         // Then it turns on the LED and delays
         led_on();
         for (i = 0; i < numDelays; i++) {
            _delay_ms(50);
         }

         state = CONTINUE;
         break;

      case FINISH:
         // When I finish, just sleep for a second repeatably
         state = FINISH;
         _delay_ms(1000);
         break;
      
      default:
         state = FINISH;
         break;
      }
   }
}

// Load the address of DDRB and PORTB into the Z register, then set the
// fifth bit of each to turn on the LED
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

// Load the address of DDRB and PORTB into the Z register, then clear the
// fifth bit of each to turn on the LED
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


