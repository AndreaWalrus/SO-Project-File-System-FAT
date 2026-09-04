#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define BLOCK_SIZE 512 // Must be at least FILE_ENTRY_SIZE Bytes
#define BLOCKS_NUM 16 // Total number of blocks and FAT entries

typedef int16_t fat_entry_t;

// File structure size in buffer: 44 bytes for name + 2 bytes for start_block + 4 bytes for size + 1 byte for is_directory + 1 for is_used + 4 bytes for file_index + 8 bytes for parent_index = 64 bytes
#define FILE_ENTRY_SIZE 64
#define MAX_OPENED_FILE 4 // Arbitrary number, must be < BLOCKS_NUM

#define FAT_SIZE ((BLOCKS_NUM*sizeof(fat_entry_t)+BLOCK_SIZE-1) / BLOCK_SIZE) // Number of blocks occupied by the FAT itself rounded up
#define FILE_ENTRY_NUM (BLOCK_SIZE / FILE_ENTRY_SIZE) // Number of file/dir entries that fit in a single block
#define BLOCKS_AVAILABLE (BLOCKS_NUM - FAT_SIZE - 1) // Number of blocks available for files and directories

#define FAT_FREE (fat_entry_t)-1 // Free block flag
#define FAT_EOC  (fat_entry_t)-2 // End of chain flag
#define FAT_RSVD (fat_entry_t)-3 // Reserved blocks for the FAT itself

#define ROOT_DIR (FileEntry) (buffer+(FAT_SIZE*BLOCK_SIZE)) // Root directory entry

struct File{
    char name[44];
    fat_entry_t start_block; // Starting block of the chain
    uint8_t is_directory; // 1 if directory, 0 if file
    uint8_t is_used; // 1 if yes, 0 otherwise
    unsigned int size; // in bytes
    unsigned int file_index; // index of the file in the current directory entry list
    FileEntry parent_dir; // Entry of parent directory
};

typedef fat_entry_t *FATEntry;
typedef struct File *FileEntry;

struct FileHandle{
    FileEntry file; // Entry of the opened file
    unsigned int position; // Cursor position
    uint8_t is_used; // 1 if yes, 0 otherwise
};

typedef struct FileHandle *FileHandleEntry;
static struct FileHandle FileHandleTable[MAX_OPENED_FILE];
extern FileEntry current_directory; // Stores the current directory entry
extern int DEBUG; // 1 for debug printf

// Backbone functions
int init_fat(char* buffer); // Initializes the FAT and the Root entry
fat_entry_t find_free_block(FATEntry fat); // Scans the fat and returns the first free block found
fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block); // Allocate a single block of the FAT
fat_entry_t extend_chain(FATEntry fat, fat_entry_t start_block, unsigned int block_num); // Adds block_num blocks to the end of the chain
int erase_chain(FATEntry fat, fat_entry_t start_block); // Removes the entire chain, and clears the block, returns the number of blocks erased

// Helper functions
int getIndex(FileEntry file); // Returns the index from a file entry
FileEntry getEntry(unsigned int index, fat_entry_t start_block, char* buffer); // Returns a file entry from an index and a starting block
int findFreeEntry(char* buffer); // Returns the index of the first available File Entry in the current directory
int find(const char* name, char* buffer, int is_directory); // Returns the index of a file or dir with the wanted name

// File functions
int createFile(const char* name, char* buffer); // Create a file on the first available entry, in the current directory, and free block, returns index of entry list
int eraseFile(FileEntry file, char* buffer); // Erases the file and wipes the data in the blocks previously owned by the file
FileHandleEntry openFile(int file_index, char* buffer); // Returns an handle of the file opened, stores the handle in the FileHandleTable
int closeFile(FileHandleEntry handle); // Closes the handle and frees the Table entry

// Data functions
int fs_write(FileHandleEntry handle, char* buffer, const void* data, size_t size); // Writes data in the file blocks,
// keeps track of the written bytes in the handle->position, if the blocks are not enough it extends the fat chain. Returns bytes written
int fs_read(FileHandleEntry handle, void* dest, char* buffer, size_t size); // Reads bytes from the file blocks starting from the handle->position, and stores them in dest. Returns bytes written
int seek(FileHandleEntry handle, char* buffer, unsigned int position); // Moves the handle->position to the desired position

// Directory functions
int createDir(const char* name, char* buffer); // Creates a directory in the current directory
int eraseDir(const char* name, char* buffer); // Deletes a directory in the current directory
int changeDir(const char* name, char* buffer); // Changes current directory, similar usage of bash cd

// Printing functions
void printFAT(char* buffer); // Prints the FAT Table
void printFile(int file_index, char* buffer); // Prints a single File Entry in the current directory
void printEntries(char* buffer); // Prints all the Entries tagged as used in the current directory
void printFileHandleTable(); // Prints the File Handle Table
int listDir(char* buffer); // Lists all the directories in the current directory
int listFile(char* buffer); // Lists all the files in the current directory

