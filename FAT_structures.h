#define BLOCK_SIZE 512 // Blocks of 512 bytes
#define BLOCKS_NUM 1024 // Total number of blocks and FAT entries
#define FAT_SIZE (int)(BLOCKS_NUM * 4) / BLOCK_SIZE // FAT size in blocks
#define BLOCKS_AVAILABLE BLOCKS_NUM - FAT_SIZE // Number of blocks available for files

#pragma once

// FAT structure size in buffer: [(BLOCKS_NUM-(BLOCKS_NUM * 4)/BLOCK_SIZE) * 4] + 4 bytes, so the FAT occupies the first (BLOCKS_NUM * 4)/BLOCK_SIZE blocks of the buffer
// eg. with BLOCKS_NUM = 1024, BLOCK_SIZE = 512, the FAT occupies the first (1024*4)/512 = 8 blocks of the buffer

// Given that the FAT structure doesn't fill the last block completely, we can use the remaining space for a free_blocks counter, and the remaining bytes will be left empty

struct FAT{
    int blocks[BLOCKS_AVAILABLE]; // -(BLOCKS_NUM*4)/BLOCK_SIZE to account for the first blocks reserved for the FAT itself
    unsigned int free_blocks;
};

struct File{
    char name[32];
    unsigned int start_block; 
    unsigned int size; // in bytes
    char is_directory; // 1 if directory, 0 if file
};

struct FileHandle{
    FileEntry file;
    unsigned int position;
};

typedef struct FAT *FATEntry;
typedef struct File *FileEntry;
typedef struct FileHandle *FileHandler;

int init_fat(char* buffer);
int find_free_block(FATEntry fat);
int allocate_block(FATEntry fat, unsigned int start_block);
int free_block(FATEntry fat, unsigned int block_index);

int createFile(const char* name, char* buffer);
int eraseFile(FileEntry file, char* buffer);

int createDir(const char* name);
