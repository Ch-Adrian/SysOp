#include <stdio.h>
#include <stdlib.h>
#include "memory_block_operations.h"

#define AMT_OF_TESTS 100

// void test_log1(char* message, short (*f)(char*)){
//     printf("\t");
//     printf(message);
//     printf(" status: %d\n")
// }

short test1(){
    Tab* t = calloc(1, sizeof(Tab));
    t->blocks = calloc(4, sizeof(MBlock*));
    t->blocks[0] = calloc(1, sizeof(MBlock));
    t->blocks[0]->size = 0;
    // printf("%d\n", t->blocks[0]->size);
    // printf("dziala");
    free(t->blocks[0]);
    free(t->blocks);
    free(t);
    return EXIT_OK;
}

short test2(){
    Tab* t = create_table(4);
    return deallocate_table(t);
}

short test3(){
    Tab* t = create_table(4);

    printf("\tAllocating status: %d\n", create_block_b(4, 0, t));
    printf("\tDeleting non-existing block status: %d\n", remove_block_from_table(1, t));
    printf("\tAllocating block status: %d\n", create_block_b(4, 2, t));
    printf("\tDeleting existing block status: %d\n", remove_block_from_table(0, t));

    return deallocate_table(t);
}

short test4(){
    if(system("touch test_file.txt") == -1)
        return EXIT_ERROR;
    if(system("echo -e ala\\nma\\nkota\\nkot ma ale > test_file.txt") == -1)
        return EXIT_ERROR;
    if(system("touch tmp.txt") == -1)
        return EXIT_ERROR;
    if(system("wc test_file.txt | grep -wo '[0-9]*' > tmp.txt") == -1) 
        return EXIT_ERROR;
    if(system("rm test_file.txt") == -1)
        return EXIT_ERROR;
    if(system("rm tmp.txt") == -1)
        return EXIT_ERROR;

    return EXIT_OK;
}

short test5(){
    Tab* t = create_table(4);

    printf("\tCreating block from file status: %d\n", create_block_from_file("tmp2.txt", t));

    // printf("\tAllocating status: %d\n", create_block(1, 0, t));

    // if(system("touch test_file.txt") == -1)
    //     return EXIT_ERROR;
    // if(system("echo -e ala\\nma\\nkota\\nkot ma ale > test_file.txt") == -1)
    //     return EXIT_ERROR;
    // if(system("touch tmp.txt") == -1)
    //     return EXIT_ERROR;
    // if(system("wc test_file.txt | grep -wo '[0-9]*' > tmp.txt") == -1) 
    //     return EXIT_ERROR;


    // if(system("rm test_file.txt") == -1)
    //     return EXIT_ERROR;
    // if(system("rm tmp.txt") == -1)
    //     return EXIT_ERROR;

    return deallocate_table(t);
}

int main(){

    short (*arr_of_fun[AMT_OF_TESTS])() = {
        test1,
        test2,
        test3,
        test4,
        test5
    };

    for(int i = 0; i<AMT_OF_TESTS; i++){
        if(arr_of_fun[i] != NULL){
            short status = (*arr_of_fun[i])();
            if(status == EXIT_ERROR){
                printf("Status \033[0;31mERROR\033[0m: in test %d\n", i+1);
            }
            else if(status != EXIT_OK){
                printf("Status PROBLEM: in test %d\n", i+1);
            }
            else{
                printf("Status \033[0;32mOK\033[0m: in test %d\n", i+1);
            }
        }
    }

    return 0;
}