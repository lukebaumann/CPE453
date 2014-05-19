#ifndef __PROGRAM4_H__
#define __PROGRAM4_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define GARBAGE_SIZE 1024

void findSuperBlock(int fd, struct ext2_super_block *sb);
void printSuperBlockInfo(struct ext2_super_block *sb);

#endif
