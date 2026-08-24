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
    printf("FAT Size: %ld\n", FAT_SIZE);
    printf("File Entries Size: %d\n", FILE_ENTRY_BLOCKS);
    printf("Free block: %hd\n", find_free_block(fat));

/*     fat[FAT_SIZE+FILE_ENTRY_BLOCKS]= FAT_EOC;
    fat_entry_t block = allocate_block(fat, FAT_SIZE+FILE_ENTRY_BLOCKS);
    block = allocate_block(fat, FAT_SIZE+FILE_ENTRY_BLOCKS+1);
    printFAT(fat);
    printf("Free block: %hd\n", find_free_block(fat)); */
    printFAT(fat);
    fat_entry_t block = createFile("pippo\0", buffer);
    if (block != -1) {
        printf("File 'pippo' created at block: %hd\n", block);
    } else {
        printf("Failed to create file 'pippo'\n");
    }

    printFAT(fat);

    int entry = find_file("pippo\0", buffer);

    int offset = (FAT_SIZE*BLOCK_SIZE)+(entry*FILE_ENTRY_SIZE);
    FileEntry file = (FileEntry)(buffer + offset);
    printFile(file);
    eraseFile(file, buffer);

    printFAT(fat);
    
    //Cleanup

    munmap(buffer, BLOCK_SIZE * BLOCKS_NUM);
    if(errno){
        fprintf(stderr, "munmap error");
        return -1;
    }


    return 0;
}