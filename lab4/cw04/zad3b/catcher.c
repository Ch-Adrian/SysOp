#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include<sys/wait.h>

int cnt = 0;
int process_waiting = 1;

void check_arguments(int argc, char** argv);
void handler(int signo, siginfo_t* info, void *context);
void set_signals(int sig1,int sig2);

int main(int argc, char** argv){

    printf("PID: %d\n", getpid());

    sigset_t new_mask;
    sigset_t block_mask;

    sigfillset(&new_mask);
    sigdelset(&new_mask, SIGINT);
    sigfillset(&block_mask);

    if(sigprocmask(SIG_SETMASK, &block_mask, NULL) < 0){
        printf("Error: Sigprocmask error.\n");
        exit(0);
    }

    sigdelset(&new_mask, SIGRTMIN);
    sigdelset(&new_mask, SIGRTMIN+1);
    sigdelset(&new_mask, SIGUSR1);
    sigdelset(&new_mask, SIGUSR2);
    set_signals(SIGRTMIN, SIGRTMIN+1);
    set_signals(SIGUSR1, SIGUSR2);

    while(process_waiting) {
        if (sigsuspend(&new_mask) != -1) {
            printf("Error: Sigsuspend error.\n");
            exit(0);
        }
    }

    return 0;
}


void handler(int signo, siginfo_t* info, void *context){

    if(signo == SIGUSR1){
        cnt++;
        if(info->si_code == SI_QUEUE){
            union sigval value;
            value.sival_int = cnt;
            if (sigqueue(info->si_pid, SIGUSR1, value) < 0) {
                printf("Error: Kill function error.\n");
                exit(0);
            }
        }
        else {
            if (kill(info->si_pid, SIGUSR1) < 0) {
                printf("Error: Kill function error.\n");
                exit(0);
            }
        }
    }
    else if(signo == SIGRTMIN) {
        cnt++;
        if (kill(info->si_pid, SIGRTMIN) < 0) {
            printf("Error: Kill function error.\n");
            exit(0);
        }
    }
    else if(signo == SIGUSR2){
        process_waiting = 0;
        printf("Number of received signals: %d by catcher\n", cnt);
        if(info->si_code == SI_QUEUE){
            union sigval value;
            value.sival_int = cnt;
            if (sigqueue(info->si_pid, SIGUSR2, value) < 0) {
                printf("Error: Sigqueue function error.\n");
                exit(0);
            }
        }
        else {
            if (kill(info->si_pid, SIGUSR2) < 0) {
                printf("Error: Kill function error.\n");
                exit(0);
            }
        }
    }
    else if(signo == SIGRTMIN+1){
        process_waiting = 0;
        printf("Number of received signals: %d by catcher\n", cnt);
        if (kill(info->si_pid, SIGRTMIN+1) < 0) {
            printf("Error: Kill function error.\n");
            exit(0);
        }
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
