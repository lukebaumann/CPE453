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
#include "ext2.h"

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
#define NUMBER_OF_BUFFERS 2
#define BUFFER_SIZE 256

// program5.c
#define MAX_NUMBER_OF_ENTRIES 5

void main();
void handleKeys();
void display_stats();
void printThreadStats(uint8_t threadIndex, uint8_t threadCount);
void playback(void);
void reader(void);

// ext2.c
void findInode(struct ext2_inode *inode, int inodeNumber);
uint32_t ext2_init(struct ext2_dir_entry **entries);
enum BLOCK_STATE {DIRECT, INDIRECT, DOUBLE_INDIRECT};
void findSuperBlock(struct ext2_super_block *sb);
uint32_t getMusicDirectoryEntries(struct ext2_dir_entry **entries);
int compare(const void *p1, const void *p2);
uint32_t getDirectoryEntries(struct ext2_inode *dirInode,
      struct ext2_dir_entry **entries);
uint32_t directBlockDirectoryReading(struct ext2_dir_entry **entries,
      uint32_t blockToReadFrom);
void ourMemcpy(void *destination, void *source, uint32_t size);
#define BLOCK_SIZE 1024
#define INDIRECT_BLOCKS_PER_ADDRESS 256
#define SECTOR_SIZE 512
#define SECTORS_PER_BLOCK 2
#define SUPER_BLOCK_INDEX 1
#define INODE_TABLE_BLOCK_INDEX 5
#define ROOT_DIR_INODE_OFFSET 2
#define FILE_MODE_TYPE_MASK 0xF000
#define REGULAR_FILE 0x8000
#define DIRECTORY 0x4000

#endif
