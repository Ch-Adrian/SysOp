#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "source/memory_block_operations.h"

const char commandCreate[13] = "create_table";
const char commandWC[9] = "wc_files";
const char commandRemove[13] = "remove_block";

typedef struct Array_of_tables{
    Tab** the_array_of_tables;
    int array_size;
    int tabWsk;
    int tabSize;
} ATables;

ATables* initATables();
void report_error(char* mess, char* mess2);
void case_create(char** argv, ATables* a_tables);
short case_wc();

int main(int argc, char** argv){

    ATables* a_tables = initATables();

    for(int i = 1; i<argc; i+=2){
        // printf("%s\n", argv[i]);
        if(!strcmp(commandCreate, argv[i])){
            if(case_create(argv, a_tables) == EXIT_ERROR) break;
        }
        else if(!strcmp(commandWC, argv[i])){
            int j = i+1;
            int jBeg = i+1;
            while(j<argc && !(!strcmp(commandCreate, argv[j]) || 
                !strcmp(commandRemove, argv[j]) || 
                !strcmp(commandWC, argv[j]))){
                j++;

            }
            j-=1;
            char** files = calloc(j-jBeg+1, sizeof(char*));
            for(int k = jBeg; k <= j; k++){
                files[k-jBeg] = argv[k];
            }

            char* tmp_file = exec_cmd_wc_on_files(j-jBeg+1, files);
            if(tmp_file != NULL){
                short e_status = create_block_from_file(tmp_file, the_array_of_tables[tabWsk]);
                if(e_status == EXIT_ERROR){
                    report_error("","");
                }
            }

            free_mem(files);
            i = j-1;
        }
        else if(!strcmp(commandRemove, argv[i])){
            int number = atoi(argv[i+1]);
            if(number < 0 || number >= tabSize){
                report_error("Invalid size in command: ", argv[i]);
                break;
            }
            short e_status = remove_block_from_table(number, the_array_of_tables[tabWsk]);
            if(e_status == EXIT_ERROR){
                report_error("","");
                exit(0);
            }
        }
        else{
            report_error("Invalid command: ", argv[i]);
            exit(0);
        }
    }

    deallocateATables(a_tables);

    return 0;
}

ATables* initATables(){
    ATables*t = calloc(1, sizeof(ATable)); 
    t->array_size = 100;
    t->the_array_of_tables = calloc(array_size, sizeof(Tab*));
    t->tabWsk = 0;
    t->tabSize = 20;
    t->the_array_of_tables[tabWsk] = create_table(tabSize);
    return t;
}

void deallocateATables(ATables* t){

    for(int i = 0; i<t->array_size; i++){
        if(t->the_array_of_tables[i] != NULL){
            deallocate_table(t->the_array_of_tables[i]);
        }
    }
    free_mem(t->the_array_of_tables);
    free_mem(t);
}

void report_error(char* mess, char* mess2){
    char message[200] = "ERROR: ";
    strcat(message, mess);
    strcat(message, mess2);
    strcat(message, "\n");
    printf(message);
}

short case_create(char** argv, ATables* a_tables){
    if(a_tables->tabWsk == 99){
        report_error("No space for next table","");
        return EXIT_ERROR;
    }
    else{
        a_tables->tabWsk++;
        a_tables->tabSize = atoi(argv[i+1]);
        a_tables->the_array_of_tables[a_tables->tabWsk] = create_table(a_tables->tabSize);
    }
    return EXIT_OK;
}

short case_wc(){

}