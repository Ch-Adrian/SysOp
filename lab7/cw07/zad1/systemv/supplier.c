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
#include <time.h>

int* data_worktop = NULL;
int id_sem = -1;
int id_shm_worktop = -1;

void at_end(){
    if(data_worktop != NULL) shmdt(data_worktop);
    if(id_shm_worktop != -1) shmctl(id_shm_worktop, IPC_RMID, NULL);
    if(id_sem != -1) semctl(id_sem, 0, IPC_RMID);
}

void handler(int signum){
    exit(0);
}

void getCurrentTime(){
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME, &timestamp);
    time_t timet = timestamp.tv_sec;
    char out_time[150];
    char milis_time[100];
    sprintf(out_time,"%s", ctime(&timet));
    char * find = strrchr(out_time, ' ');
    sprintf(milis_time, ":%d", ((int)timestamp.tv_nsec/1000)%1000 );
    strcpy(find, milis_time);
    printf("%s",out_time);
}

int main(int argc, char** argv){

    atexit(at_end);
    signal(SIGINT, handler);
    srand(time(NULL));
    key_t sem_key = ftok(getenv("HOME"), SEM_KEY);
    key_t table_key = ftok(getenv("HOME"), TABLE_KEY);

    id_sem = semget(sem_key, 2, 0 );
    if(id_sem == -1){
        printf("Error: Cannot get semaphore.\n");
        exit(1);
    }

    id_shm_worktop = shmget(table_key, 0, 0);
    if(id_shm_worktop == -1){
        printf("Error: Cannot get access to shared memory.\n");
        exit(1);
    }

    data_worktop = shmat(id_shm_worktop, NULL, 0);
    if(data_worktop == (int*)-1){
        printf("Error: Cannot share memory.\n");
        exit(1);
    }

    while(1){

        struct sembuf arr_sops[1];
        arr_sops[0].sem_num = 0;
        arr_sops[0].sem_op = -1;
        arr_sops[0].sem_flg = 0;
        if(semop(id_sem, arr_sops, 1) == -1){
            printf("Error: Cannot change semaphore.\n");
            shmdt(data_worktop);
            exit(1);
        }

        int pizza_idx = -1;
        int pizza = -1;
        int num_of_pizzas = 0;
        int i = 0;
        while(data_worktop[data_worktop[5]] == -1){
            data_worktop[5] = (data_worktop[5]+1)%5;
            i++;
            if(i == 5) break;
        }
        if(data_worktop[data_worktop[5]] != -1){
            pizza_idx = data_worktop[5];
            pizza = data_worktop[pizza_idx];
            data_worktop[pizza_idx] = -1;
        }
        for(int i = 0; i<5; i++){
            if(data_worktop[i] != -1) {
                num_of_pizzas++;
            }
        }

        if(pizza_idx != -1){
            printf("(%d ", getpid());
            getCurrentTime();
            printf(" ) Pobieram pizze: %d Liczba pizz na stole: %d\n", pizza, num_of_pizzas);
        }

        arr_sops[0].sem_op = 1;
        if(semop(id_sem, arr_sops,1) == -1){
            printf("Error: Cannot change semaphore.\n");
            shmdt(data_worktop);
            exit(1);
        }

        if(pizza_idx != -1){
            sleep(4 + rand()%2);
            printf("(%d ", getpid());
            getCurrentTime();
            printf(" ) Dostarczam pizze: %d\n", pizza);
            sleep(4 + rand()%2);
        }

    }

    return 0;
}
