#define BLOCK_SIZE 512 // Blocks of 512 bytes
#define BLOCKS_NUM 1024 // Total number of blocks and FAT entries

#pragma once

struct FAT{
    int blocks[BLOCKS_NUM]; 
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

int init_fat(FATEntry fat, char* buffer);
int find_free_block(FATEntry fat);
int allocate_block(FATEntry fat);
int free_block(FATEntry fat, unsigned int block_index);

int createFile(FileEntry file, const char* name);
int eraseFile(FileEntry file);

int create_dir(FileEntry file, const char* name);
