#include <stdio.h>
#include "FAT_structures.h"


int init_fat(char* buffer){
    if (buffer == NULL) {
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    for (int i = 0; i < FAT_SIZE; i++) {
        fat[i] = FAT_RSVD; // Set the first blocks of the FAT to reserved for the FAT itself
    }
    for (int i = FAT_SIZE; i < BLOCKS_AVAILABLE; i++) {
        fat[i] = FAT_FREE; // Set all the remaining blocks as free
    }
    return 0;  
}

fat_entry_t find_free_block(FATEntry fat) {
    for (int i = 0; i < BLOCKS_NUM; i++) {
        if (fat[i] == FAT_FREE) {
            return i;
        }
    }
    return -1;
}

fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block) {
    fat_entry_t next_block = find_free_block(fat);
    if (next_block == -1) {
        return -1;
    }
    fat[start_block] = next_block;
    fat[next_block] = FAT_EOC;
    return next_block;
}

fat_entry_t free_block(FATEntry fat, fat_entry_t block_index) {
    if (block_index >= BLOCKS_NUM || fat[block_index] == FAT_FREE || fat[block_index] == FAT_RSVD) {
        return -1; 
    }
    fat_entry_t next_block = fat[block_index];
    fat[block_index] = FAT_FREE;
    return next_block;
}

int createFile(const char* name, char* buffer) {
    if (buffer == NULL) {
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    fat_entry_t start_block = find_free_block(fat);
    if (start_block == -1) {
        return -1;
    }
    unsigned int offset = start_block * BLOCK_SIZE;
    FileEntry file = (FileEntry)(buffer + offset);
    memcpy(file->name, name, 32);
    file->start_block = start_block;
    file->size = 0;
    file->is_directory = 0;

    fat[start_block] = FAT_EOC;
    return 0;
}

void eraseFATChain(FATEntry fat, fat_entry_t start_block) {
    while (start_block != FAT_EOC ) {
        if(start_block == FAT_RSVD)
            return;
        fat_entry_t next_block = fat[start_block];
        fat[start_block] = FAT_FREE;
        start_block = next_block;
    }
    if(start_block == FAT_RSVD)
            return;
    fat[start_block] = FAT_FREE;
}

int eraseFile(FileEntry file, char* buffer) {
    if(buffer == NULL || file == NULL) {
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    eraseFATChain(fat, file->start_block);
    return 0;
}