#ifndef __PROGRAM4_H__
#define __PROGRAM4_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BLOCK_SIZE 1024
#define SUPER_BLOCK_INDEX 1
#define GROUP_DESC_BLOCK_INDEX 2
#define INODE_TABLE_BLOCK_INDEX 5

#define ROOT_DIR_INODE_OFFSET 2

void findInode(struct ext2_inode *inode, int inodeNumber);
void findSuperBlock(struct ext2_super_block *sb);
void findGroupDescriptor(struct ext2_group_desc *gd);
void printSuperBlockInfo(struct ext2_super_block *sb);
void printGroupDescriptorInfo(struct ext2_group_desc *gd);
void printInode(struct ext2_inode *inode);
void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size);

#endif
