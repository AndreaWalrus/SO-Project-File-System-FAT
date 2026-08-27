#include "FAT_structures.h"


FATEntry init_fat(char* buffer){
    if (buffer == NULL) {
        fprintf(stderr, "Buffer is NULL\n");
        return NULL;
    }
    FATEntry fat = (FATEntry)buffer;
    for (int i = 0; i < FAT_SIZE+FILE_ENTRY_BLOCKS; i++) {
        fat[i] = FAT_RSVD; // Set the first blocks of the FAT to reserved for the FAT itself and the File Entries
    }
    for (int i = FAT_SIZE+FILE_ENTRY_BLOCKS; i < BLOCKS_NUM; i++) {
        fat[i] = FAT_FREE; // Set all the remaining blocks as free
    }
    for(int i = 0; i<BLOCKS_NUM; i++){
        FileEntry file = (FileEntry) (buffer+(FAT_SIZE*BLOCK_SIZE)+(i*64));
        file->is_used=0; // Inizialize all the file entries as unused
    }
    return fat;  
}

fat_entry_t find_free_block(FATEntry fat) {
    for (int i = 0; i < BLOCKS_NUM; i++) {
        if (fat[i] == FAT_FREE) {
            return i;
        }
    }
    fprintf(stderr, "No free blocks available\n");
    return -1;
}


fat_entry_t allocate_block(FATEntry fat, fat_entry_t start_block) {
    if(fat == NULL) {
        fprintf(stderr, "FAT is NULL\n");
        return -1;
    }
    if(start_block < (fat_entry_t)0 || start_block >= (fat_entry_t)BLOCKS_NUM){
        fprintf(stderr, "Start block invalid\n");
        return -1;
    }
    fat_entry_t next_block = find_free_block(fat);
    if (next_block == -1) {
        fprintf(stderr, "No free blocks available for allocation\n");
        return -1;
    }
    fat[start_block] = next_block;
    fat[next_block] = FAT_EOC;
    return next_block;
}

fat_entry_t extend_chain(FATEntry fat, fat_entry_t start_block, unsigned int block_num) {
    if(fat == NULL) {
        fprintf(stderr, "FAT is NULL\n");
        return -1;
    }
    while(fat[start_block] != FAT_EOC) {
        if(fat[start_block] == FAT_RSVD || fat[start_block] == FAT_FREE) {
            fprintf(stderr, "Invalid block in chain\n");
            return -1;
        }
        start_block = fat[start_block];
    }
    fat_entry_t next_block;
    for(int i=0; i<block_num; i++){
        next_block = allocate_block(fat, start_block);
        start_block=next_block;
    }
    return next_block;
}

fat_entry_t free_block(FATEntry fat, fat_entry_t block_index) {
    if(fat == NULL){
        fprintf(stderr, "FAT is NULL\n");
        return -1;
    }
    if (block_index < (fat_entry_t)0 || block_index >= (fat_entry_t)BLOCKS_NUM || fat[block_index] == FAT_FREE || fat[block_index] == FAT_RSVD) {
        fprintf(stderr, "Invalid block index or block already free/reserved\n");
        return -1; 
    }
    fat_entry_t next_block = fat[block_index];
    fat[block_index] = FAT_FREE;
    return next_block;
}

int erase_chain(FATEntry fat, fat_entry_t start_block) {
    if (start_block < 0 || start_block >= BLOCKS_NUM) {
        fprintf(stderr, "Invalid start block index\n");
        return -1;
    }
    if(fat == NULL) {
        fprintf(stderr, "FAT is NULL\n");
        return -1;
    }
    int i=0;
    while (fat[start_block] != FAT_EOC ) {
        if(fat[start_block] == FAT_RSVD)
            fprintf(stderr, "Block Reserved\n");
            return -1;
        fat_entry_t next_block = fat[start_block];
        fat[start_block] = FAT_FREE;
        start_block = next_block;
        i++;
    }
    if(fat[start_block] == FAT_RSVD || fat[start_block] == FAT_FREE){
            fprintf(stderr, "Block Reserved or free\n");
            return -1;
    }
    fat[start_block] = FAT_FREE;
    i++;
    return i;
}

