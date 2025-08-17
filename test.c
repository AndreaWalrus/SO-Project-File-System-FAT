#include <sys/mman.h>
#include "FAT_structures.h"

int main(int argc, char *argv[]) {

    char* buffer = mmap(NULL, BLOCK_SIZE * BLOCKS_NUM, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return 1;
    }

    // Initialize the FAT structure

    FATEntry fat = init_fat(buffer);

    printf("Free block: %hd\n", find_free_block(fat));

    fat[1]= FAT_EOC;
    fat_entry_t block = allocate_block(fat, 1);
    block = allocate_block(fat, 2);
    printFAT(fat);
    printf("Free block: %hd\n", find_free_block(fat));

    block = createFile("pippo", buffer);
    if (block != -1) {
        printf("File 'pippo' created at block: %hd\n", block);
    } else {
        printf("Failed to create file 'pippo'\n");
    }

    printFAT(fat);

    int offset = block * BLOCK_SIZE;
    FileEntry file = (FileEntry)(buffer + offset);
    printFile(file);
    eraseFile(file, buffer);

    printFAT(fat);
    





    return 0;
}