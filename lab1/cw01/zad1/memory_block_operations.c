#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "memory_block_operations.h"

Tab* create_table(int amt_of_blocks){
    Tab* t = calloc(1, sizeof(Tab));
    t->blocks = calloc(amt_of_blocks, sizeof(MBlock*));
    t->size = amt_of_blocks;
    return t;
}

char* exec_cmd_wc_on_files(int amt, char** files){
    if(amt <= 0 ) return EXIT_ERROR;

    if(system("touch temporary.txt") == -1) return EXIT_ERROR;

    int cnt = 0;
    for(int i = 0; i<amt; i++){
        FILE* file;
        if((file = fopen(files[i], "r"))){
            fclose(file);
        }
        else {
            cnt++;
            continue;
        }

        char* command = calloc(300, sizeof(char));
        strcat(command, "wc ");
        char file_name[100];
        strcpy(file_name, files[i]);
        strcat(command, file_name);
        strcat(command, " | grep -wo '[0-9]*' >> temporary.txt");
        if(system(command) == -1) return EXIT_ERROR;
        free(command);
    }
    if(cnt == amt) return NULL;
    return "temporary.txt";
}

short create_block_from_file(char* temporary_file, Tab* tab){

    if(tab == NULL) 
        return EXIT_ERROR;

    if(temporary_file == NULL)
        return EXIT_ERROR;

    int size = 0;
    FILE*f;
    if((f = fopen(temporary_file, "r"))){
        fseek(f, 0L, SEEK_END);
        size = ftell(f);
        fclose(f);
    } 
    else return EXIT_ERROR;

    
    for(int i = 0; i<tab->size; i++){
        if(tab->blocks[i] == NULL){
            create_block_b(size, i, tab);
            
            int fd = open(temporary_file, O_RDONLY);
            if(fd == -1) return EXIT_ERROR;
            if(read(fd, tab->blocks[i]->results, size) == -1){
                return EXIT_ERROR;
            }

            break;
        }
    }

    if(system("rm temporary.txt") == -1) 
        return EXIT_ERROR;

    return EXIT_OK;
}

short remove_block_from_table(int block_idx, Tab* tab){
    
    if(tab == NULL) 
        return EXIT_NULL;

    if(block_idx >= tab->size || block_idx < 0) 
        return EXIT_OUT_OF_RANGE;

    if((tab->blocks)[block_idx] == NULL) 
        return EXIT_NULL;

    if(tab->blocks[block_idx]->results == NULL)
        return EXIT_NULL;


    free_mem(tab->blocks[block_idx]->results);
    free_mem(tab->blocks[block_idx]);
    return EXIT_OK;
}

short deallocate_table(Tab* tab){

    if(tab == NULL) 
        return EXIT_NULL;

    for(int i = 0; i<tab->size; i++){
        if(remove_block_from_table(i, tab) == EXIT_ERROR){
            return EXIT_ERROR;
        }
    }

    free_mem(tab->blocks);
    free_mem(tab);
    return EXIT_OK;
}

short create_block_b(int amt_of_bytes, int block_idx, Tab* tab){
    
    if(tab == NULL) 
        return EXIT_NULL;
    if(block_idx >= tab->size || block_idx < 0) 
        return EXIT_OUT_OF_RANGE;
    if(tab->blocks[block_idx] != NULL) 
        return EXIT_ERROR;
    
    tab->blocks[block_idx] = calloc(1, sizeof(MBlock));
    tab->blocks[block_idx]->results = malloc(amt_of_bytes);
    tab->blocks[block_idx]->size = amt_of_bytes;

    return EXIT_OK;
}
