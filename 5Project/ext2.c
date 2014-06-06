#include <stdlib.h>
#include "globals.h"

static uint32_t inodesPerGroup = 0;
static uint32_t sectorsPerGroup = 0;

uint32_t ext2_init(struct ext2_dir_entry **entries) {
   //print_string("In ext2\n\r");
   struct ext2_super_block *sb = malloc(sizeof(struct ext2_super_block));

   findSuperBlock(sb);
   inodesPerGroup = sb->s_inodes_per_group;
   sectorsPerGroup = 2 * sb->s_blocks_per_group;

   free(sb);

   return getMusicDirectoryEntries(entries);
}

/*
 * Calls directBlockFileReading after it gets the block number
 * from either the direct block addresses or indirect block addresses
 */
uint32_t getNextBlockNumber(struct ext2_inode *inode) {
   static enum BLOCK_STATE state = DIRECT;
   static struct ext2_inode *currentInode = NULL;
   static uint32_t sizeRemaining = 0;
   static uint32_t numberOfBlocksLeft = 0;
   static uint32_t blocksRead = 0;
   uint32_t blockAddressOffset = 0;
   uint32_t blockNumber = 0; 

   if (currentInode == NULL || currentInode != inode) {
      state = DIRECT;
      currentInode = inode;
      sizeRemaining = currentInode->i_size;
      numberOfBlocksLeft = currentInode->i_size / BLOCK_SIZE + 1;
      blocksRead = 0;
   }
   else {
      if (blocksRead == EXT2_NDIR_BLOCKS) {
         state = INDIRECT;
      }
      else if (blocksRead == EXT2_NDIR_BLOCKS + INDIRECT_BLOCKS_PER_ADDRESS) {
         state = DOUBLE_INDIRECT;
      }
   }

   
   print_string("numberOfBlocksLeft: ");
   print_int(numberOfBlocksLeft);
   print_string("\n\r");
   print_string("blocksRead: ");
   print_int(blocksRead);
   print_string("\n\r");

   if (!numberOfBlocksLeft) {
      return 0;
   }

   if (state == DIRECT) {
      print_string("Direct\r\n");
      blockNumber = inode->i_block[blocksRead];

      print_string("blockNumber: ");
      print_int(blockNumber);
      print_string("\n\r");
   }

   else if (state == INDIRECT) {
      print_string("Indirect\r\n");
      blockAddressOffset = blocksRead - EXT2_NDIR_BLOCKS;

      if (blockAddressOffset * sizeof(uint32_t) < SECTOR_SIZE) {
         sdReadData(inode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK,
               blockAddressOffset * sizeof(uint32_t),
               (uint8_t *) &blockNumber, sizeof(uint32_t));
      }
      else {
         sdReadData(inode->i_block[EXT2_IND_BLOCK] * SECTORS_PER_BLOCK + 1,
               blockAddressOffset * sizeof(uint32_t) - SECTOR_SIZE,
               (uint8_t *) &blockNumber, sizeof(uint32_t));
      }

      print_string("blockNumber: ");
      print_int(blockNumber);
      print_string("\n\r");
   }

   else if (state == DOUBLE_INDIRECT) {
      blockAddressOffset = blocksRead - EXT2_NDIR_BLOCKS - INDIRECT_BLOCKS_PER_ADDRESS;

      uint32_t indirectBlockAddress = 0;

      if (blockAddressOffset / INDIRECT_BLOCKS_PER_ADDRESS * sizeof(uint32_t) < SECTOR_SIZE) {
         sdReadData(inode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK,
               blockAddressOffset / INDIRECT_BLOCKS_PER_ADDRESS *
               sizeof(uint32_t), (uint8_t *) &indirectBlockAddress,
               sizeof(uint32_t));
      }
      else {
         sdReadData(inode->i_block[EXT2_DIND_BLOCK] * SECTORS_PER_BLOCK + 1,
               blockAddressOffset / INDIRECT_BLOCKS_PER_ADDRESS *
               sizeof(uint32_t) - SECTOR_SIZE, (uint8_t *) &indirectBlockAddress,
               sizeof(uint32_t));
      }

      if (blockAddressOffset % INDIRECT_BLOCKS_PER_ADDRESS * sizeof(uint32_t) < SECTOR_SIZE) {
         sdReadData(indirectBlockAddress * SECTORS_PER_BLOCK,
               blockAddressOffset % INDIRECT_BLOCKS_PER_ADDRESS *
               sizeof(uint32_t), (uint8_t *) &blockNumber,
               sizeof(uint32_t));
      }
      else {
         sdReadData(indirectBlockAddress * SECTORS_PER_BLOCK + 1,
               blockAddressOffset % INDIRECT_BLOCKS_PER_ADDRESS *
               sizeof(uint32_t) - SECTOR_SIZE, (uint8_t *) &blockNumber,
               sizeof(uint32_t));
      }
   }

   blocksRead++;
   numberOfBlocksLeft--;

   return blockNumber;
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

   sdReadData(groupOffset * sectorsPerGroup + SECTORS_PER_BLOCK *
         INODE_TABLE_BLOCK_INDEX + sectorOffset,
         inodeSectorOffset * sizeof(struct ext2_inode),
         (uint8_t *) inode, sizeof(struct ext2_inode));

   return;
}

