

// These functions will go in their own file later but for now will be right here
int inodesPerGroup = 0;
int sectorsPerGroup = 0;

void ext2_init(struct ext2_dir_entry **entries) {
   struct ext2_super_block sb;

   findSuperBlock(&sb);
   inodesPerGroup = sb.s_inodes_per_group;
   sectorsPerGroup = 2 * sb.s_blocks_per_group;

   getMusicDirectoryEntries(entries);
}

void findSuperBlock(struct ext2_super_block *sb) {
   sdReadData(2 * SUPER_BLOCK_INDEX, 0, (uint8_t *) sb,
         sizeof(struct ext2_super_block));

   return;
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

void getMusicDirectoryEntries(struct ext2_inode *dirInode,
      struct ext2_dir_entry **entries) {
   struct ext2_inode dirInode;
   findInode(&dirInode, ROOT_DIR_INODE_OFFSET);

   char typeBuffer[MAX_STRING_LENGTH];
   char nameBuffer[MAX_STRING_LENGTH];
   uint32_t i = 0;
   uint32_t numberOfMusicEntries =
      getDirectoryEntries(&dirInode, entries);

   qsort(entries, numberOfMusicEntries,
         sizeof(struct ext2_dir_entry *), compare); 
}

int compare(const void *p1, const void *p2) {
   struct ext2_dir_entry *entry1 = *(struct ext2_dir_entry **) p1;
   struct ext2_dir_entry *entry2 = *(struct ext2_dir_entry **) p2;

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

uint32_t getDirectoryEntries(struct ext2_inode *dirInode,
      struct ext2_dir_entry **entries) {
   uint32_t *indirectBlockAddressBuffer = NULL;
   uint32_t *doubleIndirectBlockAddressBuffer = NULL;
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
      indirectBlockAddressBuffer =
         malloc(sizeof(uint32_t) * INDIRECT_BLOCKS_PER_ADDRESS);
      for (i = 0; i < numberOfBlocksLeft &&
            i < INDIRECT_BLOCKS_PER_ADDRESS; i++) {
         numberOfDirectoryEntries += directBlockDirectoryReading(entries,
               numberOfDirectoryEntries, indirectBlockAddressBuffer[i]);
      }
      numberOfBlocksLeft -= i;
   }

   if (numberOfBlocksLeft) {
      sdReadData(dirInode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK, 0,
            (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
      sdReadData(dirInode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK + 1, 0,
            (uint8_t *) indirectBlockAddressBuffer + SECTOR_SIZE, SECTOR_SIZE);

      for (i = 0; i < numberOfBlocksLeft &&
            i < INDIRECT_BLOCKS_PER_ADDRESS; i++) {
         numberOfDirectoryEntries += directBlockDirectoryReading(entries,
               numberOfDirectoryEntries, indirectBlockAddressBuffer[i]); 
      }
      numberOfBlocksLeft -= i;
   }

   if (numberOfBlocksLeft) {
      sdReadData(dirInode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK, 0,
            (uint8_t *) doubleIndirectBlockAddressBuffer, SECTOR_SIZE);
      sdReadData(dirInode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK + 1, 0,
            (uint8_t *) doubleIndirectBlockAddressBuffer + SECTOR_SIZE,
            SECTOR_SIZE);

      for (j = 0; j * INDIRECT_BLOCKS_PER_ADDRESS < numberOfBlocksLeft &&
            j < INDIRECT_BLOCKS_PER_ADDRESS; j++) {
         sdReadData(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK, 0,
               (uint8_t *) indirectBlockAddressBuffer, SECTOR_SIZE);
         sdReadData(doubleIndirectBlockAddressBuffer[j] * SECTORS_PER_BLOCK + 1,
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

   if (indirectBlockAddressBuffer != NULL) {
      free(indirectBlockAddressBuffer);
   }
   if (doubleIndirectBlockAddressBuffer != NULL) {
      free(doubleIndirectBlockAddressBuffer);
   }

   return numberOfDirectoryEntries;
}

uint32_t directBlockDirectoryReading(struct ext2_dir_entry **entries,
      uint32_t numberOfDirectoryEntries, uint32_t blockToReadFrom) {
   uint8_t buffer[BLOCK_SIZE];
   uint32_t i = 0;
   uint32_t filesInDirectory = 0;
   uint32_t sizeReadAlready = 0;
   uint32_t entryLength = 0;
   struct ext2_dir_entry *entry;
   struct ext2_inode inode;

   sdReadData(blockToReadFrom * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);
   sdReadData(blockToReadFrom * SECTORS_PER_BLOCK + 1, 0,
         buffer + SECTOR_SIZE, SECTOR_SIZE);

   for (i = 0; sizeReadAlready < BLOCK_SIZE; i++) {
      entry = (struct ext2_dir_entry *) (buffer + sizeReadAlready);
      entryLength = entry->rec_len;

      findInode(&inode, entry->inode);
      if ((inode->i_mode & FILE_MODE_TYPE_MASK) == REGULAR_FILE) {
         entries[numberOfDirectoryEntries + i] = malloc(entryLength);

         memcpy(entries[numberOfDirectoryEntries + i], entry, entryLength);

         filesInDirectory++;
      }

      sizeReadAlready += entryLength;
   }

   return filesInDirectory;
}

