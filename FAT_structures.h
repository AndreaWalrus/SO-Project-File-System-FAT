#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>


#define BLOCK_SIZE 512 // Blocks of 512 bytes
#define BLOCKS_NUM 16 // Total number of blocks and FAT entries

typedef int16_t fat_entry_t;

// File structure size in buffer: 48 bytes for name + 2 bytes for start_block + 4 bytes for size + 1 byte for is_directory + 1 for is_used + 4 bytes for file_index + 4 bytes for parent_index = 64 bytes
#define FILE_ENTRY_SIZE 64
#define MAX_OPENED_FILE 4 // Arbitrary number, must be < BLOCKS_NUM

#define FAT_SIZE ((BLOCKS_NUM*sizeof(fat_entry_t)+BLOCK_SIZE-1) / BLOCK_SIZE) // Number of blocks occupied by the FAT itself rounded up
#define FILE_ENTRY_BLOCKS ((BLOCKS_NUM*FILE_ENTRY_SIZE) / BLOCK_SIZE) // Number of blocks occupied by the FileEntries, fixed amount based on the number of blocks
#define BLOCKS_AVAILABLE (BLOCKS_NUM - FAT_SIZE-FILE_ENTRY_BLOCKS) // Number of blocks available for files and directories

#define FAT_FREE (fat_entry_t)-1 // Free block flag
#define FAT_EOC  (fat_entry_t)-2 // End of chain flag
#define FAT_RSVD (fat_entry_t)-3 // Reserved blocks for the FAT itself

#define ROOT_DIR -1 // Root directory index

struct File{
    char name[48];
    fat_entry_t start_block; // Starting block of the chain
    uint8_t is_directory; // 1 if directory, 0 if file
    uint8_t is_used; // 1 if yes, 0 otherwise
    unsigned int size; // in bytes
    unsigned int file_index; // index of the file in the FileEntries list
    int parent_index; // index of parent directory
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
extern int current_directory; // Stores the current directory based on file indexes, ROOT_DIR for root

FATEntry init_fat(char* buffer); // Initializes the FAT and the FileEntry Directory
fat_entry_t find_free_block(FATEntry fat); // Scans the fat and returns the first free block found
fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block); // Allocate a single block of the FAT
fat_entry_t free_block(FATEntry fat, fat_entry_t block_index); // Frees a single block of the FAT
fat_entry_t extend_chain(FATEntry fat, fat_entry_t start_block, unsigned int block_num); // Adds block_num blocks to the end of the chain
int erase_chain(FATEntry fat, fat_entry_t start_block); // Removes the entire chain, returns the number of blocks erased

int createFile(const char* name, char* buffer); // Create a file on the first available file entry and free block, returns index of the file entry list
int eraseFile(int file_index, char* buffer); // Erases the file with the corresponding index, and wipes the data in the blocks previously owned by the file
int getOffset(unsigned int file_index); // Returns the byte offset of the FileEntry based on the file index
int getIndex(FileEntry file); // Returns the index from a file entry
FileEntry getFileEntry(unsigned int file_index, char* buffer); // Returns a file entry from an index
int findFreeFileEntry(char* buffer); // Returns the index of the first available File Entry
FileHandleEntry openFile(int file_index, char* buffer); // Returns an handle of the file opened, stores the handle in the FileHandleTable
int closeFile(FileHandleEntry handle); // Closes the handle and frees the Table entry

int write(FileHandleEntry handle, char* buffer, const void* data, size_t size); // Writes data in the file blocks, keeps track of the written bytes in the handle->position, if the blocks are not enough it extends the fat chain. Returns bytes written
int read(FileHandleEntry handle, void* dest, char* buffer, size_t size); // Reads bytes from the file blocks starting from the handle->position, and stores them in dest. Returns bytes written
int seek(FileHandleEntry handle, char* buffer, unsigned int position); // Moves the handle->position to the desired position

int find(const char* name, char* buffer, int is_directory, int local_search); // Returns the index of a file or dir with the wanted name. Local search restricts the search in the current directory.

int createDir(const char* name, char* buffer); // Creates a directory in the current directory
int eraseDir(const char* name, char* buffer); // Deletes a directory in the current directory
int changeDir(const char* name, char* buffer); // Changes current directory, similar usage of bash cd
int listDir(char* buffer); // Lists all the directories in the current directory

int listFile(char* buffer); // Lists all the files in the current directory

// Testing functions
void printFAT(char* buffer); // Prints the FAT Table
void printFile(int file_index, char* buffer); // Prints a single File Entry
void printFileEntry(char* buffer); // Prints all the File Entries tagged as used
void printFileHandleTable(); // Prints the File Handle Table