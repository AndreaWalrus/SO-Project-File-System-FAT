#include <stdio.h>
#include "FAT_structures.h"

int init_fat(char* buffer){
    if (buffer == NULL) {
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    for (int i = 0; i < BLOCKS_AVAILABLE; i++) {
        fat->blocks[i] = -1;
    }
    fat->free_blocks = BLOCKS_NUM-((int)(BLOCKS_NUM*4)/BLOCK_SIZE);
    return 0;  
}

int find_free_block(FATEntry fat) {
    for (int i = 0; i < BLOCKS_AVAILABLE; i++) {
        if (fat->blocks[i] == -1) {
            return i;
        }
    }
    return -1;
}

int allocate_block(FATEntry fat, unsigned int start_block) {
    int next_block = find_free_block(fat);
    if (next_block == -1) {
        return -1;
    }
    fat->blocks[start_block] = next_block;
    fat->free_blocks--;
    return next_block;
}

int free_block(FATEntry fat, unsigned int block_index) {
    if (block_index >= BLOCKS_NUM || fat->blocks[block_index] == -1) {
        return -1; 
    }
    unsigned int next_block = fat->blocks[block_index];
    fat->blocks[block_index] = -1;
    fat->free_blocks++;
    return next_block;
}

int createFile(const char* name, char* buffer) {
    if (buffer == NULL) {
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    unsigned int start_block = find_free_block(fat);
    if (start_block == -1) {
        return -1;
    }
    FileEntry file = (FileEntry)(buffer + FAT_SIZE + (start_block * BLOCK_SIZE));
    memcpy(file->name, name, 32);
    file->start_block = start_block;
    file->size = 0;
    file->is_directory = 0;

    fat->blocks[start_block] = BLOCKS_AVAILABLE;
    fat->free_blocks--;
    return 0;
}

void eraseFATChain(FATEntry fat, unsigned int start_block) {
    unsigned int current_block = start_block;
    while (current_block != -1) {
        if(current_block == BLOCKS_AVAILABLE)
            fat->blocks[current_block] = -1;
            fat->free_blocks++;
            break;
        unsigned int next_block = fat->blocks[current_block];
        fat->blocks[current_block] = -1;
        fat->free_blocks++;
        current_block = next_block;
    }

}

int eraseFile(FileEntry file, char* buffer) {
    if(buffer == NULL || file == NULL) {
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    eraseFATChain(fat, file->start_block);
    return 0;
}