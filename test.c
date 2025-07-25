#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "FAT_structures.h"

int main(int argc, char *argv[]) {

    char* buffer;
    mmap(buffer, BLOCK_SIZE * BLOCKS_NUM, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Initialize the FAT structure


    printf("Hello, World!\n");
    return 0;
}