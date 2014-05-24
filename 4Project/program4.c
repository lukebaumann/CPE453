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

   if ((fp = fopen(ext2Location, "r")) == NULL) {
      perror("main: Error on fopen"); 
      exit(-1);
   }

   findSuperBlock(&sb);
   inodesPerGroup = sb.s_inodes_per_group;
   sectorsPerGroup = 2 * sb.s_blocks_per_group;
   printSuperBlockInfo(&sb);

   findGroupDescriptor(&gd);
   printGroupDescriptorInfo(&gd);

   findInode(&rootInode, ROOT_DIR_INODE_OFFSET);
   printInode(&rootInode);

   printData(&rootInode);

}

void findInode(struct ext2_inode *inode, int inodeNumber) {
   int groupOffset = (inodeNumber - 1) / inodesPerGroup;
   int inodeGroupOffset = (inodeNumber - 1) % inodesPerGroup;
   int sectorOffset = (inodeGroupOffset * EXT2_GOOD_OLD_INODE_SIZE) / SECTOR_SIZE;
   int inodeSectorOffset = inodeGroupOffset % (SECTOR_SIZE / EXT2_GOOD_OLD_INODE_SIZE);


   read_data(groupOffset * sectorsPerGroup + SECTORS_PER_BLOCK *
         INODE_TABLE_BLOCK_INDEX + sectorOffset,
         inodeSectorOffset * sizeof(struct ext2_inode),
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
   printf("File mode: 0x%04X\n", (uint16_t) inode->i_mode);
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

void printData(struct ext2_inode *inode) {
   switch (inode->i_mode & FILE_MODE_TYPE_MASK) {
   case FIFO:
      printf("Inode Type: FIFO, unimplemented");
      break;
   case CHARACTER_DEVICE:
      printf("Inode Type: CHARACTER_DEVICE, unimplemented");
      break;
   case DIRECTORY:
      printDirectory(inode); 
      break;
   case BLOCK_DEVICE:
      printf("Inode Type: BLOCK_DEVICE, unimplemented");
      break;
   case REGULAR_FILE:
      printf("Inode Type: REGULAR_FILE, unimplemented");
      break;
   case SYMBOLIC_LINK:
      printf("Inode Type: SYMBOLIC_LINK, unimplemented");
      break;
   case UNIX_SOCKET:
      printf("Inode Type: UNIX_SOCKET, unimplemented");
      break;
   default:
      printf("Inode Type: unknown, 0x%04X", inode->i_mode & FILE_MODE_TYPE_MASK);
      exit(-1);
      break;
   }
   
   printf("\n");

   return;
}

// Major hack way to aphabetize directory entries
void printDirectory(struct ext2_inode *dirInode) {
   uint32_t directorySize = dirInode->i_size;
   uint8_t buffer[SECTOR_SIZE];
   char typeBuffer[MAX_STRING_LENGTH];
   char nameBuffer[MAX_STRING_LENGTH];
   struct ext2_dir_entry *entry; 
   uint32_t sizeReadAlready = 0;
   uint32_t i = 0;
   uint16_t fileType = 0;
   uint32_t numberOfDirectoryEntries = 0;

   struct ext2_inode inode;

   printf("name\tsize\ttype\n");
   for (i = 0; dirInode->i_block[i] != 0 && i < EXT2_NDIR_BLOCKS ; i++) {
      read_data(dirInode->i_block[i] * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);
      for (; sizeReadAlready < directorySize; numberOfDirectoryEntries++) {
         entry = (struct ext2_dir_entry *) (buffer + 
               sizeReadAlready - i * SECTOR_SIZE);

         //printf("entry->inode: %d\n", entry->inode);

         findInode(&inode, entry->inode);

         // Should use directoryEntires[i] as buffer but not enough time to
         // do things the right way
         getTypeName(inode.i_mode, typeBuffer);
         strncpy(nameBuffer, entry->name, entry->name_len);
         nameBuffer[entry->name_len] = '\0';

         printf("%s\t", nameBuffer);
         printf("%d\t", inode.i_size);
         printf("%s", typeBuffer);
         printf("\n");

         sizeReadAlready += entry->rec_len;
      }
   }
}

// returns the file type and fills the buffer with a string
// representing the file type
uint16_t getTypeName(uint16_t mode, char *typeBuffer) {
   switch (mode & FILE_MODE_TYPE_MASK) {
   case FIFO:
      strcpy(typeBuffer, "FIFO");
      break;
   case CHARACTER_DEVICE:
      strcpy(typeBuffer, "Character Device");
      break;
   case DIRECTORY:
      strcpy(typeBuffer, "Directory");
      break;
   case BLOCK_DEVICE:
      strcpy(typeBuffer, "Block Device");
      break;
   case REGULAR_FILE:
      strcpy(typeBuffer, "Regular File");
      break;
   case SYMBOLIC_LINK:
      strcpy(typeBuffer, "Symbolic Link");
      break;
   case UNIX_SOCKET:
      strcpy(typeBuffer, "Unix Socket");
      break;
   default:
      strcpy(typeBuffer, "Unknown");
      break;
   }
   return mode & FILE_MODE_TYPE_MASK;
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
