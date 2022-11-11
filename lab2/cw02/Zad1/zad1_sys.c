#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/times.h>

#define free_mem(ptr) free(ptr); ptr = NULL;

void f_error_invalid_arguments(char* source_file, char* destination_file, int argc, char** argv);
void copy_file(char* source_file, char* destination_file);
void apply_procedure(int argc, char** argv);
void time_function(void (*fun)(int, char**), int argc, char** argv);

int main(int argc, char** argv){

    time_function(apply_procedure, argc, argv);

    return 0;
}

void apply_procedure(int argc, char** argv){
    char* source_file = calloc(256, sizeof(char));
    char* destination_file = calloc(256, sizeof(char));

    f_error_invalid_arguments(source_file, destination_file, argc, argv);
    copy_file(source_file, destination_file);

    free_mem(source_file);
    free_mem(destination_file);

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
    printf("real time: %7.4f\n", real/(double)clktck);
    printf("user time: %7.4f\n", ((tms_end.tms_utime - tms_start.tms_utime)/(double)clktck));
    printf("system time: %7.4f\n", ((tms_end.tms_stime - tms_start.tms_stime)/(double)clktck));
    printf("\n");


}

void f_error_invalid_arguments(char* source_file, char* destination_file, int argc, char** argv){
    if(argc > 3){
        printf("Invalid amount of arguments, should be two: source_file and destination_file.\n");
        exit(0);
    }
    else if(argc == 1){
        printf("Please enter files:\n");
        printf("Source file: ");
        scanf("%s", source_file);
        printf("Destination file: ");
        scanf("%s", destination_file);
    }
    else if(argc == 2) {
        strcpy(source_file, argv[1]);
        printf("Please enter destination file: \n");
        printf("Destination file: ");
        scanf("%s", destination_file);
    }
    else if(argc == 3){
        strcpy(source_file, argv[1]);
        strcpy(destination_file, argv[2]);
    }
    if(access(source_file, F_OK)!=0){
        printf("Error: Cannot open file: ");
        printf("%s\n", source_file);
        exit(1);
    }
    if(access(destination_file, F_OK)!=0){
        printf("Error: Cannot open file: ");
        printf("%s\n", destination_file);
        exit(1);
    }
}

void copy_file(char* source_file, char* destination_file){

    int fileIn = open(source_file, O_RDONLY );
    if(fileIn == 0){
        printf("File cannot be opened.");
        exit(1);
    }

    int fileOut = open(destination_file, O_WRONLY | O_CREAT);
    if(fileOut == 0){
        printf("File cannot be opened.");
        close(fileIn);
        exit(1);
    }

    char* c = calloc(1, sizeof(char));
    int i = 1;
    while(i != 0){
        i = read(fileIn, c, sizeof(char));
        if(i != 0 && !isspace(*c)){
            write(fileOut, c, sizeof(char));
        }
    }

    free(c);
    close(fileIn);
    close(fileOut);
}
