#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "memory_block_operations.h"

Tab* create_table(int amt_of_blocks){
    Tab* t = calloc(1, sizeof(Tab));
    t->blocks = calloc(amt_of_blocks, sizeof(MBlock*));
    t->size = amt_of_blocks;
    return t;
}

char* exec_cmd_wc_on_files(int amt, char** files){
    if(amt < 0 ) return EXIT_ERROR;

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
        strcpy(command, "wc ");
        strcpy(command, files[i]);
        strcpy(command, "  | grep -wo '[0-9]*' >> temporary.txt");
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

    FILE*f;
    if((f = fopen(temporary_file, "r"))){
        fclose(f);
    } 
    else return EXIT_ERROR;

    struct stat st;
    stat(temporary_file, &st);
    int size = st.st_size;

    for(int i = 0; i<tab->size; i++){
        if(tab->blocks[i] == NULL){
            create_block_b(size, i, tab);
            
            int fd = open(temporary_file, S_IREAD);
            if(read(fd, tab->blocks[i]->results, size) == -1){
                return EXIT_ERROR;
            }

            // printf("%c\n", tab->blocks[i]->results[2]);

            break;
        }
    }

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

    // for(int i = 0; i<tab->blocks[block_idx]->size; i++){
    //         if(tab->blocks[block_idx]->results[i] != NULL){
    //             if( deallocate_result(i, block_idx, tab) == EXIT_ERROR)
    //                 return EXIT_ERROR;
    //         }
    // }

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

// short create_block_i(int amt_of_results, int block_idx, Tab* tab){
    
//     if(tab == NULL) 
//         return EXIT_NULL;
//     if(block_idx >= tab->size || block_idx < 0) 
//         return EXIT_OUT_OF_RANGE;
//     if(tab->blocks[block_idx] != NULL) 
//         return EXIT_ERROR;
    
//     tab->blocks[block_idx] = calloc(1, sizeof(MBlock));
//     tab->blocks[block_idx]->results = calloc(amt_of_results, sizeof(WCResult*));
//     tab->blocks[block_idx]->size = amt_of_results;

//     return EXIT_OK;
// }

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

// short create_result(int l, int w, int c, int result_idx, int block_idx, Tab* tab){
        
//     if(tab == NULL) 
//         return EXIT_NULL;
//     if(block_idx >= tab->size || block_idx < 0) 
//         return EXIT_OUT_OF_RANGE;
//     if(tab->blocks[block_idx] == NULL) 
//         return EXIT_ERROR;
//     int amt_of_results = tab->blocks[block_idx]->size;
//     if(result_idx >= amt_of_results || result_idx < 0)
//         return EXIT_OUT_OF_RANGE;
    
//     tab->blocks[block_idx]->results[result_idx] = calloc(1, sizeof(WCResult));
//     tab->blocks[block_idx]->results[result_idx]->lines = l;
//     tab->blocks[block_idx]->results[result_idx]->words = w;
//     tab->blocks[block_idx]->results[result_idx]->chars = c;
//     return EXIT_OK;
// }

// short deallocate_result(int result_idx, int block_idx, Tab* tab){
//     if(tab == NULL) 
//         return EXIT_NULL;
//     if(block_idx >= tab->size || block_idx < 0) 
//         return EXIT_OUT_OF_RANGE;
//     if(tab->blocks[block_idx] == NULL) 
//         return EXIT_ERROR;
//     int amt_of_results = tab->blocks[block_idx]->size;
//     if(result_idx >= amt_of_results || result_idx < 0)
//         return EXIT_OUT_OF_RANGE;
    
//     free_mem(tab->blocks[block_idx]->results[result_idx]);
//     return EXIT_OK;
// }
