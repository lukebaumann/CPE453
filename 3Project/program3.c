/**
 * File: program2.c
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 02
 * 04/25/2014
 */

#include "globals.h"
#include "os.h"

extern uint32_t oneSecondCounter;

/**
 * Ivokes the operating system.
 */
void main(void) {
   serial_init();
   os_init();

   create_thread((uint16_t) printSystemInfo, 0, 50);
   create_thread((uint16_t) blink, 0, 50);

   os_start();
   sei();
   while(1) {}
}

/**
 * Flashes the LED on and then off with a frequency of 1 second.
 */
void blink(void) {
   volatile uint8_t numDelays = 5;
   volatile uint8_t i = 0;

   while (1) {
      // Then it turns off the LED and delays
      led_off();
      __asm__ volatile ("push r4");
      for (i = 0; i < numDelays; i++) {
         _delay_ms(50);
      }

      // Then it turns on the LED and delays
      led_on();
      __asm__ volatile ("pop r4");
      for (i = 0; i < numDelays; i++) {
         _delay_ms(50);
      }
   }
}

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
