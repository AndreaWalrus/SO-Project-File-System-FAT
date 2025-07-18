#define BLOCK_SIZE 512 // Blocks of 512 bytes
#define BLOCKS_NUM 1024 // Total number of blocks and FAT entries

struct FAT{
    unsigned int FAT[BLOCKS_NUM]; // FAT entries
    unsigned int free_blocks; // Number of free blocks
};