#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include<sys/wait.h>

void argument_validation(int argc, char** argv);

void signal_received(int signum){
    printf("Signal SIGUSR1 has been received.\n");
}

int main(int argc, char** argv){

    argument_validation(argc, argv);

    if (strcmp(argv[1], "ignore") == 0){
        if(signal(SIGUSR1, SIG_IGN) == SIG_ERR){
            printf("Error: Signal error.\n");
            exit(0);
        }
    }
    else if(strcmp(argv[1], "handler") == 0){
        if(signal(SIGUSR1, signal_received) == SIG_ERR){
            printf("Error: Signal error.\n");
            exit(0);
        }
    }
    else if((strcmp(argv[1], "mask") == 0) ||(strcmp(argv[1], "pending") == 0)) {
        sigset_t new_mask;
        sigset_t old_mask;

        sigemptyset(&new_mask);
        sigaddset(&new_mask, SIGUSR1);
        if(sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0){
            printf("Error: Sigprocmask error.\n");
            exit(0);
        }
    }

    pid_t p = fork();
    if(p == 0){
        if(strcmp(argv[1], "pending") != 0){
            raise(SIGUSR1);
            sigset_t mask;
            if(sigpending(&mask) < 0){
                printf("Error: Sigpending error.\n");
                exit(0);
            }
            if (sigismember(&mask, SIGUSR1)) {
                printf("Pending SIGUSR1 in subprocess of zad1.\n");
            }
        }
        else{
            sigset_t mask;
            if(sigpending(&mask) < 0){
                printf("Error: Sigpending error.\n");
                exit(0);
            }
            if (sigismember(&mask, SIGUSR1)) {
                printf("Subprocess of zad1 see pending SIGUSR1 in zad1.\n");
            }
        }
        return 0;
    }
    else{

        raise(SIGUSR1);
        sigset_t mask;
        if(sigpending(&mask) < 0){
            printf("Error: Sigpending error.\n");
            exit(0);
        }
        if (sigismember(&mask, SIGUSR1)) {
            printf("Pending SIGUSR1 in zad1\n");
        }
        if(strcmp(argv[1], "handler") != 0) {
            if (execv("./zad1e", argv) == -1) {
                printf("Error: In function execv\n");
                exit(0);
            }
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