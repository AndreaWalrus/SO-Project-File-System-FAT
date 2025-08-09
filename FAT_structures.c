#include <stdio.h>
#include "FAT_structures.h"

FATEntry init_fat(char* buffer){
    if (buffer == NULL) {
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    for (int i = 0; i < BLOCKS_NUM; i++) {
        fat->blocks[i] = -1;
    }
    fat->free_blocks = BLOCKS_NUM;
    return fat;  
}

int find_free_block(FATEntry fat) {
    for (int i = 0; i < BLOCKS_NUM; i++) {
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