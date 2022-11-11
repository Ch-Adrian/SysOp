#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include "properties.h"
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>

int client_q = -1;
int server_q = -1;
int parent_pid = -1;
static int *shclient_id;

void init(struct msgbuf* mess);
void list(struct msgbuf* mess);
void _2all(struct msgbuf* mess);
void _2one(struct msgbuf* mess);
void stop(struct msgbuf* mess);

void handle_message(struct msgbuf *mess);
void exit_queue();
void exit_function_parent();
void handler_parent(int sig);

int main(int argc, char** argv){
    atexit(exit_function_parent);
    signal(SIGINT, handler_parent);
    shclient_id = mmap(NULL, sizeof *shclient_id, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    char* home_path = getenv("HOME");
    if(home_path == NULL){
        printf("Error: Cannot get environment variable HOME.\n");
        exit(1);
    }

    key_t key_c = ftok(home_path, getpid());
    key_t key_s = ftok(home_path, SERVER_Q_NO);

    if((server_q = msgget(key_s, 0)) == -1){
        printf("Error: Cannot get access to server.\n");
        exit(1);
    }

    if((client_q = msgget(key_c, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        printf("Error: Cannot create queue.\n");
        exit(1);
    }

    struct msgbuf structmsgbuf;
    structmsgbuf.mtype = _INIT;
    structmsgbuf.client_id = -1;
//    structmsgbuf.mtext = queue;
    sprintf(structmsgbuf.mtext, "%d", key_c);

    if(msgsnd(server_q, &structmsgbuf, sizeof(struct msgbuf) - sizeof(long), 0) == -1){
        printf("Error: Cannot send INIT message to server.\n");
        exit(0);
    }

    pid_t pid = fork();
    if(pid == 0){
        parent_pid = getppid();
        while(1){
            struct msgbuf message_from_server;
            if(msgrcv(client_q, &message_from_server, sizeof(struct msgbuf)- sizeof(long), _ABSMIN, 0) == -1){
                printf("Error: Cannot receive message.\n");
                exit(1);
            }
            printf("Received request.\n");
            handle_message(&message_from_server);
        }
        return 0;
    }
    else{
        size_t len = MSGLEN+5;
//        char cmd[len];
        char* cmd = NULL;
        while(1){
            struct msgbuf message_to_server;
            getline(&cmd, &len, stdin);
            char first_part[5];
            char second_part[MSGLEN];
            if(strlen(cmd)>=4) {
                strncpy(first_part, cmd, 4);
                first_part[4]='\0';
            }
            else{
                free(cmd);
                cmd=NULL;
                continue;
            }
            if(strlen(cmd)>=6) {
                strncpy(second_part, cmd + 5, strlen(cmd + 5));
                second_part[strlen(cmd+5)-1] = '\0';
            }
            if(strcmp(first_part, "LIST") == 0){
                message_to_server.mtype=_LIST;
                strcpy(message_to_server.mtext,"Command list");
            }
            else if(strcmp(first_part, "2ALL") == 0) {
                if(strlen(cmd)<6){
                    printf("Error: Command 2ALL should have 1 parameter.\n");
                    free(cmd);
                    cmd=NULL;
                    continue;
                }
                message_to_server.mtype=_2ALL;
                char date[100];
                time_t t;
                time(&t);
                sprintf(date, " %s", ctime(&t));
                strcat(second_part, date);
                strcpy(message_to_server.mtext, second_part);

            } else if(strcmp(first_part, "2ONE") == 0){
                if(strlen(cmd)<6){
                    printf("Error: Command 2ONE should have 2 parameters.\n");
                    free(cmd);
                    cmd=NULL;
                    continue;
                }
                message_to_server.mtype=_2ONE;
                char date[100];
                time_t t;
                time(&t);
                sprintf(date, " %s", ctime(&t));
                strcat(second_part, date);
                char* txt = strchr(second_part, ' ');
                if(txt != NULL) {
                    txt += 1;
                    strcpy(message_to_server.mtext, second_part);
                }
                else{
                    printf("Error: Cannot find second parameter.\n");
                    free(cmd);
                    cmd=NULL;
                    continue;
                }

            } else if(strcmp(first_part, "STOP") == 0){
                kill(pid, SIGINT);
                kill(getpid(), SIGINT);
            }
            else{
                printf("Cannot match command!\nPlease enter one of the following: LIST, 2All, 2ONE, STOP.\n");
                free(cmd);
                cmd= NULL;
                continue;
            }

            message_to_server.client_id = *shclient_id;
            printf("Sending message to server.\n");
            if(msgsnd(server_q, &message_to_server, sizeof(struct msgbuf)- sizeof(long), 0) == -1){
                printf("Error: Cannot send message to server.\n");
            }


            free(cmd);
            cmd= NULL;
        }
    }

    return 0;
}

void init(struct msgbuf* mess){

    if(strcmp(mess->mtext, "false")==0){
        printf("Connection to the server failed.\n");
        kill(getppid(), SIGINT);
        exit(0);
    }
    else{
        *shclient_id = mess->client_id;
        printf("Connected to the server. Client ID on the server: %d\n", *shclient_id);
        return;
    }
}

void list(struct msgbuf* mess){
    printf("List:\n%s", mess->mtext);
}

void _2all(struct msgbuf* mess){
    printf("From %d\nMessage: %s", mess->client_id, mess->mtext);
}

void _2one(struct msgbuf* mess){
    printf("From %d\nMessage: %s", mess->client_id, mess->mtext);
}

void stop(struct msgbuf* mess){
    kill(getppid(), SIGINT);
    exit(0);
}

void handle_message(struct msgbuf *mess){
    switch(mess->mtype){
        case _INIT:
            init(mess);
            break;
        case _LIST:
            list(mess);
            break;
        case _2ALL:
            _2all(mess);
            break;
        case _2ONE:
            _2one(mess);
            break;
        case _STOP:
            stop(mess);
            break;
        default:
            break;
    }
}

void exit_queue(){
    if(client_q != -1 && parent_pid != -1) {
        struct msgbuf message_to_server;
        message_to_server.mtype=_STOP;
        message_to_server.client_id = *shclient_id;
        strcpy(message_to_server.mtext, "Stopping queue");
        if(msgsnd(server_q, &message_to_server, sizeof(struct msgbuf)- sizeof(long), 0) == -1){
            printf("Error: Cannot send message to server.\n");
        }

        if (msgctl(client_q, IPC_RMID, 0) == -1) {
            printf("Error: Cannot delete queue. Errno: %d\n", errno);
        }
        client_q = -1;
    }
}

void exit_function_parent(){
    exit_queue();
    if(parent_pid == -1) {
        munmap(shclient_id, sizeof *shclient_id);
    }
}

void handler_parent(int sig){
    exit(0);
}