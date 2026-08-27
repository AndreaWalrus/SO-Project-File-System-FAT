#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>


#define BLOCK_SIZE 512 // Blocks of 512 bytes
#define BLOCKS_NUM 16 // Total number of blocks and FAT entries

typedef int16_t fat_entry_t;

// File structure size in buffer: 48 bytes for name + 2 bytes for start_block + 4 bytes for size + 1 byte for is_directory + 1 for is_used + 4 bytes for file_index + 4 bytes of padding = 64 bytes
#define FILE_ENTRY_SIZE 64
#define MAX_OPENED_FILE 4

#define FAT_SIZE ((BLOCKS_NUM*sizeof(fat_entry_t)+BLOCK_SIZE-1) / BLOCK_SIZE) // Number of blocks occupied by the FAT itself rounded up
#define FILE_ENTRY_BLOCKS ((BLOCKS_NUM*FILE_ENTRY_SIZE) / BLOCK_SIZE) // Number of blocks occupied by the FileEntries, fixed amount based on the number of blocks
#define BLOCKS_AVAILABLE (BLOCKS_NUM - FAT_SIZE) // Number of blocks available for files and directories

#define FAT_FREE (fat_entry_t)-1 // Free block flag
#define FAT_EOC  (fat_entry_t)-2 // End of chain flag
#define FAT_RSVD (fat_entry_t)-3 // Reserved blocks for the FAT itself

struct File{
    char name[48];
    fat_entry_t start_block; 
    unsigned int size; // in bytes
    uint8_t is_directory; // 1 if directory, 0 if file
    uint8_t is_used; // 1 if yes, 0 otherwise
    unsigned int file_index; // index of the file in the FileEntries list
};

typedef fat_entry_t *FATEntry;
typedef struct File *FileEntry;

struct FileHandle{
    unsigned int file_index; // Index position of the opened file in the FileEntries list
    unsigned int position; // Cursor position
    uint8_t is_used; // 1 if yes, 0 otherwise
};

typedef struct FileHandle *FileHandleEntry;
static struct FileHandle FileHandleTable[MAX_OPENED_FILE];

FATEntry init_fat(char* buffer); // Initializes the FAT and the FileEntry Directory
fat_entry_t find_free_block(FATEntry fat); // Scans the FAT for the first available free block
fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block); // Allocate a single block of the FAT
fat_entry_t free_block(FATEntry fat, fat_entry_t block_index); // Frees a single block of the FAT
fat_entry_t extend_chain(FATEntry fat, fat_entry_t start_block, unsigned int block_num); // Adds a block to the end of the chain
int erase_chain(FATEntry fat, fat_entry_t start_block); // Removes the entire chain, returns the number of blocks erased

int createFile(const char* name, char* buffer); // Create a file on the first available file entry and free block, returns index of the file entry list
int eraseFile(int file_index, char* buffer);
int getOffset(unsigned int file_index);
int getIndex(FileEntry file);
FileEntry getFileEntry(unsigned int file_index, char* buffer);
FileHandleEntry openFile(FileEntry file);
int closeFile(FileHandleEntry handle);
int findFile(const char* name, char* buffer);

int write(FileHandleEntry handle, char* buffer, const void* data, size_t size);
int read(FileHandleEntry handle, void* buffer, size_t size);


int createDir(const char* name);

// Testing functions
void printFAT(FATEntry fat);
void printFile(FileEntry file);
void printFileEntryList(char* buffer);
void printFileHandleTable();