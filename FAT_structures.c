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

fat_entry_t extend_chain(FATEntry fat, fat_entry_t start_block) {
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
    fat_entry_t next_block = allocate_block(fat, start_block);
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
        file = (FileEntry) (buffer+(FAT_SIZE*BLOCK_SIZE)+(i*FILE_ENTRY_SIZE));
        if(file->is_used==0){
            printf("Found available entry at position %d\n", i);
            pos=i;
            break;
        }
    }

    memcpy(file->name, name, 48);
    file->start_block = start_block;
    file->size = 0;
    file->is_directory = 0;
    file->is_used = 1;

    fat[start_block] = FAT_EOC;
    return pos;
}

int eraseFile(FileEntry file, char* buffer) {
    if(buffer == NULL || file == NULL) {
        fprintf(stderr, "Buffer or file is NULL\n");
        return -1;
    }
    FATEntry fat = (FATEntry)buffer;
    fat_entry_t start_block = file->start_block;
    int erased = erase_chain(fat, start_block);
    printf("Erased %d blocks\n", erased);
    unsigned int offset = (int) start_block * BLOCK_SIZE;
    memset(buffer+offset,0,file->size);
    file = NULL;
    return 0;
}

FileHandleEntry openFile(FileEntry file) {
    if (file == NULL) {
        fprintf(stderr, "File is NULL\n");
        return NULL;
    }
    FileHandleEntry handle = (FileHandleEntry)malloc(sizeof(struct FileHandle));
    handle->file = file;
    handle->position = sizeof(struct File); // Start after the file metadata
    return handle;
}

int closeFile(FileHandleEntry handle) {
    if (handle == NULL) {
        fprintf(stderr, "Handle is NULL\n");
        return -1;
    }
    free(handle);
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
        FileEntry file = (FileEntry) (buffer+(FAT_SIZE*BLOCK_SIZE)+(i*FILE_ENTRY_SIZE));
        if(strcmp(file->name, name)==0){
            printf("File found at entry %d\n", i);
            return i;
        }
    }
    printf("File not found\n");
    return -1;
}

int write(FileHandleEntry handle, char* buffer, const void* data, size_t size) {
    if(handle == NULL || data == NULL) {
        fprintf(stderr, "Handle or data is NULL\n");
        return -1;
    }
    if(handle->file == NULL) {
        fprintf(stderr, "File in handle is NULL\n");
        return -1; 
    }
    if(buffer == NULL){
        fprintf(stderr, "Buffer is NULL\n");
        return -1;
    }
    if(size <= 0){
        fprintf(stderr, "size error\n");
        return -1;
    }
    unsigned int file_size = handle->file->size - sizeof(struct File); // Bytes used for file data
    if(handle->position < sizeof(struct File)) {
        fprintf(stderr, "Cursor out of bounds\n");
        return -1;
    }
    if(handle->position < BLOCK_SIZE){ // Cursor is within the first block
        if((handle->position%BLOCK_SIZE) + file_size < BLOCK_SIZE){ // Data fits in the first block
            unsigned int offset = (handle->file->start_block * BLOCK_SIZE) + handle->position;
            memcpy(buffer + offset, data, size);
            handle->position += size;
            handle->file->size += size;
            return size;
        }
        if((handle->position%BLOCK_SIZE) + file_size >= BLOCK_SIZE){ // Data does not fit in the first block
            int i=0;
            FATEntry fat = (FATEntry)buffer;
            while((i*BLOCK_SIZE)-handle->position < size){ // Extend the file chain to contain the data
                fat_entry_t next_block = allocate_block(fat, handle->file->start_block);
                if (next_block == -1) {
                    fprintf(stderr, "Failed to allocate new block for writing\n");
                    return -1;
                }
                fat_entry_t new_block = extend_chain(fat, handle->file->start_block);
                i++;
            }
            unsigned int written = 0;
            fat_entry_t current_block = handle->file->start_block;
            while(written < size){
                unsigned int block_offset = (int)(handle->position / BLOCK_SIZE)+1;
                unsigned int to_write = 0;
                if(size-written>BLOCK_SIZE)
                    to_write = (BLOCK_SIZE*block_offset) - handle->position;
                else
                    to_write = size - written;
                unsigned int offset = (current_block * (fat_entry_t)BLOCK_SIZE) + (handle->position%BLOCK_SIZE);
                memcpy(buffer + offset, (char*)data + written, to_write);
                handle->position += to_write;
                written += to_write;
                handle->file->size += to_write;
                current_block = fat[current_block];
                if(current_block == FAT_RSVD) {
                    fprintf(stderr, "Block reserved\n");
                    return -1;
                }
            }

        }
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
    printf("-----------------\nFile Name: %s\n", file->name);
    printf("Start Block: %hd\n", file->start_block);
    printf("Size: %u bytes\n", file->size);
    printf("Is Directory: %s\n", file->is_directory ? "Yes" : "No");
    printf("Is used: %s\n-----------------\n", file->is_used ? "Yes" : "No");
}