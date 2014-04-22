//Global defines

#ifndef GLOBALS_H
#define GLOBALS_H

#include <avr/io.h>
#include <util/delay.h>

typedef enum { CONTINUE = 0, FINISH, WAIT } STATE;

void led_on();
void led_off();

#endif
