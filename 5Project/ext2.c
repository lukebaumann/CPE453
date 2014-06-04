

// These functions will go in their own file later but for now will be right here
int inodesPerGroup = 0;
int sectorsPerGroup = 0;

void ext2_init() {
   struct ext2_super_block sb;

   findSuperBlock(&sb);
   inodesPerGroup = sb.s_inodes_per_group;
   sectorsPerGroup = 2 * sb.s_blocks_per_group;

}
void findSuperBlock(struct ext2_super_block *sb) {
   read_data(2 * SUPER_BLOCK_INDEX, 0, (uint8_t *) sb,
         sizeof(struct ext2_super_block));

   return;
}
