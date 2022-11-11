#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "pantry.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

int amount_of_cooks = -1;
int amount_of_suppliers = -1;
int* processes_pid = NULL;
char* cmd = NULL;
int id_sem = -1;
int id_shm_oven = -1;
int id_shm_worktop = -1;
int* data_oven = NULL;
int* data_worktop = NULL;

union semun {
    int value;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};

void check_arguments(int argc, char** argv);
void close_business();
void handler_end(int signum);

void at_end(){
    close_business();
    if(cmd != NULL) free(cmd);
    if(data_oven != NULL) shmdt(data_oven);
    if(data_worktop != NULL) shmdt(data_worktop);
    if(id_shm_oven != -1) shmctl(id_shm_oven, IPC_RMID, NULL);
    if(id_shm_worktop != -1) shmctl(id_shm_worktop, IPC_RMID, NULL);
    if(id_sem != -1) semctl(id_sem, 0, IPC_RMID);
    if(processes_pid != NULL) free(processes_pid);
}

int main(int argc, char** argv){

    atexit(at_end);
    signal(SIGINT, handler_end);
    check_arguments(argc, argv);

    amount_of_cooks = atoi(argv[1]);
    amount_of_suppliers = atoi(argv[2]);
    processes_pid = calloc(amount_of_cooks + amount_of_suppliers, sizeof(int));

    key_t sem_key = ftok(getenv("HOME"), SEM_KEY);
    key_t oven_key = ftok(getenv("HOME"), OVEN_KEY);
    key_t table_key = ftok(getenv("HOME"), TABLE_KEY);

    id_sem = semget(sem_key, 2, IPC_CREAT | 0666 );
    if(id_sem == -1){
        printf("Error: Cannot get semaphore.\n");
        exit(1);
    }
    id_shm_oven = shmget(oven_key, sizeof(int)*6, IPC_CREAT | 0666);
    if(id_shm_oven == -1){
        printf("Error: Cannot get access to shared memory.\n");
        exit(1);
    }
    id_shm_worktop = shmget(table_key, sizeof(int)*6, IPC_CREAT | 0666);
    if(id_shm_worktop == -1){
        printf("Error: Cannot get access to shared memory.\n");
        exit(1);
    }

    union semun arg;
    arg.value = 1;
    semctl(id_sem, 0, SETVAL, arg);
    semctl(id_sem, 1, SETVAL, arg);

    data_oven = shmat(id_shm_oven, NULL, 0);
    if(data_oven == (int*)-1){
        printf("Error: Cannot share memory.\n");
        exit(1);
    }
    data_worktop = shmat(id_shm_worktop, NULL, 0);
    if(data_worktop == (int*)-1){
        printf("Error: Cannot share memory.\n");
        exit(1);
    }

    for(int i = 0; i<5; i++){
        data_oven[i] = -1;
        data_worktop[i] = -1;
    }
    data_oven[5] = 0;
    data_worktop[5] = 0;

    for(int i = 0; i < amount_of_cooks; i++){
        pid_t pid = fork();
        if(pid == 0){
            execl("./cook", "", NULL);
            return 0;
        }
        else{
            processes_pid[i] = pid;
        }
    }

    for(int i = 0; i <amount_of_suppliers; i++){
        pid_t pid = fork();
        if(pid == 0){
            execl("./supplier","",NULL);
            return 0;
        }
        else{
            processes_pid[i+amount_of_cooks] = pid;
        }
    }

    cmd= NULL;
    size_t cmd_len = 100;
    getline(&cmd,&cmd_len, stdin);

    handler_end(0);

    return 0;
}

void check_arguments(int argc, char** argv){
    if(argc != 3){
        printf("Error: Invalid amount of arguments.\n");
        exit(1);
    }

    for(int i = 0; i < strlen(argv[1]); i++){
        if(argv[1][i] < '0' || argv[1][i] > '9'){
            printf("Error: First argument should be a positive integer.\n");
            exit(1);
        }
    }

    for(int i = 0; i < strlen(argv[2]); i++){
        if(argv[2][i] < '0' || argv[2][i] > '9'){
            printf("Error: Second argument should be a positive integer.\n");
            exit(1);
        }
    }
}

void close_business(){

    for(int i = 0; i<amount_of_suppliers + amount_of_cooks; i++){
        kill(processes_pid[i], SIGINT);
    }
    for(int i = 0; i<amount_of_cooks + amount_of_suppliers; i++){
        wait(NULL);
    }
}

void handler_end(int signum){
    exit(0);
}