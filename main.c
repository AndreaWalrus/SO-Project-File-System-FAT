#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "FAT_structures.h"

int test1();

int main(int argc, char *argv[]) {

    if(argc>1){
        DEBUG=strtol(argv[1], NULL, 10);
    }else{
        DEBUG=0;
    }

    const char* path = "./FileSystem.img";
    int exists=1;

    // Check if File system image already exists

    int res = open(path, O_RDWR);
    if(res==-1){
        exists=0;
        if(DEBUG) printf("File system image doesn't exists, creating...\n");
    }else{
        if(DEBUG) printf("File system image found, loading...\n");
    }

    // Open File system image

    int fd = open(path, O_RDWR | O_CREAT, 0777);
    if(fd==-1){
        fflush(stdin);
        fprintf(stderr, "open failed: %s\n", strerror(errno));
        return 1;
    }

    if(ftruncate(fd, BLOCK_SIZE*BLOCKS_NUM)==-1){ // Truncate to buffer size
        fflush(stdin);
        fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
        return 1;
    }

    // Maps the buffer onto the File system file to simulate memory

    char* buffer = mmap(NULL, BLOCK_SIZE * BLOCKS_NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        fflush(stdin);
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return 1;
    }

    // Initialize the FAT structure

    if(!exists){
        int fat = init_fat(buffer);
        if(fat==-1){
            return -1;
        }
    }

    // Main always starts in the Root directory

    current_directory=ROOT_DIR;

    if(DEBUG){
        printf("FAT Size: %ld blocks\n", FAT_SIZE);
        printf("File Entries Size: %d blocks\n", FILE_ENTRY_BLOCKS);
    }
    printf("Main loop, type help for all available commands\n");

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
            printf("-help: Lists all available commands\n");
            printf("-exit: Exits the program\n");
            printf("-test1: Runs a predetermined test, it uses a separate buffer from the main loop\n");
            printf("\n-createFile <name>: Creates a file\n");
            printf("-eraseFile <name>: Erases a file\n");
            printf("-listFile: Lists all files in the current path\n");
            printf("-openFile <name>: Opens a file\n");
            printf("\n-createDir <name>: Creates a directory\n");
            printf("-eraseDir <name>: Erases a directory\n");
            printf("-listDir: Lists all directories in the current path\n");
            printf("-changeDir <name>: Changes the current directory\n");
            printf("\n-printFAT: Prints the FAT\n");
            printf("-printFile <name>: Prints file information\n");
            printf("-printFileEntry: Prints all file entries\n");
            printf("-printFHT: Prints the file handle table\n");
            printf("\nFile mode commands (after openFile):\n");
            printf("-write <text>: Writes text to the open file\n");
            printf("-read <size>: Reads bytes from the open file\n");
            printf("-seek <position>: Changes the file cursor position\n");
            printf("-closeFile: Closes the open file\n");
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
                            wrote+=fs_write(handle, buffer, token, strlen(token));
                            wrote+=fs_write(handle, buffer, " ", 1);
                            token = strtok(NULL, " \n");
                        }
                        seek(handle, buffer, wrote-1);
                        fs_write(handle, buffer, "", 1);
                        getFileEntry(handle->file_index, buffer)->size-=2;
                        wrote--;
                        seek(handle, buffer, wrote);
                        printf("Wrote %d bytes\n", wrote);
                    }
                    else if(!strcmp(command, "read\0")){
                        command = strtok(NULL, " \n");
                        if(command==NULL){
                            printf("Invalid argument\n");
                            continue;
                        }
                        size_t size = (size_t) strtoul(command,NULL, 10);
                        char dest[size+1];
                        int bytes = fs_read(handle, dest, buffer, size);
                        if(bytes==-1) continue;
                        memcpy(dest+size, "\0", 1);
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
        else if(!strcmp(command, "test1\0")){
            test1();
            current_directory=ROOT_DIR;
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

    res = munmap(buffer, BLOCK_SIZE * BLOCKS_NUM);
    if(res == -1){
        fflush(stdin);
        fprintf(stderr, "munmap error: %s\n", strerror(errno));
        return 1;
    }

    res = close(fd);
    if (res == -1) {
        fflush(stdin);
        fprintf(stderr, "close failed: %s\n", strerror(errno));
        return 1;
    }


    return 0;
}