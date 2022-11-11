#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// Santa
int santa_sleeps = 1;
int santa_cnt = 0;
pthread_mutex_t santaMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  santaWaiting = PTHREAD_COND_INITIALIZER;

//Elves
int amt_working_elves = 0;
int santa_not_solved = 1;
int santa_meeting = 1;
pthread_mutex_t elfMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elfMutex_meeting = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  elfCondWaiting = PTHREAD_COND_INITIALIZER;
pthread_cond_t  elfCondWorking = PTHREAD_COND_INITIALIZER;

//Reindeers
int amt_waiting_reindeers = 0;
int santa_not_delivered = 0;
int series_end = 1;
pthread_mutex_t reindeerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  reindeerCondWaiting = PTHREAD_COND_INITIALIZER;
pthread_cond_t  reindeerCondWorking = PTHREAD_COND_INITIALIZER;

pthread_t elf[10];
pthread_t reindeer[9];
pthread_t santa;
pthread_t tid_elves[3];

void* elf_routine(void* arg);
void* reindeer_routine(void* arg);
void* santa_routine(void* arg);
void sleep_thread(int beg, int end);

int main(int argc, char** argv){

    for(int i = 0; i<9; i++){
        pthread_create(&reindeer[i], NULL, reindeer_routine, NULL);
    }

    for(int i = 0; i<10; i++){
        pthread_create(&elf[i], NULL, elf_routine, NULL);
    }

    pthread_create(&santa, NULL, santa_routine, NULL);
    pthread_join(santa,NULL);

    for(int i = 0; i<9; i++){
        pthread_kill(reindeer[i], SIGINT);
    }
    for(int i = 0; i<10; i++){
        pthread_kill(elf[i], SIGINT);
    }

    return 0;
}

void sleep_thread(int beg, int end){
    struct timespec timestamp2;
    clock_gettime(CLOCK_REALTIME, &timestamp2);
    usleep(beg*1000000 + timestamp2.tv_nsec%((end-beg)*1000000));
}

void* elf_routine(void* arg){

    while(1){
        sleep_thread(2, 5);
        pthread_mutex_lock(&elfMutex);

        while(amt_working_elves >= 3) {
            printf("Elf: czeka na powrot elfow, %lu\n", pthread_self());
            pthread_cond_wait(&elfCondWaiting, &elfMutex);
        }

        tid_elves[amt_working_elves] = pthread_self();
        amt_working_elves++;
        printf("Elf: czeka %d elfow na Mikolaja, %lu\n", amt_working_elves, pthread_self());
        if(amt_working_elves == 3){
            printf("Elf: wybudzam Mikolaja, %lu\n", pthread_self());
            santa_sleeps = 0;
            santa_not_solved = 1;
            pthread_cond_broadcast(&santaWaiting);
        }

        while(santa_not_solved) {
            pthread_cond_wait(&elfCondWorking, &elfMutex);
            pthread_mutex_lock(&elfMutex_meeting);
            if(!santa_meeting){
                santa_meeting = 1;
                printf("Elf: Mikolaj rozwiazuje problem, %lu\n", pthread_self());
            }
            pthread_mutex_unlock(&elfMutex_meeting);
        }

        amt_working_elves -= 1;

        if(amt_working_elves == 0){
            santa_not_solved = 1;
            santa_meeting = 1;
            pthread_cond_broadcast(&elfCondWaiting);
        }

        pthread_mutex_unlock(&elfMutex);

    }

    pthread_exit(NULL);
}

void* reindeer_routine(void* arg){
    while(1){
        sleep_thread(5, 10);
        pthread_mutex_lock(&reindeerMutex);
        amt_waiting_reindeers++;
        printf("Renifer: czeka %d reniferow na Mikolaja, %lu\n", amt_waiting_reindeers, pthread_self());

        while(amt_waiting_reindeers < 9)
            pthread_cond_wait(&reindeerCondWaiting, &reindeerMutex);

        if(series_end){
            printf("Renifer: wybudzam Mikolaja, %lu\n", pthread_self());
            santa_not_delivered = 1;
            santa_sleeps = 0;
            series_end = 0;
            pthread_cond_broadcast(&santaWaiting);
            pthread_cond_broadcast(&reindeerCondWaiting);
        }

        while(santa_not_delivered)
            pthread_cond_wait(&reindeerCondWorking, &reindeerMutex);

        amt_waiting_reindeers--;
        if(amt_waiting_reindeers == 0){
            santa_not_delivered = 0;
            series_end = 1;
        }

        pthread_mutex_unlock(&reindeerMutex);
    }

    pthread_exit(NULL);
}

void* santa_routine(void* arg){

    pthread_mutex_lock(&santaMutex);
    while(1){

        while(santa_sleeps)
            pthread_cond_wait(&santaWaiting, &santaMutex);
        santa_sleeps = 1;
        printf("Mikolaj: budze sie.\n");
        if(santa_not_delivered){
            printf("Mikolaj: dostarczam zabawki.\n");
            santa_cnt++;
            sleep_thread(2,4);
            santa_not_delivered = 0;
            pthread_cond_broadcast(&reindeerCondWorking);
        }

        if(santa_cnt == 3){
            break;
        }

        if(santa_not_solved){
            santa_not_solved = 0;
            printf("Mikolaj: rozwiazuje problemy elfow %lu %lu %lu %lu\n", tid_elves[0], tid_elves[1], tid_elves[2], pthread_self());
            pthread_mutex_lock(&elfMutex_meeting);
            santa_meeting = 0;
            pthread_cond_broadcast(&elfCondWorking);
            pthread_mutex_unlock(&elfMutex_meeting);
            sleep_thread(1,2);
            pthread_cond_broadcast(&elfCondWorking);
        }
        printf("Mikolaj: zasypiam.\n");
    }
    pthread_mutex_unlock(&santaMutex);

    pthread_exit(NULL);
}