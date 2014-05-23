#ifndef __PROGRAM4_H__
#define __PROGRAM4_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_STRING_LENGTH 256

#define SECTORS_PER_BLOCK 2

#define SECTOR_SIZE 512
#define SUPER_BLOCK_INDEX 1
#define GROUP_DESC_BLOCK_INDEX 2
#define INODE_TABLE_BLOCK_INDEX 5

#define ROOT_DIR_INODE_OFFSET 2

#define FILE_MODE_TYPE_MASK 0xF000
#define FIFO 0x1000
#define CHARACTER_DEVICE 0x2000
#define DIRECTORY 0x4000
#define BLOCK_DEVICE 0x5000
#define REGULAR_FILE 0x8000
#define SYMBOLIC_LINK 0xA000
#define UNIX_SOCKET 0xC000

void findInode(struct ext2_inode *inode, int inodeNumber);
void findSuperBlock(struct ext2_super_block *sb);
void findGroupDescriptor(struct ext2_group_desc *gd);
void printSuperBlockInfo(struct ext2_super_block *sb);
void printGroupDescriptorInfo(struct ext2_group_desc *gd);
void printInode(struct ext2_inode *inode);
void printDirectory(struct ext2_inode *dirInode);
void printData(struct ext2_inode *inode);
uint8_t getTypeName(uint16_t mode, char *typeBuffer);
void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size);

#endif