int createFile(const char* name, char* buffer) {
    if (buffer == NULL) {
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    fat_entry_t start_block = find_free_block(fat);
    if (start_block == -1) {
        fprintf(stderr, "No free blocks available for file creation\n");
        return -1;
    }

    FileEntry file;
    int pos;
    // Finds the first available FileEntry in buffer
    for(int i = 0; i<BLOCKS_NUM; i++){
        file = (FileEntry) (buffer+getOffset(i));
        if(file->is_used==0){
            //printf("Found available entry at position %d\n", i);
            pos=i;
            break;
        }
    }

    memcpy(file->name, name, 48);
    file->start_block = start_block;
    file->size = 0;
    file->is_directory = 0;
    file->is_used = 1;
    file->file_index = pos;

    fat[start_block] = FAT_EOC;
    printf("File 'pippo' created at block: %hd, index: %d\n", start_block, pos);
    return pos;
}

int eraseFile(int file_index, char* buffer) {
    if(buffer == NULL) {
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(file_index<0 || file_index>BLOCKS_NUM){
        fprintf(stderr, "Index out of bound\n");
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    FileEntry file = getFileEntry(file_index, buffer);
    fat_entry_t start_block = file->start_block;
    int erased = erase_chain(fat, start_block);
    printf("Erased %d blocks\n", erased);
    unsigned int offset = (int) start_block * BLOCK_SIZE;
    memset(buffer+offset,0,file->size);
    file->is_used=0;
    file = NULL;
    return 0;
}

int getOffset(unsigned int file_index){
    return (FAT_SIZE*BLOCK_SIZE)+(file_index*FILE_ENTRY_SIZE);
}

int getIndex(FileEntry file){
    if(file==NULL){
        fprintf(stderr, "File is NULL\n");
        return -1;
    }
    return file->file_index;
}

FileEntry getFileEntry(unsigned int file_index, char* buffer){
    if(file_index>BLOCKS_NUM){
        fprintf(stderr, "Index out of range\n");
        return NULL;
    }
    if(buffer==NULL){
        fprintf(stderr, "buffer is NULL\n");
        return NULL;
    }
    return (FileEntry) (buffer+getOffset(file_index));
}

FileHandleEntry openFile(FileEntry file) {
    if (file == NULL) {
        fprintf(stderr, "File is NULL\n");
        return NULL;
    }
    for(int i=0; i<MAX_OPENED_FILE; i++){
        if(!FileHandleTable[i].is_used){
            FileHandleTable[i].file_index=getIndex(file);
            FileHandleTable[i].position=0;
            FileHandleTable[i].is_used=1;
            return &FileHandleTable[i];
        }
    }
    fprintf(stderr, "Max number of Files opened\n");
    return NULL;
}

int closeFile(FileHandleEntry handle) {
    if (handle == NULL) {
        fprintf(stderr, "Handle is NULL\n");
        return -1;
    }
    if(!handle->is_used){
        fprintf(stderr, "Handle not in use\n");
        return -1;
    }
    handle->file_index = -1;
    handle->position = 0;
    handle->is_used = 0;
    return 0;
}

int findFile(const char* name, char* buffer){
    if (buffer == NULL) {
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if (name == NULL) {
        fprintf(stderr, "Name is NULL\n");
        return -1;
    }
    for(int i=0;i<BLOCKS_NUM;i++){
        FileEntry file = (FileEntry) (buffer+getOffset(i));
        if(strcmp(file->name, name)==0){
            printf("File found at entry %d\n", i);
            return i;
        }
    }
    printf("File not found\n");
    return -1;
}

int write(FileHandleEntry handle, char* buffer, const void* data, size_t size){
    if(handle == NULL || data == NULL) {
        fprintf(stderr, "Handle or data is NULL\n");
        return -1;
    }
    if(handle->file_index < 0 || handle->file_index >= BLOCKS_NUM) {
        fprintf(stderr, "File index out of bound\n");
        return -1; 
    }
    if(buffer == NULL){
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(size <= 0){
        fprintf(stderr, "Size error\n");
        return -1;
    }
    if(handle->position < 0 || handle->position > BLOCK_SIZE*BLOCKS_NUM) {
        fprintf(stderr, "Cursor out of bounds\n");
        return -1;
    }
    FileEntry file = getFileEntry(handle->file_index, buffer);
    FATEntry fat = (FATEntry) buffer;
    int current_block = handle->position/BLOCK_SIZE;
    fat_entry_t next_block;
    fat_entry_t start_block=file->start_block;
    for(int i=0; i<current_block; i++){
        next_block = fat[start_block];
        start_block=next_block;
    }
    unsigned int offset = start_block*BLOCK_SIZE;
    if(handle->position+size < BLOCK_SIZE){ // Data fits in the first block
        memcpy(buffer + offset + (handle->position%BLOCK_SIZE), data, size);
        handle->position += size;
        file->size += size;
        return size;
    }else{
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
            if(size-wrote>=BLOCK_SIZE){
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
                fprintf(stderr, "Block reserved\n");
                return -1;
            }
        }
        return wrote;
    }
}

// Testing functions

void printFAT(FATEntry fat) {
    if (fat == NULL) {
        fprintf(stderr, "FAT is NULL\n");
        return;
    }
    printf("-----------------\nFAT Entries:\n");
    for (int i = 0; i < BLOCKS_NUM; i++) {
        printf("[%d]: %d\n", i, fat[i]);
    }
    printf("-----------------\n");
}

void printFile(FileEntry file){
    if (file == NULL) {
        fprintf(stderr, "File is NULL\n");
        return;
    }
    printf("File Name: %s\n", file->name);
    printf("Start Block: %hd\n", file->start_block);
    printf("Size: %u bytes\n", file->size);
    printf("Is Directory: %s\n", file->is_directory ? "Yes" : "No");
    printf("Is used: %s\n", file->is_used ? "Yes" : "No");
    printf("File index: %d\n", file->file_index);
}

void printFileEntryList(char* buffer){
    if(buffer == NULL){
        fprintf(stderr, "Buffer is NULL\n");
        return;
    }
    printf("-----------------\nFile Entry List:\n");
    for(int i=0; i<BLOCKS_NUM; i++){
        FileEntry file = (FileEntry) (buffer+getOffset(i));
        if(file->is_used){
            printf("[%d]:\n", i);
            printFile(getFileEntry(i, buffer));
        }
    }
    printf("-----------------\n");
}

void printFileHandleTable(){
    printf("File Handle Table:\n");
    for(int i=0; i<MAX_OPENED_FILE; i++){
        printf("File index: %d\n", FileHandleTable[i].file_index);
        printf("Position: %d\n", FileHandleTable[i].position);
        printf("Is used: %s\n", FileHandleTable[i].is_used ? "Yes" : "No");
    }
}