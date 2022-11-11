#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void check_arguments(int, char**);

int main(int argc, char** argv){

    check_arguments(argc, argv);

    if(argc == 2){
        char *buffer = calloc(1024, sizeof(char));
        if(strcmp(argv[1], "data") == 0){
            strcpy(buffer, "echo q | mail | tail -n +3 | head -n -1");
            FILE *f = popen(buffer, "w");
            fputs("exit", f);
            pclose(f);
        }
        else{
            strcpy(buffer, "echo q | mail | tail -n +3 | head -n -1 | sort -k3");
            FILE *f = popen(buffer, "w");
            fputs("exit", f);
            pclose(f);
        }
        free(buffer);
    }
    else if(argc == 4){
        char *buffer = calloc(1024, sizeof(char));
        strcpy(buffer, "mail -s ");
        strcat(buffer, argv[2]);
        strcat(buffer, " ");
        strcat(buffer, argv[1]);

        FILE *f = popen(buffer, "w");
        fputs(argv[3], f);
        pclose(f);

        free(buffer);
    }

    return 0;
}

void check_arguments(int argc, char** argv){
    if(argc == 2){
        if(!(strcmp(argv[1], "data") == 0 || strcmp(argv[1], "nadawca") == 0)){
            printf("Error: First argument is invalid: should be data or nadawca.\n");
            exit(0);
        }
    }
    else if(argc == 4){
        if(strchr(argv[1], '@') == NULL){
            printf("Error: First argument should be an email.\n");
            exit(0);
        }
    }
    else{
        printf("Error: Invalid amount of arguments.\n");
        exit(0);
    }
}