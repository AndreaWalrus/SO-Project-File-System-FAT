#include "FAT_structures.h"


FATEntry init_fat(char* buffer){
    if (buffer == NULL) {
        fprintf(stderr, "Buffer is NULL\n");
        return NULL;
    }
    FATEntry fat = (FATEntry)buffer;
    for (int i = 0; i < FAT_SIZE; i++) {
        fat[i] = FAT_RSVD; // Set the first blocks of the FAT to reserved for the FAT itself
    }
    for (int i = FAT_SIZE; i < BLOCKS_NUM; i++) {
        fat[i] = FAT_FREE; // Set all the remaining blocks as free
    }
    return fat;  
}

fat_entry_t find_free_block(FATEntry fat) {
    for (int i = 0; i < BLOCKS_NUM; i++) {
        if (fat[i] == FAT_FREE) {
            return i;
        }
    }
    fprintf(stderr, "No free blocks available\n");
    return -1;
}

fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block) {
    fat_entry_t next_block = find_free_block(fat);
    if (next_block == -1) {
        fprintf(stderr, "No free blocks available for allocation\n");
        return -1;
    }
    fat[start_block] = next_block;
    fat[next_block] = FAT_EOC;
    return next_block;
}

fat_entry_t free_block(FATEntry fat, fat_entry_t block_index) {
    if (block_index >= (fat_entry_t)BLOCKS_NUM || fat[block_index] == FAT_FREE || fat[block_index] == FAT_RSVD) {
        fprintf(stderr, "Invalid block index or block already free/reserved\n");
        return -1; 
    }
    fat_entry_t next_block = fat[block_index];
    fat[block_index] = FAT_FREE;
    return next_block;
}

fat_entry_t createFile(const char* name, char* buffer) {
    if (buffer == NULL) {
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    fat_entry_t start_block = find_free_block(fat);
    if (start_block == -1) {
        fprintf(stderr, "No free blocks available for file creation\n");
        return -1;
    }
    unsigned int offset = (int) start_block * BLOCK_SIZE;
    FileEntry file = (FileEntry)(buffer + offset);
    memcpy(file->name, name, 32);
    file->start_block = start_block;
    file->size = sizeof(struct File);
    file->is_directory = 0;

    fat[start_block] = FAT_EOC;
    return start_block;
}

void eraseFATChain(FATEntry fat, fat_entry_t start_block) {
    if (start_block < 0 || start_block >= BLOCKS_NUM) {
        fprintf(stderr, "Invalid start block index\n");
        return;
    }
    while (fat[start_block] != FAT_EOC ) {
        if(fat[start_block] == FAT_RSVD)
            return;
        fat_entry_t next_block = fat[start_block];
        fat[start_block] = FAT_FREE;
        start_block = next_block;
    }
    if(fat[start_block] == FAT_RSVD || fat[start_block] == FAT_FREE)
            return;
    fat[start_block] = FAT_FREE;
}

int eraseFile(FileEntry file, char* buffer) {
    if(buffer == NULL || file == NULL) {
        fprintf(stderr, "Buffer or file is NULL\n");
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    eraseFATChain(fat, file->start_block);
    return 0;
}

FileHandleEntry openFile(FileEntry file) {
    if (file == NULL) {
        fprintf(stderr, "File is NULL\n");
        return NULL;
    }
    FileHandleEntry handle = (FileHandleEntry)malloc(sizeof(struct FileHandle));
    handle->file = file;
    handle->position = sizeof(struct File); // Start after the file metadata
    return handle;
}

int closeFile(FileHandleEntry handle) {
    if (handle == NULL) {
        fprintf(stderr, "Handle is NULL\n");
        return -1;
    }
    free(handle);
    return 0;
}

int write(FileHandleEntry handle, char* buffer, const void* data, size_t size) {
    if(handle == NULL || data == NULL) {
        fprintf(stderr, "Handle or data is NULL\n");
        return -1;
    }
    if(handle->file == NULL) {
        fprintf(stderr, "File in handle is NULL\n");
        return -1; 
    }
    unsigned int file_size = handle->file->size - sizeof(struct File); // Bytes used for file data
    if(handle->position < sizeof(struct File)) {
        fprintf(stderr, "Cursor out of bounds\n");
        return -1;
    }
    if(handle->position < BLOCK_SIZE){ // Cursor is within the first block
        if(handle->position + file_size < BLOCK_SIZE){ // Data fits in the first block
            unsigned int offset = (handle->file->start_block * BLOCK_SIZE) + handle->position;
            memcpy(buffer + offset, data, size);
            handle->position += size;
            handle->file->size += size;
            return size;
        }
        if(handle->position + file_size >= BLOCK_SIZE){ // Data does not fit in the first block
            int i=0;
            FATEntry fat = (FATEntry)buffer;
            while((i*BLOCK_SIZE)-handle->position < size){
                fat_entry_t next_block = allocate_block(fat, handle->file->start_block);
                if (next_block == -1) {
                    fprintf(stderr, "Failed to allocate new block for writing\n");
                    return -1;
                }
                
            }
        }
    }
}

// Testing functions

void printFAT(FATEntry fat) {
    if (fat == NULL) {
        fprintf(stderr, "FAT is NULL\n");
        return;
    }
    printf("FAT Entries:\n");
    for (int i = 0; i < BLOCKS_NUM; i++) {
        printf("[%d]: %d\n", i, fat[i]);
    }
}

void printFile(FileEntry file){
    if (file == NULL) {
        fprintf(stderr, "File is NULL\n");
        return;
    }
    printf("File Name: %s\n", file->name);
    printf("Start Block: %hd\n", file->start_block);
    printf("Size: %u bytes\n", file->size);
    printf("Is Directory: %s\n", file->is_directory ? "Yes" : "No");
}