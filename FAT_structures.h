#pragma once

#include <stdint.h>

#define BLOCK_SIZE 512 // Blocks of 512 bytes
#define BLOCKS_NUM 1024 // Total number of blocks and FAT entries

typedef int16_t fat_entry_t;

#define FAT_SIZE (BLOCKS_NUM*sizeof(fat_entry_t)+BLOCK_SIZE-1) / BLOCK_SIZE // Number of blocks occupied by the FAT itself rounded up
#define BLOCKS_AVAILABLE (BLOCKS_NUM - FAT_SIZE) // Number of blocks available for files and directories

#define FAT_FREE (fat_entry_t)-1 // Free block flag
#define FAT_EOC  (fat_entry_t)-2 // End of chain flag
#define FAT_RSVD (fat_entry_t)-3 // Reserved blocks for the FAT itself

// File structure size in buffer: 32 bytes for name + 4 bytes for start_block + 4 bytes for size + 1 byte for is_directory + 3 bytes of padding = 44 bytes

struct File{
    char name[32];
    fat_entry_t start_block; 
    unsigned int size; // in bytes
    uint8_t is_directory; // 1 if directory, 0 if file
};

struct FileHandle{
    FileEntry file;
    unsigned int position;
};

typedef fat_entry_t *FATEntry;
typedef struct File *FileEntry;
typedef struct FileHandle *FileHandler;

int init_fat(char* buffer);
fat_entry_t find_free_block(FATEntry fat);
fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block);
fat_entry_t free_block(FATEntry fat, fat_entry_t block_index);

int createFile(const char* name, char* buffer);
int eraseFile(FileEntry file, char* buffer);

int createDir(const char* name);
