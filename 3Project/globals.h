/**
 * File: globals.h
 * Authors: Luke Baumann, Tyler Kowallis
 * CPE 453 Program 02
 * 04/25/2014
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <avr/io.h>
#include <util/delay.h>

//place defines and prototypes here
#define MAX_STRING_LENGTH 30
#define MAX_ROW 25
#define MAX_COL 80
#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define WHITE 37

// For serial.c
uint8_t write_byte(uint8_t b);
void print_string(char* s);
uint8_t myITOA10(char *string, uint32_t i);
uint8_t myITOA16(char *string, uint32_t i);
void reverseString(char *string, uint8_t stringSize);
void print_int(uint16_t i);
void print_int32(uint32_t i);
void print_hex(uint16_t i);
void print_hex32(uint32_t i);
void set_cursor(uint8_t row, uint8_t col);
void set_color(uint8_t color);
void clear_screen(void);
void testDec8(uint8_t tested);
void testDec16(uint16_t tested);
void testDec32(uint32_t tested);
void testHex8(int8_t tested);
void testHex16(uint16_t tested);
void testHex32(uint32_t tested);

#define MAX_BUFFER_SIZE 10
#define DEFAULT_CONSUME_TIME 100
#define DEFAULT_PRODUCE_TIME 100
#define STAT_DISPLAY_HEIGHT 11

// program3.c
void main();
void led_on();
void led_off();
void blink();
void display_stats();
void printThreadStats(uint8_t threadIndex, uint8_t threadCount);
void display_bounded_buffer();
void handleKeys();
void consumer();
void producer();

#endif
