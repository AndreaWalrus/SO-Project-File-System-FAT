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

    printFAT(fat);
    int entry = createFile("pippo\0", buffer);
    int offset = getOffset(entry);
    FileEntry file = (FileEntry)(buffer + offset);

    printFAT(fat);
    createFile("pluto\0", buffer);
    createFile("paperone\0", buffer);

    printFileEntryList(buffer);
    
    eraseFile(find("pluto\0", buffer, 0, 0), buffer);
    printFileEntryList(buffer);
    printFAT(fat);
    
    createFile("topolino\0", buffer);
    printFileEntryList(buffer);

    FileHandleEntry handle = openFile(file);
    printFileHandleTable();
    int data[550];
    for(int i=0; i<550; i++){
        data[i]=i;
    }
    write(handle, buffer, data, 550);
    printFileHandleTable();
    printFAT(fat);
    printFileEntryList(buffer);

    seek(handle, buffer, 0);
    printFileHandleTable();
    char results[550];
    read(handle, (void*) results, buffer, 550);
    printf("Data:\n");
    for(int i=0; i<550; i++){
        printf("%u ", (unsigned char)results[i]);
        if(i%32==0 && i!=0) printf("\n");
    }
    printf("\n");
    printFileHandleTable();

    closeFile(handle);

    eraseFile(find("pippo\0", buffer, 0, 0), buffer);
    printFileEntryList(buffer);

    listDir(buffer);
    createDir("pippo\0", buffer);
    listDir(buffer);
    changeDir("pippo\0", buffer);
    listDir(buffer);

    printFileEntryList(buffer);
    
    //Cleanup

    munmap(buffer, BLOCK_SIZE * BLOCKS_NUM);
    if(errno){
        fprintf(stderr, "munmap error");
        return -1;
    }


    return 0;
}