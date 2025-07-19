#define BLOCK_SIZE 512 // Blocks of 512 bytes
#define BLOCKS_NUM 1024 // Total number of blocks and FAT entries

#pragma once

struct FAT{
    int blocks[BLOCKS_NUM]; 
    unsigned int space[BLOCKS_NUM];
    unsigned int free_blocks; 
};

struct Directory{
    char name[32];
    DirectoryEntry parent;
    DirectoryEntry* childs;
    FileEntry* files;
    unsigned int num_childs;
    unsigned int num_files;
    unsigned int start_block; 
    unsigned int size; // in bytes
};

struct File{
    char name[32];
    DirectoryEntry parent;
    unsigned int start_block; 
    unsigned int size; // in bytes
    unsigned int cursor;
};

typedef struct FAT *FATEntry;
typedef struct Directory *DirectoryEntry;
typedef struct File *FileEntry;

int init_fat(FATEntry fat);
int find_free_block(FATEntry fat);

int create_dir(DirectoryEntry parent, const char* name);
int create_file(DirectoryEntry parent, const char* name);