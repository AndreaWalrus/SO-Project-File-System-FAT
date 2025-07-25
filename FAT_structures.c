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