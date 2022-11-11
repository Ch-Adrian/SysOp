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
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>

int* data_worktop = NULL;
sem_t* id_sem_worktop= NULL;
int id_shm_worktop = -1;

void at_end(){
    if(id_sem_worktop != NULL) sem_close(id_sem_worktop);
    if(id_sem_worktop != NULL) sem_unlink(WORKTOP_KEY);
    if(data_worktop != NULL) munmap(data_worktop, sizeof(int)*6);
    if(id_shm_worktop != -1) shm_unlink(SHARED_MEM_OVEN);
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

    id_sem_worktop = sem_open(WORKTOP_KEY, 0, 0, 1);
    if(id_sem_worktop == SEM_FAILED){
        printf("Error: Cannot get semaphore.\n");
        exit(1);
    }

    id_shm_worktop = shm_open(SHARED_MEM_WORKTOP, O_RDWR, 0666);
    if(id_shm_worktop == -1){
        printf("Error: Cannot get access to shared memory.\n");
        exit(1);
    }

    if(ftruncate(id_shm_worktop, sizeof(int)*6) == -1){
        printf("Error: Cannot set memory size.\n");
        exit(1);
    }

    data_worktop = mmap(NULL, sizeof(int)*6, PROT_READ | PROT_WRITE, MAP_SHARED,id_shm_worktop, 0);
    if(data_worktop == (void*)-1){
        printf("Error: Cannot share memory.\n");
        exit(1);
    }

    while(1){

        if(sem_wait(id_sem_worktop) == -1){
            printf("Error: Cannot change semaphore.\n");
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


        if(sem_post(id_sem_worktop) == -1){
            printf("Error: Cannot change semaphore.\n");
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
