#include "FAT_structures.h"

FileEntry current_directory;
int DEBUG;

// Backbone functions

int init_fat(char* buffer){
    if (buffer == NULL) {
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    memset(buffer, 0, BLOCK_SIZE*BLOCKS_NUM); // Set all the mapped buffer to 0 for precaution
    FATEntry fat = (FATEntry)buffer;
    for (int i = 0; i < FAT_SIZE+1; i++) {
        fat[i] = FAT_RSVD; // Set the first blocks of the FAT to reserved for the FAT itself + 1 for the root directory
    }
    fat[FAT_SIZE+1]=FAT_EOC; // Set the first available block as starting block of the root
    for (int i = FAT_SIZE+2; i < BLOCKS_NUM; i++) {
        fat[i] = FAT_FREE; // Set all the remaining blocks as free
    }
    // Insert the Root Entry in the reserved block
    FileEntry dir = ROOT_DIR;
    strcpy(dir->name,"root\0");
    dir->start_block=FAT_SIZE+1; // Assign the first available block to root
    dir->size=0;
    dir->file_index=0;
    dir->parent_dir=dir;
    dir->is_used=1;
    dir->is_directory=1;
    for(int i=0; i<FILE_ENTRY_NUM; i++){ // Initialize all the root entries
        dir = getEntry(i, FAT_SIZE+1, buffer);
        strcpy(dir->name,"\0");
        dir->start_block=-1;
        dir->size=0;
        dir->file_index=0;
        dir->parent_dir=ROOT_DIR;
        dir->is_used=0;
        dir->is_directory=0;
    }
    return 0;  
}

fat_entry_t find_free_block(FATEntry fat) {
    for (int i = 0; i < BLOCKS_NUM; i++) {
        if (fat[i] == FAT_FREE) {
            return i;
        }
    }
    fflush(stdout);
    fprintf(stderr, "No free blocks available\n");
    return -1;
}

fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block) {
    if(fat == NULL) {
        fflush(stdout);
        fprintf(stderr, "FAT is NULL\n");
        return -1;
    }
    if(start_block < (fat_entry_t)0 || start_block >= (fat_entry_t)BLOCKS_NUM){
        fflush(stdout);
        fprintf(stderr, "Start block invalid\n");
        return -1;
    }
    fat_entry_t next_block = find_free_block(fat);
    if (next_block == -1) {
        fflush(stdout);
        fprintf(stderr, "No free blocks available for allocation\n");
        return -1;
    }
    fat[start_block] = next_block;
    fat[next_block] = FAT_EOC;
    return next_block;
}

fat_entry_t extend_chain(FATEntry fat, fat_entry_t start_block, unsigned int block_num) {
    if(fat == NULL) {
        fflush(stdout);
        fprintf(stderr, "FAT is NULL\n");
        return -1;
    }
    while(fat[start_block] != FAT_EOC) { // Searches for the last block in the chain
        if(fat[start_block] == FAT_RSVD || fat[start_block] == FAT_FREE) {
            fflush(stdout);
            fprintf(stderr, "Invalid block in chain\n");
            return -1;
        }
        start_block = fat[start_block];
    }
    fat_entry_t next_block;
    for(int i=0; i<block_num; i++){ // Extends the chain by block_num
        next_block = allocate_block(fat, start_block);
        start_block=next_block;
    }
    return next_block;
}

int erase_chain(FATEntry fat, fat_entry_t start_block) {
    if (start_block < 0 || start_block >= BLOCKS_NUM) {
        fflush(stdout);
        fprintf(stderr, "Invalid start block index\n");
        return -1;
    }
    if(fat == NULL) {
        fflush(stdout);
        fprintf(stderr, "FAT is NULL\n");
        return -1;
    }
    int i=0;
    char* buffer = (char*) fat;
    while (fat[start_block] != FAT_EOC ) { // Erases block per block over the chain
        if(fat[start_block] == FAT_RSVD){
            fflush(stdout);
            fprintf(stderr, "Block Reserved\n");
            return -1;
        }
        fat_entry_t next_block = fat[start_block];
        fat[start_block] = FAT_FREE;
        int offset = BLOCK_SIZE * start_block;
        memset(buffer+offset, 0, BLOCK_SIZE); // Wipes the data in the blocks
        start_block = next_block;
        i++;
    }
    if(fat[start_block] == FAT_RSVD || fat[start_block] == FAT_FREE){
            fflush(stdout);
            fprintf(stderr, "Block Reserved or free\n");
            return -1;
    }
    fat[start_block] = FAT_FREE; // Frees the last block
    i++;
    return i;
}

