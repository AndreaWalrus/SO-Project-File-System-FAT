#include "FAT_structures.h"

int test1(char* buffer){

    FATEntry fat = init_fat(buffer);
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
}