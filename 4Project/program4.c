#include "ext2.h"
#include "program4.h"

char buffer[BLOCK_SIZE];

int main(int argc, char *argv[]) {
   int fd = 0;
   char *ext2Location = 0;
   struct ext2_super_block sb;
   struct ext2_group_desc gd;
   struct ext2_inode rootDir;
   char inodeBlock[BLOCK_SIZE];

   ext2Location = argv[1];

   if ((fd = open(ext2Location, O_RDONLY)) < 0) {
      perror("main: Error on open"); 
      exit(-1);
   }

   findSuperBlock(fd, &sb);
   printSuperBlockInfo(&sb);

   findGroupDescriptor(fd, &gd);
   printGroupDescriptorInfo(&gd);

   findInodeTable(fd, 0, &inodeBlock); 

   findInode(&rootDir, ROOT_DIR_INODE_OFFSET, &inodeBlock);
   printRootDirectory(&rootDir);
}

void findInodeTable(int fd, int blockOffset, char *inodeBlock) {
   readBlock(fd, INODE_TABLE_BLOCK_INDEX + blockOffset, inodeBlock);

   return;
}

void findInode(struct ext2_inode *inode, int inodeOffset, char *inodeBlock) {
   memcpy(inode, inodeBlock + (struct ext2_inode) * inodeOffset,
         sizeof(struct ext2_inode));

   return;
}

void readBlock(int fd, int blockIndex, void *destination) {
   if (lseek(fd, blockIndex * BLOCK_SIZE, SEEK_SET) < 0) {
      perror("readBlock: Error in lseek");
   }

   if (read(fd, destination, BLOCK_SIZE) < BLOCK_SIZE) {
      perror("readBlock: Error in read");
   }

   return;
}

void findSuperBlock(int fd, struct ext2_super_block *sb) {
   readBlock(fd, SUPER_BLOCK_INDEX, buffer);

   memcpy(sb, buffer, sizeof(struct ext2_super_block));

   return;
}

void findGroupDescriptor(int fd, struct ext2_group_desc *gd) {
   readBlock(fd, GROUP_DESC_BLOCK_INDEX, buffer);

   memcpy(gd, buffer, sizeof(struct ext2_group_desc));

   return;
}

void printSuperBlockInfo(struct ext2_super_block *sb) {
   printf("Inodes count: %d\n", sb->s_inodes_count);
   printf("Blocks count: %d\n", sb->s_blocks_count);
   printf("Reserved blocks count: %d\n", sb->s_r_blocks_count);
   printf("Free blocks count: %d\n", sb->s_free_blocks_count);
   printf("Free inodes count: %d\n", sb->s_free_inodes_count);
   printf("First Data Block: %d\n", sb->s_first_data_block);
   printf("Block size: %d\n", sb->s_log_block_size);
   printf("Fragment size: %d\n", sb->s_log_frag_size);
   printf("Blocks per group: %d\n", sb->s_blocks_per_group);
   printf("Fragments per group: %d\n", sb->s_frags_per_group);
   printf("Inodes per group: %d\n", sb->s_inodes_per_group);
   printf("Mount time: %d\n", sb->s_mtime);
   printf("Write time: %d\n", sb->s_wtime);
   printf("Mount count: %d\n", sb->s_mnt_count);
   printf("Maximal mount count: %d\n", sb->s_max_mnt_count);
   printf("Magic signature: %d\n", sb->s_magic);
   printf("File system state: %d\n", sb->s_state);
   printf("Behavior when detecting errors: %d\n", sb->s_errors);
   printf("minor revision level: %d\n", sb->s_minor_rev_level);
   printf("time of last check: %d\n", sb->s_lastcheck);
   printf("max. time between checks: %d\n", sb->s_checkinterval);
   printf("OS: %d\n", sb->s_creator_os);
   printf("Revision level: %d\n", sb->s_rev_level);
   printf("Default uid for reserved blocks: %d\n", sb->s_def_resuid);
   printf("Default gid for reserved blocks: %d\n", sb->s_def_resgid);

   printf("\n");

   return;
}

void printGroupDescriptorInfo(struct ext2_group_desc *gd) {
   printf("Blocks bitmap block: %d\n", gd->bg_block_bitmap);
   printf("Inodes bitmap block: %d\n", gd->bg_inode_bitmap);
   printf("Inodes table block: %d\n", gd->bg_inode_table);
   printf("Free blocks count: %d\n", gd->bg_free_blocks_count);
   printf("Free inodes count: %d\n", gd->bg_free_inodes_count);
   printf("Directories count: %d\n", gd->bg_used_dirs_count);

   printf("\n");

   return;
}

