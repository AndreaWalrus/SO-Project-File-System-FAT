#include <sys/mman.h>
#include "FAT_structures.h"

int test1(){

    // Maps the buffer to simulate memory

    char* buffer = mmap(NULL, BLOCK_SIZE * BLOCKS_NUM, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return 1;
    }

    int fat = init_fat(buffer);
    if(fat==-1){
        return -1;
    }

    printFAT(buffer);

    int index = createFile("pippo\0", buffer);

    printFAT(buffer);
    createFile("pluto\0", buffer);
    createFile("paperone\0", buffer);

    printFileEntry(buffer);
    
    eraseFile(find("pluto\0", buffer, 0, 0), buffer);
    printFileEntry(buffer);
    printFAT(buffer);
    
    createFile("topolino\0", buffer);
    printFileEntry(buffer);

    FileHandleEntry handle = openFile(index, buffer);
    printFileHandleTable();
    size_t size = 550; // Bytes to write
    char data[size];
    // Writes the alphabet in loop
    for(int i=0; i<size; i++){
        data[i]=(char) ((i%26)+97);
    }
    fs_write(handle, buffer, data, size);
    printFileHandleTable();
    printFAT(buffer);
    printFileEntry(buffer);

    seek(handle, buffer, 0);
    printFileHandleTable();

    char results[550];
    fs_read(handle, (void*) results, buffer, size);
    printf("Data:\n");
    for(int i=0; i<550; i++){
        printf("%c", results[i]);
        if(i%32==0 && i!=0) printf("\n");
    }
    printf("\n");
    printFile(find("pippo\0", buffer, 0, 0), buffer);
    printFileHandleTable();
    closeFile(handle);
    printFileHandleTable();

    eraseFile(find("pippo\0", buffer, 0, 0), buffer);
    printFileEntry(buffer);

    listDir(buffer);
    createDir("pippo\0", buffer);
    listDir(buffer);
    changeDir("pippo\0", buffer);
    listDir(buffer);

    printFileEntry(buffer);

    // Cleanup

    munmap(buffer, BLOCK_SIZE * BLOCKS_NUM);
    if(errno){
        fprintf(stderr, "munmap error");
        return -1;
    }

    return 0;
}