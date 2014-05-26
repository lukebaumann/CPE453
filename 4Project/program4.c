#include "program4.h"

FILE *fp;
int inodesPerGroup = 0;
int sectorsPerGroup = 0;

/*
 * main program
 * usage: ext2reader ext2_file_system path
 *
 * prints out directory entries aphabetically
 * or prints out file contents to stdout
 */
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
      fclose(fp);
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

   fclose(fp);
}

/*
 * Searches the file system for the inode of the desired directory or file
 *
 * returns non-zero if found, zero is not found
 */
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

/*
 * Recursively explores the file system, looking for the inode
 * designated by the path
 */
uint8_t findFileRecursive(struct ext2_inode *inode,
      char *desiredPath, char *currentPath, uint32_t currentPathLength) {
   struct ext2_dir_entry *entries[MAX_DIR_ENTRIES];
   uint32_t numberOfDirectoryEntries;
   uint8_t found = 0;
   struct ext2_inode tempInode;
   uint32_t i = 0;

   numberOfDirectoryEntries = getDirectoryEntries(inode, entries);

   for (i = 0; i < numberOfDirectoryEntries; i++) {
      findInode(&tempInode, entries[i]->inode);
      currentPath[currentPathLength] = '/';
      strncpy(currentPath + currentPathLength + 1, entries[i]->name, entries[i]->name_len);
      currentPath[currentPathLength + 1 + entries[i]->name_len] = '\0';

      if (!strcmp(currentPath, desiredPath)) {
         found = 1;
         memcpy(inode, &tempInode, sizeof(struct ext2_inode));
         break;
      }
      else if ((tempInode.i_mode & FILE_MODE_TYPE_MASK) == DIRECTORY &&
            !strncmp(currentPath, desiredPath,
            currentPathLength + 1 + entries[i]->name_len) &&
            findFileRecursive(&tempInode, desiredPath,
            currentPath, currentPathLength + 1 + entries[i]->name_len)) {
         found = 1;
         memcpy(inode, &tempInode, sizeof(struct ext2_inode));
         break;
      }
   }

   return found;
}

/*
 * Given an inodeNumber, this function calculated the correct offset
 * and fills in the inode struct pointed to by inode.
 */
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

/*
 * This function finds the super block and fills the structure
 * pointed to by sb
 */
void findSuperBlock(struct ext2_super_block *sb) {
   read_data(2 * SUPER_BLOCK_INDEX, 0, (uint8_t *) sb,
         sizeof(struct ext2_super_block));

   return;
}

/*
 * Calls the correct printing function based on the inode mode
 */
void printData(struct ext2_inode *inode) {
   switch (inode->i_mode & FILE_MODE_TYPE_MASK) {
   case DIRECTORY:
      printDirectory(inode); 
      break;
   case REGULAR_FILE:
      printRegularFile(inode);
      break;
   default:
      printf("Inode Type, 0x%04X, is unsupported",
            inode->i_mode & FILE_MODE_TYPE_MASK);
      fclose(fp);
      exit(-1);
      break;
   }

   return;
}

/*
 * Given a block of data to read from, this function prints
 * the data to the screen
 */
void directBlockFileReading(uint32_t *sizeRemaining, uint32_t blockToReadFrom) {
   uint8_t buffer[BLOCK_SIZE];

   read_data(blockToReadFrom * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);

   *sizeRemaining -= fwrite(buffer, 1, *sizeRemaining > SECTOR_SIZE ?
         SECTOR_SIZE : *sizeRemaining, stdout);

   if (*sizeRemaining > 0) {
      read_data(blockToReadFrom * SECTORS_PER_BLOCK + 1, 0, buffer, SECTOR_SIZE);

      *sizeRemaining -= fwrite(buffer, 1, *sizeRemaining > SECTOR_SIZE ?
            SECTOR_SIZE : *sizeRemaining, stdout);
   }
}

/*
 * Calls directBlockFileReading after it gets the block number
 * from either the direct block addresses or indirect block addresses
 */
