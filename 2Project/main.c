#include "globals.h"
#include "os.h"

extern uint32_t isrCounter;

void main(void) {
   serial_init();
   //blink();
   os_init();

   create_thread((uint16_t) counting, 0, 50);
   os_start();

   while(1);
}

void counting(void) {
   while(1) {
      print_int(isrCounter);
      clear_screen();
   }
}

void blink(void) {
   volatile uint8_t numDelays = 20;
   volatile uint8_t i = 0;

   while (1) {
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
   }
}

// Load the address of DDRB and PORTB into the Z register, then set the
// fifth bit of each to turn on the LED
void led_on() {
   asm volatile("ldi r31, 0x00");
   asm volatile("ldi r30, 0x25");
   asm volatile("ldi r18, 0x20");
   asm volatile("st z, r18");

   asm volatile("ldi r31, 0x00");
   asm volatile("ldi r30, 0x24");
   asm volatile("ldi r18, 0x20");
   asm volatile("st z, r18");
}

// Load the address of DDRB and PORTB into the Z register, then clear the
// fifth bit of each to turn on the LED
void led_off() {
   asm volatile("ldi r31, 0x00");
   asm volatile("ldi r30, 0x25");
   asm volatile ("ldi r18, 0x00");
   asm volatile ("st z, r18");

   asm volatile("ldi r31, 0x00");
   asm volatile("ldi r30, 0x24");
   asm volatile("ldi r18, 0x00");
   asm volatile ("st z, r18");
}
