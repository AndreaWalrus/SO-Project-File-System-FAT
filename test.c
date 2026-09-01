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

/*     printFAT(fat);
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

    printFileEntryList(buffer); */

    // Active loop
    while(1){
        if(current_directory==-1){
            printf("root/: ");
        }
        else{
            FileEntry file = getFileEntry(current_directory, buffer);
            printf("%s/: ", file->name);
        }
        
        char input[32];
        fgets(input, 32, stdin);
        char * command = strtok(input, " \n");
        if(command==NULL) continue;
        if(!strcmp(command, "exit\0")){
            printf("Exiting...\n");
            break;
        }
        else if(!strcmp(command, "help\0")){
            printf("Available commands:\n");
            printf("-listDir: Lists all the directories in the current path\n");
            printf("-changeDir: Changes current directory\n");
        }
        else if(!strcmp(command, "createFile\0")){
            command = strtok(NULL, " \n");
            if(command==NULL){
                printf("Invalid argument\n");
                continue;
            }
            createFile(command, buffer);
        }
        else if(!strcmp(command, "eraseFile\0")){
            char* name = strtok(NULL, " \n");
            if(name==NULL){
                printf("Invalid argument\n");
                continue;
            }
            int res = find(name, buffer, 0, 0);
            if(res>=0) eraseFile(res, buffer);
        }
        else if(!strcmp(command, "openFile\0")){
            char* name = strtok(NULL, " \n");
            if(name==NULL){
                printf("Invalid argument\n");
                continue;
            }
            int res = find(name, buffer, 0, 0);
            FileHandleEntry handle;
            if(res>=0){
                handle = openFile(res, buffer);
                printf("File mode, available commands:\n-write\n-read\n-seek\n");
                while(1){
                    fgets(input, 32, stdin);
                    command = strtok(input, " \n");
                    if(command==NULL) continue;

                    if(!strcmp(command, "write\0")){
                        int wrote = 0;
                        char* token = strtok(NULL, " \n");
                        if(token==NULL){
                            printf("Invalid argument\n");
                            continue;
                        }
                        while(token != NULL){
                            wrote+=write(handle, buffer, token, strlen(token));
                            wrote+=write(handle, buffer, " ", 1);
                            token = strtok(NULL, " \n");
                        }
                        seek(handle, buffer, wrote-1);
                        write(handle, buffer, "", 1);
                        wrote--;
                        printf("Wrote %d bytes\n", wrote);
                    }
                    else if(!strcmp(command, "read\0")){
                        command = strtok(NULL, " \n");
                        if(command==NULL){
                            printf("Invalid argument\n");
                            continue;
                        }
                        size_t size = (size_t) strtoul(command,NULL, 10);
                        char dest[size];
                        int bytes = read(handle, dest, buffer, size);
                        if(bytes==-1) continue;
                        printf("%s\n", dest);
                    }
                    else if(!strcmp(command, "seek\0")){
                        command = strtok(NULL, " \n");
                        if(command==NULL){
                            printf("Invalid argument\n");
                            continue;
                        }
                        seek(handle, buffer, (unsigned int) strtoul(command, NULL, 10));
                    }
                    else if(!strcmp(command, "closeFile\0")){
                        closeFile(handle);
                        printf("Exiting File mode...\n");
                        break;
                    }
                    else{
                        printf("Invalid command\n");
                    }
                }
            }
        }
        else if(!strcmp(command, "listDir\0")){
            listDir(buffer);
        }
        else if(!strcmp(command, "changeDir\0")){
            command = strtok(NULL, " \n");
            if(command==NULL){
                printf("Invalid argument\n");
                continue;
            }
            changeDir(command, buffer);
        }
        else if(!strcmp(command, "createDir\0")){
            command = strtok(NULL, " \n");
            if(command==NULL){
                printf("Invalid argument\n");
                continue;
            }
            createDir(command, buffer);
        }
        else if(!strcmp(command, "eraseDir\0")){
            command = strtok(NULL, " \n");
            if(command==NULL){
                printf("Invalid argument\n");
                continue;
            }
            eraseDir(command, buffer);
        }
        else if(!strcmp(command, "listFile\0")){
            listFile(buffer);
        }
        else if(!strcmp(command, "printFAT\0")){
            printFAT(buffer);
        }
        else if(!strcmp(command, "printFile\0")){
            char* name = strtok(NULL, " \n");
            if(name==NULL){
                printf("Invalid argument\n");
                continue;
            }
            int res = find(name, buffer, 0, 0);
            if(res>=0) printFile(res,buffer);
        }
        else if(!strcmp(command, "printFileEntry\0")){
            printFileEntry(buffer);
        }
        else if(!strcmp(command, "printFHT\0")){
            printFileHandleTable();
        }
        else{
            printf("Invalid command, type help for available commands\n");
        }
        
    }
    
    // Cleanup

    munmap(buffer, BLOCK_SIZE * BLOCKS_NUM);
    if(errno){
        fprintf(stderr, "munmap error");
        return -1;
    }


    return 0;
}