void printRegularFile(struct ext2_inode *inode) {
   uint32_t indirectBlockAddressBuffer[INDIRECT_BLOCKS_PER_ADDRESS];
   uint32_t doubleIndirectBlockAddressBuffer[INDIRECT_BLOCKS_PER_ADDRESS];
   uint32_t sizeRemaining = inode->i_size;
   uint32_t i = 0;
   uint32_t j = 0;
   uint32_t numberOfBlocksLeft = inode->i_size / BLOCK_SIZE + 1;

   if (numberOfBlocksLeft) {
      for (i = 0; sizeRemaining > 0 && i < numberOfBlocksLeft &&
            i < EXT2_NDIR_BLOCKS; i++) {
         directBlockFileReading(&sizeRemaining, inode->i_block[i]);
      }

      numberOfBlocksLeft -= i;
   }

   if (numberOfBlocksLeft) {
      read_data(inode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK, 0,
            (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
      read_data(inode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK + 1, 0,
            (uint8_t *) indirectBlockAddressBuffer + SECTOR_SIZE, SECTOR_SIZE);

      for (i = 0; sizeRemaining > 0 && i < numberOfBlocksLeft &&
            i < INDIRECT_BLOCKS_PER_ADDRESS; i++) {
         directBlockFileReading(&sizeRemaining, indirectBlockAddressBuffer[i]);
      }

      numberOfBlocksLeft -= i;
   }

   if (numberOfBlocksLeft) {
      read_data(inode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK, 0,
            (uint8_t *) doubleIndirectBlockAddressBuffer, SECTOR_SIZE);
      read_data(inode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK + 1, 0,
            (uint8_t *) doubleIndirectBlockAddressBuffer + SECTOR_SIZE,
            SECTOR_SIZE);

      for (j = 0; sizeRemaining > 0 && j < INDIRECT_BLOCKS_PER_ADDRESS; j++) {
         read_data(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK, 0,
               (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
         read_data(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK + 1,
               0, (uint8_t *) indirectBlockAddressBuffer + SECTOR_SIZE,
               SECTOR_SIZE);

         for (i = 0; sizeRemaining > 0 && i < numberOfBlocksLeft &&
               i < INDIRECT_BLOCKS_PER_ADDRESS; i++) {
            directBlockFileReading(&sizeRemaining,
                  indirectBlockAddressBuffer[i]);
         }
         numberOfBlocksLeft -= i;
      }
   }
}

/*
 * Given the block of data where the directory
 * entries are located, this function copies
 * the data into the entries array
 */
uint32_t directBlockDirectoryReading(struct ext2_dir_entry **entries,
      uint32_t numberOfDirectoryEntries, uint32_t blockToReadFrom) {
   uint8_t buffer[BLOCK_SIZE];
   uint32_t i = 0;
   uint32_t sizeReadAlready = 0;
   uint32_t entryLength = 0;
   struct ext2_dir_entry *entry;

   read_data(blockToReadFrom * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);
   read_data(blockToReadFrom * SECTORS_PER_BLOCK + 1, 0,
         buffer + SECTOR_SIZE, SECTOR_SIZE);

   for (i = 0; sizeReadAlready < BLOCK_SIZE; i++) {
      entry = (struct ext2_dir_entry *) (buffer + sizeReadAlready);
      entryLength = entry->rec_len;

      entries[numberOfDirectoryEntries + i] = malloc(entryLength);

      memcpy(entries[numberOfDirectoryEntries + i], entry, entryLength);

      sizeReadAlready += entryLength;
   }

   return i;
}

/*
 * Calls directBlockDirectoryReading for the different
 * data blocks for a directory inode
 */
uint32_t getDirectoryEntries(struct ext2_inode *dirInode,
      struct ext2_dir_entry **entries) {
   uint32_t indirectBlockAddressBuffer[INDIRECT_BLOCKS_PER_ADDRESS];
   uint32_t doubleIndirectBlockAddressBuffer[INDIRECT_BLOCKS_PER_ADDRESS];
   uint32_t i = 0;
   uint32_t j = 0;
   uint32_t numberOfDirectoryEntries = 0;
   uint8_t numberOfBlocksLeft = dirInode->i_size / BLOCK_SIZE;

   if (numberOfBlocksLeft) {
      for (i = 0; i < numberOfBlocksLeft && i < EXT2_NDIR_BLOCKS; i++) {
         numberOfDirectoryEntries += directBlockDirectoryReading(entries,
               numberOfDirectoryEntries, dirInode->i_block[i]); 
      }
      numberOfBlocksLeft -= i;
   }

   if (numberOfBlocksLeft) {
      for (i = 0; i < numberOfBlocksLeft &&
            i < INDIRECT_BLOCKS_PER_ADDRESS; i++) {
         numberOfDirectoryEntries += directBlockDirectoryReading(entries,
               numberOfDirectoryEntries, indirectBlockAddressBuffer[i]);
      }
      numberOfBlocksLeft -= i;
   }

   if (numberOfBlocksLeft) {
      read_data(dirInode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK, 0,
            (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
      read_data(dirInode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK + 1, 0,
            (uint8_t *) indirectBlockAddressBuffer + SECTOR_SIZE, SECTOR_SIZE);

      for (i = 0; i < numberOfBlocksLeft &&
            i < INDIRECT_BLOCKS_PER_ADDRESS; i++) {
         numberOfDirectoryEntries += directBlockDirectoryReading(entries,
               numberOfDirectoryEntries, indirectBlockAddressBuffer[i]); 
      }
      numberOfBlocksLeft -= i;
   }

   if (numberOfBlocksLeft) {
      read_data(dirInode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK, 0,
            (uint8_t *) doubleIndirectBlockAddressBuffer, SECTOR_SIZE);
      read_data(dirInode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK + 1, 0,
            (uint8_t *) doubleIndirectBlockAddressBuffer + SECTOR_SIZE,
            SECTOR_SIZE);

      for (j = 0; j * INDIRECT_BLOCKS_PER_ADDRESS < numberOfBlocksLeft &&
            j < INDIRECT_BLOCKS_PER_ADDRESS; j++) {
         read_data(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK, 0,
               (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
         read_data(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK + 1,
               0, (uint8_t *) indirectBlockAddressBuffer + SECTOR_SIZE,
               SECTOR_SIZE);

         for (i = 0; i < numberOfBlocksLeft &&
               i < INDIRECT_BLOCKS_PER_ADDRESS; i++) {
            numberOfDirectoryEntries += directBlockDirectoryReading(entries,
                  numberOfDirectoryEntries, indirectBlockAddressBuffer[i]); 
         }
         numberOfBlocksLeft -= i;
      }
   }

   return numberOfDirectoryEntries;
}

/*
 * Given two directory entry double pointers, returns -1 if the first
 * is earlier in the dictionary than the second, 1 for visa-versa, and
 * 0 if they are the same. "." will always be first and ".." will
 * always be second
 */
int compare(const void *p1, const void *p2) {
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

/*
 * Gets the contents of the directory, sorts them alphabetically,
 * and then prints the name, size, and type to the screen.
 * Type can wither be 'F' for regular file or 'd' for directory.
 * Size is 0 if the file is a directory. 
 */
void printDirectory(struct ext2_inode *dirInode) {
   char typeBuffer[MAX_STRING_LENGTH];
   char nameBuffer[MAX_STRING_LENGTH];
   uint32_t i = 0;
   struct ext2_dir_entry *entries[MAX_DIR_ENTRIES];
   uint32_t numberOfDirectoryEntries =
      getDirectoryEntries(dirInode, entries);
   qsort(entries, numberOfDirectoryEntries,
         sizeof(struct ext2_dir_entry *), compare); 

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
         printf("%d\t", (inode.i_mode & FILE_MODE_TYPE_MASK) == DIRECTORY ?
               0 : inode.i_size);
         printf("%s", typeBuffer);
         printf("\n");
      }
      free(entry);
   }
}

/* 
 * Returns the file type and fills the buffer with a string
 * representing the file type
 */
uint16_t getTypeName(uint16_t mode, char *typeBuffer) {
   switch (mode & FILE_MODE_TYPE_MASK) {
   case DIRECTORY:
      strcpy(typeBuffer, "D");
      break;
   case REGULAR_FILE:
      strcpy(typeBuffer, "F");
      break;
   default:
      strcpy(typeBuffer, "Unsupported type");
      break;
   }
   return mode & FILE_MODE_TYPE_MASK;
}

/* 
 * Reads the sectors from the file system.
 * The block argument is in terms of SD card 512 byte sectors
 */
void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size) {
   if (offset > 511) {
      printf ("Offset greater than 511.\n");
      fclose(fp);
      exit(0);
   }   

   fseek(fp,block*512 + offset,SEEK_SET);
   fread(data,size,1,fp);
}
