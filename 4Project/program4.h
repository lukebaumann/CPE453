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

void readBlock(int fd, int blockIndex, void *destination);
void findSuperBlock(int fd, struct ext2_super_block *sb);
void findGroupDescriptor(int fd, struct ext2_group_desc *gd);
void printSuperBlockInfo(struct ext2_super_block *sb);
void printGroupDescriptorInfo(struct ext2_group_desc *gd);

#endif