uint32_t getMusicDirectoryEntries(struct ext2_dir_entry **entries) {
	//print_string("In getMusicDirectoryEntries\n\r");
   struct ext2_inode *dirInode = malloc(sizeof(struct ext2_inode));

   findInode(dirInode, ROOT_DIR_INODE_OFFSET);
   uint32_t blockToReadDirectoryEntriesFrom = dirInode->i_block[0];
   free(dirInode);

   uint32_t i = 0;
   uint32_t numberOfMusicEntries =
      directBlockDirectoryReading(entries, blockToReadDirectoryEntriesFrom);


   //print_string("Left getDirectoryEntries\n\r");

   qsort(entries, numberOfMusicEntries,
         sizeof(struct ext2_dir_entry *), compare);

   return numberOfMusicEntries;
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

uint32_t directBlockDirectoryReading(struct ext2_dir_entry **entries,
      uint32_t blockToReadFrom) {
   uint8_t buffer[SECTOR_SIZE];
   uint32_t i = 0;
   uint32_t filesInDirectory = 0;
   uint32_t sizeReadAlready = 0;
   uint32_t entryLength = 0;
   struct ext2_dir_entry *entry;
   struct ext2_inode inode;

   //We are reading only 1 sector's-worth  of directory entries
   sdReadData(blockToReadFrom * SECTORS_PER_BLOCK, 0, buffer, SECTOR_SIZE);

   for (i = 0; sizeReadAlready < SECTOR_SIZE - sizeof(struct ext2_dir_entry) &&
         filesInDirectory < MAX_NUMBER_OF_ENTRIES; i++) {
      entry = (struct ext2_dir_entry *) (buffer + sizeReadAlready);
      entryLength = entry->rec_len;

      findInode(&inode, entry->inode);
      //print_string(entry->name);
      //print_string("\n\r");
      //print_int(entryLength);
      //print_string("\n\r");
      

      //print_hex32(inode.i_mode);
      //print_string("\n\r");
      if ((inode.i_mode & FILE_MODE_TYPE_MASK) == REGULAR_FILE) {
         entries[filesInDirectory] = malloc(entryLength);

         ourMemcpy(entries[filesInDirectory], entry, entryLength);

         filesInDirectory++;
      }

      sizeReadAlready += entryLength;
   }

   //print_int(filesInDirectory);
   //print_string("\n\r");
   //print_string("Out of directBlockDirectoryReading loop\n\r");

   return filesInDirectory;
}

void ourMemcpy(void *destination, void *source, uint32_t size) {
   while (size) {
      ((uint8_t *) destination)[size - 1] = ((uint8_t *) source)[size - 1]; 
      size--;
   }
}

