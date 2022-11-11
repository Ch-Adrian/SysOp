#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

void argument_validation(int argc, char** argv);

int main(int argc, char** argv){

    argument_validation(argc, argv);

        if(strcmp(argv[1], "pending") == 0){
            sigset_t mask;
            sigpending(&mask);
            if (sigismember(&mask, SIGUSR1)) {
                printf("Zad1e see pending SIGUSR1.\n");
            }
        }
        else if(strcmp(argv[1], "handler") != 0){
//            raise(SIGUSR1);
            sigset_t mask;
            sigpending(&mask);
            if (sigismember(&mask, SIGUSR1)) {
                printf("Pending SIGUSR1 in zad1e.\n");
            }
        }

    return 0;
}

void argument_validation(int argc, char** argv){
    if(argc != 2){
        printf("Error: Invalid amount of arguments.\n");
        exit(0);
    }
    if (!( (strcmp(argv[1], "ignore") ==0) || (strcmp(argv[1], "handler") ==0) ||
           (strcmp(argv[1], "mask") ==0) || (strcmp(argv[1], "pending") ==0) ) ) {
        printf("Error: First argument should be: ignore, handler, mask or pending.\n");
        exit(0);
    }
}