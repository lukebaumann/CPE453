#include "program4.h"

FILE *fp;
int inodesPerGroup = 0;
int sectorsPerGroup = 0;

int main(int argc, char *argv[]) {
   char *ext2Location = 0;
   struct ext2_super_block sb;
   struct ext2_group_desc gd;
   struct ext2_inode inode;
   char *desiredPath;

   ext2Location = argv[1];
   if (argc > 2) {
      desiredPath = argv[2];
   }
   else {
      desiredPath = "/";
   }

   if ((fp = fopen(ext2Location, "r")) == NULL) {
      perror("main: Error on fopen"); 
      exit(-1);
   }

   findSuperBlock(&sb);
   inodesPerGroup = sb.s_inodes_per_group;
   sectorsPerGroup = 2 * sb.s_blocks_per_group;

   if (findFile(&inode, desiredPath)) {  
      printData(&inode);
   }
   else {
      printf("Cannot find: %s\n", desiredPath);
   }
}

uint8_t findFile(struct ext2_inode *inode, char *desiredPath) {
   char currentPath[MAX_STRING_LENGTH];
   struct ext2_dir_entry *entries[MAX_DIR_ENTRIES];
   uint32_t numberOfDirectoryEntries;
   uint8_t found = 0;
   struct ext2_inode tempInode;
   uint32_t i = 0;
   uint32_t currentPathLength = 0;

   findInode(inode, ROOT_DIR_INODE_OFFSET);

   if (!strcmp(desiredPath, "/")) {
      found = 1;
   }
   else {
      currentPath[0] = '\0';
      found = findFileRecursive(inode, desiredPath, currentPath, 0);
   }

   return found;
}

