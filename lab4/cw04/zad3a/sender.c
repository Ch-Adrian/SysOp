#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include<sys/wait.h>


sigset_t new_mask;

int cnt = 0;
int n = 0;
int process_waiting =1;
void check_arguments(int argc, char** argv);

void handler(int signo, siginfo_t* info, void *context);
void set_signals(int sig1,int sig2);

int main(int argc, char** argv){

    check_arguments(argc, argv);

    int p = atoi(argv[1]);
    n = atoi(argv[2]);

    sigset_t block_mask;

    sigfillset(&new_mask);
    sigfillset(&block_mask);

    if(sigprocmask(SIG_SETMASK, &block_mask, NULL) < 0){
        printf("Error: Sigprocmask error.\n");
        exit(0);
    }

    if(strcmp(argv[3], "SIGRT") == 0){
        sigdelset(&new_mask, SIGRTMIN);
        sigdelset(&new_mask, SIGRTMIN+1);

        set_signals(SIGRTMIN, SIGRTMIN+1);
        for(int i = 0; i<n; i++) {
            if (kill(p, SIGRTMIN) < 0) {
                printf("Error: Kill function error.\n");
                exit(0);
            }
        }

        if (kill(p, SIGRTMIN+1) < 0) {
            printf("Error: Kill function error.\n");
            exit(0);
        }

    }
    else {

        sigdelset(&new_mask, SIGUSR1);
        sigdelset(&new_mask, SIGUSR2);
        set_signals(SIGUSR1, SIGUSR2);

        if (strcmp(argv[3], "KILL") == 0) {
            for (int i = 0; i < n; i++) {
                if (kill(p, SIGUSR1) < 0) {
                    printf("Error: Kill function error.\n");
                    exit(0);
                }
            }

            if (kill(p, SIGUSR2) < 0) {
                printf("Error: Kill function error.\n");
                exit(0);
            }
        } else if (strcmp(argv[3], "SIGQUEUE") == 0) {
            union sigval value;
            for (int i = 0; i < n; i++) {
                if (sigqueue(p, SIGUSR1, value) < 0) {
                    printf("Error: sigqueue function error.\n");
                    exit(0);
                }
            }

            if (sigqueue(p, SIGUSR2, value) < 0) {
                printf("Error: sigqueue function error.\n");
                exit(0);
            }
        }

    }

    while(process_waiting) {
        if (sigsuspend(&new_mask) != -1) {
            printf("Error: Sigsuspend error.\n");
            exit(0);
        }
    }


    return 0;
}

void check_arguments(int argc, char** argv){

    if(argc != 4){
        printf("Invalid amount of arguments.\n");
        exit(1);
    }

    for(int i = 0; i<strlen(argv[1]); i++){
        if( argv[1][i] < '0' || argv[1][i] > '9'){
            printf("Error: First argument must be a number.\n");
            exit(1);
        }
    }

    for(int i = 0; i<strlen(argv[2]); i++){
        if( argv[2][i] < '0' || argv[2][i] > '9'){
            printf("Error: Second argument must be an positive integer.\n");
            exit(1);
        }
    }

    if( ! ( strcmp(argv[3], "KILL") == 0 || strcmp(argv[3], "SIGQUEUE") == 0 || strcmp(argv[3], "SIGRT") == 0 )){
        printf("Error: Third argument should be: KILL, SIGQUEUE or SIGRT.\n");
        exit(1);
    }

}

void handler(int signo, siginfo_t* info, void *context){

    if(signo == SIGUSR1 || signo == SIGRTMIN) {
        cnt++;
    }
    else{
        process_waiting = 0;
        printf("Number of received signals: %d by sender\n", cnt);
        if(info->si_code == SI_QUEUE){
            printf("Catcher sended %d signals.\n", info->si_value.sival_int);
        }
        printf("Sender should receive %d signals\n", n);
        exit(0);
    }

}

void set_signals(int sig1, int sig2){
        struct sigaction act = {0};
        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = &handler;
        sigemptyset(&act.sa_mask);

        if (sigaction(sig1, &act, NULL) == -1) {
            printf("Error: Sigaction error.\n");
            exit(0);
        }
        if (sigaction(sig2, &act, NULL) == -1) {
            printf("Error: Sigaction error.\n");
            exit(0);
        }
}


