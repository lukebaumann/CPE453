#include "ext2.h"
#include "program4.h"

char buffer[BLOCK_SIZE];
FILE *fp;

int main(int argc, char *argv[]) {
   char *ext2Location = 0;
   struct ext2_super_block sb;
   struct ext2_group_desc gd;
   struct ext2_inode rootDir;
   char blockOfInodes[BLOCK_SIZE];

   ext2Location = argv[1];

   if ((fp = fopen(ext2Location, O_RDONLY)) < 0) {
      perror("main: Error on open"); 
      exit(-1);
   }

   findSuperBlock(fp, &sb);
   printSuperBlockInfo(&sb);

   findGroupDescriptor(fp, &gd);
   printGroupDescriptorInfo(&gd);

   findInode(&rootDir, ROOT_DIR_INODE_OFFSET, &blockOfInodes);
   printRootDirectory(&rootDir);
}

void findBlockOfInodes(FILE *fp, int blockOffset, char *blockOfInodes) {
   readBlock(fp, INODE_TABLE_BLOCK_INDEX + blockOffset, blockOfInodes);

   return;
}

void findInode(struct ext2_inode *inode, int inodeOffset) {
   char blockOfInodes[BLOCK_SIZE];

   findBlockOfInodes(fp, (inodeOffset) / 7977, blockOfInodes); 
   memcpy(inode, blockOfInodes + (struct ext2_inode) *
         (inodeOffset % 7977), sizeof(struct ext2_inode));

   return;
}

void readBlock(FILE *fp, int blockIndex, void *destination) {
   if (fseek(fp, blockIndex * BLOCK_SIZE, SEEK_SET) < 0) {
      perror("readBlock: Error in fseek");
   }

   if (fread(fp, destination, BLOCK_SIZE) < BLOCK_SIZE) {
      perror("readBlock: Error in fread");
   }

   return;
}

void findSuperBlock(FILE *fp, struct ext2_super_block *sb) {
   readBlock(fp, SUPER_BLOCK_INDEX, buffer);

   memcpy(sb, buffer, sizeof(struct ext2_super_block));

   return;
}

void findGroupDescriptor(FILE *fp, struct ext2_group_desc *gd) {
   readBlock(fp, GROUP_DESC_BLOCK_INDEX, buffer);

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

void printRootDirectory(struct ext2_ext2_inode *rootDir) {
   struct ext2_dir_entry *entry;
   int i = 0;

   for (i = 0; i < EXT2_N_BLOCKS; i++) {
      entry = r
      printf("%60s\trootDir->i_block[i]; 
   }
}