uint8_t findFileRecursive(struct ext2_inode *inode, char *desiredPath, char *currentPath, uint32_t currentPathLength) {
   struct ext2_dir_entry *entries[MAX_DIR_ENTRIES];
   uint32_t numberOfDirectoryEntries;
   uint8_t found = 0;
   struct ext2_inode tempInode;
   uint32_t i = 0;

   numberOfDirectoryEntries = getDirectories(inode, entries);

   for (i = 0; i < numberOfDirectoryEntries; i++) {
      findInode(&tempInode, entries[i]->inode);
      currentPath[currentPathLength] = '/';
      strncpy(currentPath + currentPathLength + 1, entries[i]->name, entries[i]->name_len);
      currentPath[currentPathLength + 1 + entries[i]->name_len] = '\0';

      /*printf("currentPath: %s\t", currentPath);
      printf("entry[%d]->inode: %d\n", i, entries[i]->inode);
      printInode(&tempInode);
*/
      if (!strcmp(currentPath, desiredPath)) {
         found = 1;
         memcpy(inode, &tempInode, sizeof(struct ext2_inode));
         break;
      }
      else if ((tempInode.i_mode & FILE_MODE_TYPE_MASK) == DIRECTORY &&
            !strncmp(currentPath, desiredPath, currentPathLength + 1 + entries[i]->name_len) &&
            findFileRecursive(&tempInode, desiredPath,
            currentPath, currentPathLength + 1 + entries[i]->name_len)) {
         found = 1;
         memcpy(inode, &tempInode, sizeof(struct ext2_inode));
         break;
      }
   }

   return found;
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
      printRegularFile(inode);
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

void printRegularFile(struct ext2_inode *inode) {
   uint8_t buffer[BLOCK_SIZE];
   uint32_t indirectBlockAddressBuffer[NUMBER_OF_INDIRECT_BLOCKS_PER_INDIRECT_BLOCK_ADDRESS];
   uint32_t doubleIndirectBlockAddressBuffer[NUMBER_OF_INDIRECT_BLOCKS_PER_INDIRECT_BLOCK_ADDRESS];
   uint32_t sizeRemaining = inode->i_size;
   uint32_t i = 0;
   uint32_t j = 0;
   uint32_t numberOfBlocks = inode->i_size / BLOCK_SIZE + 1;

   if (numberOfBlocks) {
      for (i = 0; sizeRemaining > 0 && i < numberOfBlocks && i < EXT2_NDIR_BLOCKS; i++) {
         read_data(inode->i_block[i] * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);

         sizeRemaining -= fwrite(buffer, 1, sizeRemaining > SECTOR_SIZE ? SECTOR_SIZE : sizeRemaining, stdout);

         if (sizeRemaining > 0) {
            read_data(inode->i_block[i] * SECTORS_PER_BLOCK + 1, 0, buffer, SECTOR_SIZE);

            sizeRemaining -= fwrite(buffer, 1, sizeRemaining > SECTOR_SIZE ? SECTOR_SIZE : sizeRemaining, stdout);
         }
      }
      numberOfBlocks -= i;
   }

   if (numberOfBlocks) {
      read_data(inode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK, 0, (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
      read_data(inode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK + 1, 0, (uint8_t *) indirectBlockAddressBuffer + SECTOR_SIZE, SECTOR_SIZE);

      for (i = 0; sizeRemaining > 0 && i < numberOfBlocks && i < NUMBER_OF_INDIRECT_BLOCKS_PER_INDIRECT_BLOCK_ADDRESS; i++) {
         read_data(indirectBlockAddressBuffer[i] * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);

         sizeRemaining -= fwrite(buffer, 1, sizeRemaining > SECTOR_SIZE ? SECTOR_SIZE : sizeRemaining, stdout);

         if (sizeRemaining > 0) {
            read_data(indirectBlockAddressBuffer[i] * SECTORS_PER_BLOCK + 1, 0, buffer, SECTOR_SIZE);

            sizeRemaining -= fwrite(buffer, 1, sizeRemaining > SECTOR_SIZE ? SECTOR_SIZE : sizeRemaining, stdout);
         }
      }
      numberOfBlocks -= i;
   }

   if (numberOfBlocks) {
      read_data(inode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK, 0, (uint8_t *) doubleIndirectBlockAddressBuffer, SECTOR_SIZE);
      read_data(inode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK + 1, 0, (uint8_t *) doubleIndirectBlockAddressBuffer + SECTOR_SIZE, SECTOR_SIZE);

      for (j = 0; sizeRemaining > 0 && j < NUMBER_OF_INDIRECT_BLOCKS_PER_INDIRECT_BLOCK_ADDRESS; j++) {
         read_data(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK, 0, (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
         read_data(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK + 1, 0, (uint8_t *) indirectBlockAddressBuffer + SECTOR_SIZE, SECTOR_SIZE);

         for (i = 0; sizeRemaining > 0 && i < numberOfBlocks && i < NUMBER_OF_INDIRECT_BLOCKS_PER_INDIRECT_BLOCK_ADDRESS; i++) {
            read_data(indirectBlockAddressBuffer[i] * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);

            sizeRemaining -= fwrite(buffer, 1, sizeRemaining > SECTOR_SIZE ? SECTOR_SIZE : sizeRemaining, stdout);

            if (sizeRemaining > 0) {
               read_data(indirectBlockAddressBuffer[i] * SECTORS_PER_BLOCK + 1, 0, buffer, SECTOR_SIZE);

               sizeRemaining -= fwrite(buffer, 1, sizeRemaining > SECTOR_SIZE ? SECTOR_SIZE : sizeRemaining, stdout);
            }
         }
         numberOfBlocks -= i;
      }
   }
}

// Need to do indirect blocks next
uint32_t getDirectories(struct ext2_inode *dirInode, struct ext2_dir_entry **entries) {
   uint8_t buffer[BLOCK_SIZE];
   uint32_t sizeReadAlready = 0;
   uint32_t i = 0;
   uint32_t numberOfDirectoryEntries = 0;
   struct ext2_dir_entry *entry;
   uint32_t entryLength = 0;
   uint32_t nameLength = 0;

   uint8_t numberOfBlocks = dirInode->i_size / BLOCK_SIZE;
   uint8_t numberOfDirectBlocks = numberOfBlocks > EXT2_NDIR_BLOCKS ? EXT2_NDIR_BLOCKS : numberOfBlocks;

   for (i = 0; i < numberOfDirectBlocks; i++) {
      //printf("dirInode->i_block[%d]: %d\n", i, dirInode->i_block[i]); 

      sizeReadAlready = 0;
      read_data(dirInode->i_block[i] * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);
      read_data(dirInode->i_block[i] * SECTORS_PER_BLOCK + 1, 0, buffer + SECTOR_SIZE, SECTOR_SIZE);

      for (; sizeReadAlready < BLOCK_SIZE; numberOfDirectoryEntries++) {
         entry = (struct ext2_dir_entry *) (buffer + sizeReadAlready);
         entryLength = entry->rec_len;

         entries[numberOfDirectoryEntries] = malloc(entryLength);

         memcpy(entries[numberOfDirectoryEntries], entry, entryLength);

         sizeReadAlready += entryLength;
         //printf("sizeReadAlready: %d\n", sizeReadAlready);
         //printf("dirInode->size: %d\n", dirInode->i_size);
      } 
   }

   return numberOfDirectoryEntries;
}

int compar(const void *p1, const void *p2) {
   struct ext2_dir_entry *entry1 = *(struct ext2_dir_entry **) p1;
   struct ext2_dir_entry *entry2 = *(struct ext2_dir_entry **) p2;

   if (!strncmp(entry1->name, ".", 1)) {
      if (entry1->name_len == 1) {
         return -1;
      }
   }
   if (!strncmp(entry2->name, ".", 1)) {
      if (entry2->name_len == 1) {
         return 1;
      }
   }
   if (!strncmp(entry1->name, "..", 2)) {
      if (entry1->name_len == 2) {
         return -1;
      }
   }
   if (!strncmp(entry2->name, "..", 2)) {
      if (entry2->name_len == 2) {
         return 1;
      }
   }

   uint8_t minNameLength = entry1->name_len < entry2->name_len ?
      entry1->name_len : entry2->name_len;

   int8_t test = strncmp(entry1->name, entry2->name, minNameLength);

   if (!test) {
      if (entry1->name_len < entry2->name_len) {
         return -1;
      }
      else if (entry1->name_len > entry2->name_len) {
         return 1;
      }
      else {
         return 0;
      }
   }
   else {
      return test;
   }
}

void printDirectory(struct ext2_inode *dirInode) {
   char typeBuffer[MAX_STRING_LENGTH];
   char nameBuffer[MAX_STRING_LENGTH];
   uint32_t i = 0;
   struct ext2_dir_entry *entries[MAX_DIR_ENTRIES];
   uint32_t numberOfDirectoryEntries = getDirectories(dirInode, entries);
   qsort(entries, numberOfDirectoryEntries, sizeof(struct ext2_dir_entry *), compar); 

   struct ext2_inode inode;
   struct ext2_dir_entry *entry;

   printf("name\tsize\ttype\n");
   for (i = 0; i < numberOfDirectoryEntries; i++) {
      entry = entries[i]; 

      if (1 && entry->inode) {
         findInode(&inode, entry->inode);

         getTypeName(inode.i_mode, typeBuffer);
         strncpy(nameBuffer, entry->name, entry->name_len);
         nameBuffer[entry->name_len] = '\0';

         printf("%s\t", nameBuffer);
         printf("%d\t", inode.i_size);
         printf("%s", typeBuffer);
         printf("\n");
      }
      free(entry);
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

void printDirectoryEntry(struct ext2_dir_entry *entry) {
   char name[MAX_STRING_LENGTH];
   strncpy(name, entry->name, entry->name_len);
   name[entry->name_len] = '\0';

   printf("Inode number: %d\n", entry->inode);
   printf("Directory entry length: %d\n", entry->rec_len);
   printf("Name length: %d\n", entry->name_len);
   printf("File name, up to EXT2_NAME_LEN: %s\n", name);

   return;
}
