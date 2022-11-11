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
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>

int amount_of_cooks;
int amount_of_suppliers;
int* processes_pid = NULL;
char* cmd = NULL;
sem_t* id_sem_oven = NULL;
sem_t* id_sem_worktop= NULL;
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

void at_end();
void check_arguments(int argc, char** argv);
void close_business();
void handler_end(int signum);

int main(int argc, char** argv){

    atexit(at_end);
    signal(SIGINT, handler_end);
    check_arguments(argc, argv);

    amount_of_cooks = atoi(argv[1]);
    amount_of_suppliers = atoi(argv[2]);
    processes_pid = calloc(amount_of_cooks + amount_of_suppliers, sizeof(int));

    id_sem_oven = sem_open(OVEN_KEY, O_CREAT, 0666, 1);
    if(id_sem_oven == SEM_FAILED){
        printf("Error: Cannot get semaphore.\n");
        exit(1);
    }

    id_sem_worktop = sem_open(WORKTOP_KEY, O_CREAT, 0666, 1);
    if(id_sem_worktop == SEM_FAILED){
        printf("Error: Cannot get semaphore.\n");
        exit(1);
    }


    id_shm_oven = shm_open(SHARED_MEM_OVEN, O_CREAT | O_RDWR, 0666);
    if(id_shm_oven == -1){
        printf("Error: Cannot get access to shared memory.\n");
        exit(1);
    }

    if(ftruncate(id_shm_oven, sizeof(int)*6) == -1){
        printf("Error: Cannot set memory size.\n");
        exit(1);
    }

    id_shm_worktop = shm_open(SHARED_MEM_WORKTOP, O_CREAT | O_RDWR, 0666);
    if(id_shm_worktop == -1){
        printf("Error: Cannot get access to shared memory.\n");
        exit(1);
    }

    if(ftruncate(id_shm_worktop, sizeof(int)*6) == -1){
        printf("Error: Cannot set memory size.\n");
        exit(1);
    }

    data_oven = mmap(NULL, sizeof(int)*6, PROT_READ | PROT_WRITE, MAP_SHARED,id_shm_oven, 0);
    if(data_oven == (void*)-1){
        printf("Error: Cannot share memory.\n");
        exit(1);
    }
    data_worktop = mmap(NULL, sizeof(int)*6, PROT_READ | PROT_WRITE, MAP_SHARED,id_shm_worktop, 0);
    if(data_worktop == (void*)-1){
        printf("Error: Cannot share memory.\n");
        exit(1);
    }

    for(int i = 0; i<5; i++){
        data_oven[i] = -1;
        data_worktop[i] = -1;
    }
    data_worktop[5] = 0;
    data_oven[5] = 0;

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

void at_end(){
    close_business();
    if(cmd != NULL) free(cmd);

    if(id_sem_oven != NULL) sem_close(id_sem_oven);
    if(id_sem_worktop != NULL) sem_close(id_sem_worktop);
    if(id_sem_oven != NULL) sem_unlink(OVEN_KEY);
    if(id_sem_worktop != NULL) sem_unlink(WORKTOP_KEY);

    if(data_oven != NULL) munmap(data_oven, sizeof(int)*6);
    if(data_worktop != NULL) munmap(data_worktop, sizeof(int)*6);
    if(id_shm_oven != -1) shm_unlink(SHARED_MEM_WORKTOP);
    if(id_shm_worktop != -1) shm_unlink(SHARED_MEM_OVEN);

    if(processes_pid != NULL) free(processes_pid);
}

void handler_end(int signum){
    exit(0);
}