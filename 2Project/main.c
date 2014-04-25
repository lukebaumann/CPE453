#include "globals.h"
#include "os.h"

extern uint32_t isrCounter;

void main(void) {
   serial_init();
   os_init();

   create_thread((uint16_t) printSystemInfo, 0, 50);
   create_thread((uint16_t) blink, 0, 50);
   os_start();

   while(1) {}
}

void counting(void) {
   while(1) {
      print_int(isrCounter);
      print_string("\r");
      _delay_ms(100);
      clear_screen();
   }
}

void blink(void) {
   //print_string("\n\rI'm here!\n\r");
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
// void led_on() {
//    asm volatile("ldi r31, 0x00");
//    asm volatile("ldi r30, 0x25");
//    asm volatile("ldi r18, 0x20");
//    asm volatile("st z, r18");

//    asm volatile("ldi r31, 0x00");
//    asm volatile("ldi r30, 0x24");
//    asm volatile("ldi r18, 0x20");
//    asm volatile("st z, r18");
// }

// Load the address of DDRB and PORTB into the Z register, then clear the
// fifth bit of each to turn on the LED
// void led_off() {
//    asm volatile("ldi r31, 0x00");
//    asm volatile("ldi r30, 0x25");
//    asm volatile ("ldi r18, 0x00");
//    asm volatile ("st z, r18");

//    asm volatile("ldi r31, 0x00");
//    asm volatile("ldi r30, 0x24");
//    asm volatile("ldi r18, 0x00");
//    asm volatile ("st z, r18");
// }

void led_on() {
   //Set data direction to OUTPUT
   //Clear Z high byte
   __asm__ volatile ("clr r31");
   //Set Z low byte to DDRB
   __asm__ volatile ("ldi r30,0x24");
   //Set 0x20 (DDRB bit 5 to 1)
   __asm__ volatile ("ldi r18,0x20");
   //Write 0x20 to location 0x24 (set LED pin as output)
   __asm__ volatile ("st Z, r18");

   //Set LED to ON
   //Set Z low byte to LED register
   __asm__ volatile ("ldi r30,0x25");
   
   __asm__ volatile ("ldi r18,0x20");
   __asm__ volatile ("st Z, r18");
}

void led_off() {
   //Set data direction to OUTPUT
   //Clear Z high byte
   __asm__ volatile ("clr r31");
   //Set Z low byte to DDRB
   __asm__ volatile ("ldi r30,0x24");
   //Set 0x20 (DDRB bit 5 to 1)
   __asm__ volatile ("ldi r18,0x20");
   //Write 0x20 to location 0x24 (set LED pin as output)
   __asm__ volatile ("st Z, r18");

   //Set LED to OFF
   __asm__ volatile ("ldi r30,0x25");
   
   __asm__ volatile ("ldi r18,0x00");
   __asm__ volatile ("st Z, r18");
}