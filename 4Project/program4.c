#include "ext2.h"
#include "program4.h"

FILE *fp;
int inodesPerGroup = 0;
int sectorsPerGroup = 0;

int main(int argc, char *argv[]) {
   char *ext2Location = 0;
   struct ext2_super_block sb;
   struct ext2_group_desc gd;
   struct ext2_inode rootInode;

   ext2Location = argv[1];

   printf("Hello\n");
   if ((fp = fopen(ext2Location, "r")) == NULL) {
      perror("main: Error on fopen"); 
      exit(-1);
   }

   printf("Hello\n");
   findSuperBlock(&sb);
   inodesPerGroup = sb.s_inodes_per_group;
   sectorsPerGroup = 2 * sb.s_blocks_per_group;
   printSuperBlockInfo(&sb);

   findGroupDescriptor(&gd);
   printGroupDescriptorInfo(&gd);

   findInode(&rootInode, ROOT_DIR_INODE_OFFSET);
   printInode(&rootInode);
}

void findInode(struct ext2_inode *inode, int inodeNumber) {
   int groupNumber = (inodeNumber - 1) / inodesPerGroup;
   int inodeTableSectorOffset = (inodeNumber - 1) *
      sizeof(struct ext2_inode) / SECTOR_SIZE;
   int inodeOffset = (inodeNumber - 1) % (SECTOR_SIZE /
      sizeof(struct ext2_inode));

   printf("groupNumber: %d\n", groupNumber);
   printf("inodeTableSectorOffset: %d\n", inodeTableSectorOffset);
   printf("inodeOffset: %d\n", inodeOffset);

   read_data(groupNumber * sectorsPerGroup + 2 * INODE_TABLE_BLOCK_INDEX +
         inodeTableSectorOffset, inodeOffset * sizeof(struct ext2_inode),
         (uint8_t *) inode, sizeof(struct ext2_inode));

   return;
}

void findSuperBlock(struct ext2_super_block *sb) {
   read_data(2 * SUPER_BLOCK_INDEX, 0, (uint8_t *) sb,
         sizeof(struct ext2_super_block));

   return;
}

void findGroupDescriptor(struct ext2_group_desc *gd) {
   read_data(2 * GROUP_DESC_BLOCK_INDEX, 0, (uint8_t *) gd,
         sizeof(struct ext2_group_desc));

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

void printInode(struct ext2_inode *inode) {
   struct ext2_dir_entry *entry;
   int i = 0;
   printf("File mode: %d\n", inode->i_mode);
   printf("Size in bytes: %d\n", inode->i_size);
   printf("Access time: %d\n", inode->i_atime);
   printf("Creation time: %d\n", inode->i_ctime);
   printf("Modification time: %d\n", inode->i_mtime);
   printf("Deletion Time: %d\n", inode->i_dtime);
   printf("Links count: %d\n", inode->i_links_count);
   printf("Blocks count: %d\n", inode->i_blocks);
   printf("File flags: %d\n", inode->i_flags);
   printf("File version (for NFS): %d\n", inode->i_generation);

   printf("\n");

   return;
}

//the bloc argument is in terms of SD card 512 byte sectors
void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size) {
   if (offset > 511) {
      printf ("Offset greater than 511.\n");
      exit(0);
   }   

   fseek(fp,block*512 + offset,SEEK_SET);
   fread(data,size,1,fp);
}
