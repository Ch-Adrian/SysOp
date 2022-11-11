#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

#define free_mem(ptr) free(ptr); ptr = NULL;

void f_error_invalid_arguments(char* source_file, char* destination_file, int argc, char** argv);
void count_character(char c, char* source_file);
void apply_procedure(int argc, char** argv);
void time_function(void (*fun)(int, char**), int argc, char** argv);

int main(int argc, char** argv){

    time_function(apply_procedure, argc, argv);

    return 0;
}

void apply_procedure(int argc, char** argv){
    char* achar = calloc(1, sizeof(char));
    char* source_file = calloc(256, sizeof(char));

    f_error_invalid_arguments(achar, source_file, argc, argv);
    count_character(*achar, source_file);

    free_mem(achar);
    free_mem(source_file);
}

void f_error_invalid_arguments(char* achar, char* source_file, int argc, char** argv){
    char* string_for_char = calloc(256, sizeof( char));
    if(argc > 3){
        printf("Invalid amount of arguments, should be two: character and source_file.\n");
        exit(0);
    }
    else if(argc == 1){
        printf("Please enter arguments:\n");
        printf("Character: ");
        scanf("%s", string_for_char);
        if(strlen(string_for_char) > 1) {
            printf("Error: Invalid first argument. Should be a character. But is: ");
            printf("%s\n", string_for_char);
            exit(1);
        }
        printf("Source file: ");
        scanf("%s", source_file);
    }
    else if(argc == 2){
        strcpy(string_for_char, argv[1]);
        printf("Please enter source file: \n");
        printf("Source file: ");
        scanf("%s", source_file);
    }
    else if(argc == 3){
        strcpy(string_for_char, argv[1]);
        strcpy(source_file, argv[2]);
    }
    if(strlen(string_for_char) > 1) {
        printf("Error: Invalid first argument. Should be a character. But is: ");
        printf("%s\n", string_for_char);
        exit(1);
    }
    FILE *file;
    if((file = fopen(source_file, "r"))){
        fclose(file);
    }
    else{
        printf("Error: Cannot open file: ");
        printf("%s\n", source_file);
        exit(1);
    }

    *achar = string_for_char[0];
    free_mem(string_for_char);
}

void count_character(char c, char* source_file){

    FILE* fileIn = fopen(source_file, "r");
    if(fileIn == NULL){
        printf("File cannot be opened.");
        exit(1);
    }

    char* line = calloc(257, sizeof (char));
    int amt_of_lines = 0;
    int amt_of_char_per_line = 0;
    int amt_of_char = 0;

    while(!feof(fileIn)){
        int amt = fread(line, sizeof(char), 256, fileIn);
        if(amt == 0) break;
        for(int i = 0; i<amt; i++){

            if(line[i] == '\n'){
                if(amt_of_char_per_line > 0){
                    amt_of_lines++;
                }
                amt_of_char_per_line = 0;
            }
            if(line[i] == c){
                amt_of_char_per_line++;
                amt_of_char++;
            }
        }
    }

    if(amt_of_char_per_line > 0){
        amt_of_lines++;
    }
    amt_of_char_per_line = 0;

    printf("Liczba znaków: %d\nLiczba linii: %d\n", amt_of_char, amt_of_lines);
    free_mem(line);
    fclose(fileIn);
}

void time_function(void (*fun)(int, char**),int argc, char** argv){
    struct tms tms_start, tms_end;
    clock_t clock_start, clock_end;
    if( (clock_start = times(&tms_start)) == -1){
        printf("ERROR: function times");
    }

    fun(argc, argv);

    if( (clock_end = times(&tms_end)) == -1){
        printf("ERROR: function times");
    }

    static long clktck = 0;

    if( (clktck = sysconf(_SC_CLK_TCK)) < 0){
        printf("ERROR: Function sysconf");
        clktck = 1;
    }
    clock_t real = clock_end - clock_start;
    printf("Czas wykonania:\n");
    printf("real time: %7.4f\n", real/(double)clktck);
    printf("user time: %7.4f\n", ((tms_end.tms_utime - tms_start.tms_utime)/(double)clktck));
    printf("system time: %7.4f\n", ((tms_end.tms_stime - tms_start.tms_stime)/(double)clktck));
    printf("\n");


}