// Helper functions

int getIndex(FileEntry file){
    if(file==NULL){
        fflush(stdout);
        fprintf(stderr, "File is NULL\n");
        return -1;
    }
    return file->file_index;
}

FileEntry getEntry(unsigned int index, fat_entry_t start_block, char* buffer){
    if(index<0 || index>=BLOCKS_AVAILABLE){
        fflush(stdout);
        fprintf(stderr, "Index out of range\n");
        return NULL;
    }
    if(start_block < (fat_entry_t)0 || start_block >= (fat_entry_t)BLOCKS_NUM){
        fflush(stdout);
        fprintf(stderr, "Start block invalid\n");
        return NULL;
    }
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "buffer is NULL\n");
        return NULL;
    }
    return (FileEntry) (buffer+(start_block*BLOCK_SIZE)+(FILE_ENTRY_SIZE*index));
}

int findFreeEntry(char* buffer){ // Returns offset of the first available entry in the current directory
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    for(int i=0; i<FILE_ENTRY_NUM; i++){ // Searches for a free entry in a block
        FileEntry file = getEntry(i, current_directory->start_block, buffer);
        if(!file->is_used) return i;
    }
    fflush(stdout);
    fprintf(stderr, "Free entry not available\n");
    return -1;
}

int find(const char* name, char* buffer, int is_directory){
    if (buffer == NULL) {
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if (name == NULL) {
        fflush(stdout);
        fprintf(stderr, "Name is NULL\n");
        return -1;
    }
    for(int i=0;i<FILE_ENTRY_NUM;i++){ // Loops on all the Entries of the current directory
        FileEntry file = getEntry(i, current_directory->start_block, buffer);
        if(!file->is_used) continue;
        if(!strcmp(file->name, name)){
            if(DEBUG){
                printf("File found at entry %d\n", i);
            }
            return i;
        }
    }
    if(DEBUG){
        printf("File/Dir not found\n");
    }
    return -1;
}

// File functions

int createFile(const char* name, char* buffer) {
    if(name==NULL){
        fflush(stdout);
        fprintf(stderr, "Name is NULL\n");
        return -1;
    }
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    int res = find(name, buffer, 1);

    if(res!=-1){
        fflush(stdout);
        fprintf(stderr, "File already exists\n");
        return -1;
    }
    int index = findFreeEntry(buffer);
    if(index == -1) return -1;
    if(DEBUG){
        printf("Found available entry at position %d\n", index);
    }
    FileEntry file = getEntry(index, current_directory->start_block, buffer);
    if(strlen(name)>48){
        fflush(stdout);
        fprintf(stderr, "Name is too long\n");
        return -1;
    }

    // Assign a free block on file creation
    FATEntry fat = (FATEntry)buffer;
    fat_entry_t start_block = find_free_block(fat);
    if (start_block == -1) {
        fflush(stdout);
        fprintf(stderr, "No free blocks available for file creation\n");
        return -1;
    }

    strcpy(file->name, name);
    file->is_directory=0;
    file->is_used=1;
    file->parent_dir=current_directory;
    file->size=0;
    file->start_block=start_block;
    file->file_index=index;
    fat[start_block]=FAT_EOC;
    current_directory->size+=FILE_ENTRY_SIZE;

    if(DEBUG){
        printf("File %s created at index: %d, with start block: %hd\n", name, index, start_block);
    }
    return index;
}

int eraseFile(FileEntry file, char* buffer) {
    if(buffer == NULL) {
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(file==NULL){
        fflush(stdout);
        fprintf(stderr, "File is NULL\n");
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    fat_entry_t start_block = file->start_block;
    int erased = erase_chain(fat, start_block);
    if(DEBUG){
        printf("Erased %d blocks\n", erased);
    }

    // Clears file's attributes
    memset(file->name, 0, 48);
    file->start_block=-1;
    file->size=0;
    file->file_index=-1;
    file->parent_dir=NULL;
    file->is_used=0;
    return 0;
}

FileHandleEntry openFile(int file_index, char* buffer) {
    if (buffer == NULL) {
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return NULL;
    }
    if (file_index<0 || file_index>=FILE_ENTRY_NUM) {
        fflush(stdout);
        fprintf(stderr, "File index out of Bounds\n");
        return NULL;
    }
    FileEntry file = getEntry(file_index, current_directory->start_block, buffer);
    for(int i=0; i<MAX_OPENED_FILE; i++){ // Searches for the first table entry available, and populates it
        if(!FileHandleTable[i].is_used){
            FileHandleTable[i].file=file;
            FileHandleTable[i].position=0;
            FileHandleTable[i].is_used=1;
            return &FileHandleTable[i];
        }
    }
    fflush(stdout);
    fprintf(stderr, "Max number of Files opened\n");
    return NULL;
}

int closeFile(FileHandleEntry handle) {
    if (handle == NULL) {
        fflush(stdout);
        fprintf(stderr, "Handle is NULL\n");
        return -1;
    }
    if(!handle->is_used){
        fflush(stdout);
        fprintf(stderr, "Handle not in use\n");
        return -1;
    }
    // Clears file handle attributes
    handle->file = NULL;
    handle->position = 0;
    handle->is_used = 0;
    return 0;
}

// Data functions

int fs_write(FileHandleEntry handle, char* buffer, const void* data, size_t size){
    if(handle == NULL || data == NULL) {
        fflush(stdout);
        fprintf(stderr, "Handle or data is NULL\n");
        return -1;
    }
    if(handle->file == NULL) {
        fflush(stdout);
        fprintf(stderr, "File is NULL\n");
        return -1; 
    }
    if(buffer == NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(size <= 0){
        fflush(stdout);
        fprintf(stderr, "Size error\n");
        return -1;
    }
    if(handle->position < 0 || handle->position > BLOCK_SIZE*BLOCKS_NUM) {
        fflush(stdout);
        fprintf(stderr, "Cursor out of bounds\n");
        return -1;
    }
    FileEntry file = handle->file;
    FATEntry fat = (FATEntry) buffer;
    int current_block = handle->position/BLOCK_SIZE; // Which block the handle is currently in
    fat_entry_t next_block;
    fat_entry_t start_block=file->start_block;
    for(int i=0; i<current_block; i++){ // Navigate the fat chain to the block where the handle is in
        next_block = fat[start_block];
        start_block=next_block;
    }
    unsigned int offset = start_block*BLOCK_SIZE;
    if(handle->position+size < BLOCK_SIZE){ // Data fits in the first block
        memcpy(buffer + offset + (handle->position%BLOCK_SIZE), data, size);
        handle->position += size;
        file->size += size;
        return size;
    }else{ // Data doesn't fit in the first block
        int blocks_needed = (handle->position+size)/BLOCK_SIZE;
        if(extend_chain(fat, file->start_block, blocks_needed) == -1){
            return -1;
        }
        int wrote=0;
        memcpy(buffer + offset + (handle->position%BLOCK_SIZE), data, BLOCK_SIZE-handle->position);
        wrote+=BLOCK_SIZE-(handle->position%BLOCK_SIZE);
        handle->position+=wrote;
        fat_entry_t next_block = fat[start_block];
        for(int i=0; i<blocks_needed; i++){
            offset = next_block*BLOCK_SIZE;
            if(size-wrote>=BLOCK_SIZE){ // There's more than BLOCK_SIZE bytes to write still, so it can copy an entire block worth of data
                memcpy(buffer + offset, data+wrote, BLOCK_SIZE);
                wrote+=BLOCK_SIZE;
                handle->position+=BLOCK_SIZE;
            }else{
                memcpy(buffer + offset + (handle->position%BLOCK_SIZE), data+wrote, size-wrote);
                handle->position+=size-wrote;
                wrote=size;
            }
            next_block = fat[next_block];
            if(next_block == FAT_RSVD){
                fflush(stdout);
                fprintf(stderr, "Block reserved\n");
                return -1;
            }
        }
        file->size += size;
        return wrote;
    }
}

int fs_read(FileHandleEntry handle, void* dest, char* buffer, size_t size){
    if(handle == NULL){
        fflush(stdout);
        fprintf(stderr, "Handle is NULL\n");
        return -1;
    }
    if(buffer == NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(size < 0 || size >= handle->file->size){
        fflush(stdout);
        fprintf(stderr, "Size out of bounds\n");
        return -1;
    }
    FATEntry fat = (FATEntry) buffer;
    FileEntry file = handle->file;   
    if(size>file->size-handle->position){
        fflush(stdout);
        fprintf(stderr, "Not enough data in File\n");
        return -1;
    }
    fat_entry_t current_block = handle->position/BLOCK_SIZE;
    fat_entry_t start_block = file->start_block;
    for(int i=0; i<current_block; i++){
        start_block=fat[start_block];
    }
    unsigned int offset = start_block*BLOCK_SIZE;
    int read=0;
    int relative_position = handle->position%BLOCK_SIZE;
    if(size+relative_position<BLOCK_SIZE){ // First block is enough to read size bytes
        memcpy(dest, buffer+offset+relative_position, size);
        read+=size;
        handle->position+=read;
        return read;
    }else{ // Multiple blocks needed to read size bytes
        memcpy(dest, buffer+offset+relative_position, BLOCK_SIZE-relative_position);
        read+=BLOCK_SIZE-relative_position;
        handle->position+=read;
        start_block=fat[start_block];
        offset=start_block*BLOCK_SIZE;
        while(size-read>BLOCK_SIZE){
            memcpy(dest+read, buffer+offset, BLOCK_SIZE);
            read+=BLOCK_SIZE;
            handle->position=read;
            start_block=fat[start_block];
            offset=start_block*BLOCK_SIZE;
        }
        memcpy(dest+read, buffer+offset, size-read); // Last block needed
        handle->position+=size-read;
        read=size;
        return read;
    }
}

int seek(FileHandleEntry handle, char* buffer, unsigned int position){
    if(handle == NULL){
        fflush(stdout);
        fprintf(stderr, "Handle is NULL\n");
        return -1;
    }
    if(buffer == NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(position < 0 || position > BLOCK_SIZE*BLOCKS_AVAILABLE){
        fflush(stdout);
        fprintf(stderr, "Position out of bounds\n");
        return -1;
    }
    handle->position=position;
    return 0;
}

// Directory functions

int createDir(const char* name, char* buffer){
    if(name==NULL){
        fflush(stdout);
        fprintf(stderr, "Name is NULL\n");
        return -1;
    }
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    int res = find(name, buffer, 1);

    if(res!=-1){
        fflush(stdout);
        fprintf(stderr, "Dir already exists\n");
        return -1;
    }
    int index = findFreeEntry(buffer);
    if(index == -1) return -1;
    if(DEBUG){
        printf("Found available entry at position %d\n", index);
    }
    FileEntry file = getEntry(index, current_directory->start_block, buffer);
    if(strlen(name)>48){
        fflush(stdout);
        fprintf(stderr, "Name is too long\n");
        return -1;
    }

    // Assign a free block on dir creation
    FATEntry fat = (FATEntry)buffer;
    fat_entry_t start_block = find_free_block(fat);
    if (start_block == -1) {
        fflush(stdout);
        fprintf(stderr, "No free blocks available for file creation\n");
        return -1;
    }

    strcpy(file->name, name);
    file->is_directory=1;
    file->is_used=1;
    file->parent_dir=current_directory;
    file->size=0;
    file->start_block=start_block;
    file->file_index=index;
    fat[start_block]=FAT_EOC;
    current_directory->size+=FILE_ENTRY_SIZE;
    for(int i=0; i<FILE_ENTRY_NUM; i++){ // Initialize all the dir entries
        FileEntry dir = getEntry(i, start_block, buffer);
        strcpy(dir->name,"\0");
        dir->start_block=-1;
        dir->size=0;
        dir->file_index=0;
        dir->parent_dir=file;
        dir->is_used=0;
        dir->is_directory=0;
    }
    return 0;
}

int eraseDir(const char* name, char* buffer){
    if(name==NULL){
        fflush(stdout);
        fprintf(stderr, "Name is NULL\n");
        return -1;
    }
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    int res = find(name, buffer, 1);
    if(res==-1){
        return -1;
    }
    FileEntry dir = getEntry(res, current_directory->start_block, buffer);
    res = eraseDir_helper(dir, buffer);
    if(res==-1){
        fflush(stdout);
        fprintf(stderr, "Erase dir error\n");
        return -1;
    }
    if(DEBUG){
        printf("Removed directory\n");
    }
    return 0;
}

int eraseDir_helper(FileEntry dir, char* buffer){ // Recursively delete all files and sub directories
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(dir==NULL){
        fflush(stdout);
        fprintf(stderr, "Dir is NULL\n");
        return -1;
    }
    if(!dir->is_used){
        fflush(stdout);
        fprintf(stderr, "Dir not used\n");
        return -1;
    }
    int res;
    for(int i=0; i<FILE_ENTRY_NUM; i++){
        FileEntry entry = getEntry(i, dir->start_block, buffer);
        if(!entry->is_used) continue;
        if(!entry->is_directory){
            res = eraseFile(entry, buffer);
        }else{
            res = eraseDir_helper(entry, buffer);
        }
        if(res==-1) return -1;
    }
    erase_chain((FATEntry) buffer, dir->start_block);
    memset(dir->name, 0, 48);
    dir->start_block=-1;
    dir->size=0;
    dir->file_index=-1;
    dir->parent_dir=NULL;
    dir->is_used=0;
    return 0;
}

int changeDir(const char* name, char* buffer){
    if(name==NULL){
        fflush(stdout);
        fprintf(stderr, "Name is NULL\n");
        return -1;
    }
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(!strcmp(name, ".\0")){
        return 0;
    }
    if(!strcmp(name, "..\0")){
        if(current_directory==ROOT_DIR) return 0;
        current_directory=current_directory->parent_dir;
        return 0;
    }
    int dir = find(name, buffer, 1);
    if(dir==-1){
        return -1;
    }
    current_directory=getEntry(dir, current_directory->start_block, buffer);
    return 0;
}

// Printing functions

void printFAT(char* buffer) {
    if (buffer == NULL) {
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return;
    }
    FATEntry fat = (FATEntry) buffer;
    printf("-----------------\nFAT Entries:\n");
    for (int i = 0; i < BLOCKS_NUM; i++) {
        printf("[%d]: %d\n", i, fat[i]);
    }
    printf("-----------------\n");
}

void printFile(int file_index, char* buffer){
    if (file_index<0 || file_index>=BLOCKS_AVAILABLE) {
        fflush(stdout);
        fprintf(stderr, "File index out of bound\n");
        return;
    }
    FileEntry file = getEntry(file_index, current_directory->start_block, buffer);
    printf("-----------------\n");
    printf("File Name: %s\n", file->name);
    printf("Start Block: %hd\n", file->start_block);
    printf("Size: %u bytes\n", file->size);
    printf("Is Directory: %s\n", file->is_directory ? "Yes" : "No");
    printf("Is used: %s\n", file->is_used ? "Yes" : "No");
    printf("File index: %d\n", file->file_index);
    printf("-----------------\n");
}

void printEntries(char* buffer){
    if(buffer == NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return;
    }
    printf("-----------------\nEntry List:\n");
    for(int i=0; i<FILE_ENTRY_NUM; i++){
        FileEntry file = getEntry(i, current_directory->start_block, buffer);
        if(file->is_used){
            printf("[%d]:\n", i);
            printFile(i, buffer);
        }
    }
    printf("-----------------\n");
}

void printFileHandleTable(){
    printf("-----------------\n");
    printf("File Handle Table:\n");
    for(int i=0; i<MAX_OPENED_FILE; i++){
        printf("File name: %s\n", FileHandleTable[i].file->name);
        printf("Position: %d\n", FileHandleTable[i].position);
        printf("Is used: %s\n", FileHandleTable[i].is_used ? "Yes" : "No");
    }
    printf("-----------------\n");
}

int listFile(char* buffer){
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    for(int i=0; i<FILE_ENTRY_NUM; i++){
        FileEntry file = getEntry(i, current_directory->start_block, buffer);
        if(file->is_used && !file->is_directory){
            printf("%s\n", file->name);
        }
    }
}

int listDir(char* buffer){
    if(buffer==NULL){
        fflush(stdout);
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    printf("./\n");
    if(current_directory!=ROOT_DIR){
        printf("../\n");
    }
    for(int i=0; i<FILE_ENTRY_NUM; i++){
        FileEntry file = getEntry(i, current_directory->start_block, buffer);
        if(file->is_used && file->is_directory){
            printf("%s\n", file->name);
        }
    }
}

