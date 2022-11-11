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
#include <mqueue.h>
#include <time.h>

mqd_t client_q = -1;
mqd_t server_q = -1;
int parent_pid = -1;
char server_name[50];
char home_path[50];

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

    strcpy(home_path, "/client");
    char client_pid[30];
    sprintf(client_pid, "%d", getpid());
    strcat(home_path, client_pid);

    strcat(server_name, PATH_TO_SERVER);

    if((server_q = mq_open(server_name, O_WRONLY)) == -1){
        printf("Error: Cannot get access to server.\n");
        exit(1);
    }

    struct mq_attr client_attr;
    client_attr.mq_maxmsg = QUEUE_SIZE;
    client_attr.mq_msgsize = sizeof (struct msgbuf );

    if((client_q = mq_open(home_path, O_CREAT | IPC_EXCL | O_RDONLY, 0666, &client_attr)) == -1){
        printf("Error: Cannot create queue.\n");
        exit(1);
    }

    struct msgbuf structmsgbuf;
    structmsgbuf.mtype = _INIT;
    structmsgbuf.client_id = getpid();
    strcpy(structmsgbuf.mtext, home_path);

    if(mq_send(server_q, (char*)&structmsgbuf, sizeof(struct msgbuf), 2) == -1){
        printf("Error: Cannot send INIT message to server.\n");
        exit(0);
    }

    pid_t pid = fork();
    if(pid == 0){
        parent_pid = getppid();
        while(1){
            struct msgbuf message_from_server;
            if(mq_receive(client_q, (char*)&message_from_server, sizeof(struct msgbuf), NULL) == -1){
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
                message_to_server.client_id = getpid();
                printf("Sending message to server.\n");
                if(mq_send(server_q, (char*)&message_to_server, sizeof(struct msgbuf), 2) == -1){
                    printf("Error: Cannot send message to server.\n");
                }
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
                message_to_server.client_id = getpid();
                printf("Sending message to server.\n");
                if(mq_send(server_q, (char*)&message_to_server, sizeof(struct msgbuf), 1) == -1){
                    printf("Error: Cannot send message to server.\n");
                }
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
                message_to_server.client_id = getpid();
                printf("Sending message to server.\n");
                if(mq_send(server_q, (char*)&message_to_server, sizeof(struct msgbuf), 1) == -1){
                    printf("Error: Cannot send message to server.\n");
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
        printf("Connected to the server. Client ID on the server: %d\n", parent_pid);
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
        message_to_server.client_id = parent_pid;
        strcpy(message_to_server.mtext, home_path);
        if(mq_send(server_q, (char*)&message_to_server, sizeof(struct msgbuf), 4) == -1){
            printf("Error: Cannot send message to server.\n");
        }

        if (mq_close(client_q) == -1) {
            printf("Error: Cannot delete client queue. Errno: %d\n", errno);
        }
        else{
            printf("Closed client queue.\n");
        }
        if (mq_close(server_q) == -1) {
            printf("Error: Cannot delete server queue. Errno: %d\n", errno);
        }
        else{
            printf("Closed server queue.\n");
        }
        client_q = -1;
        if(mq_unlink(home_path) == -1){
            printf("Error: Cannot remove queue.\n");
        }
        else{
            printf("Removed queue.\n");
        }

    }
}

void exit_function_parent(){
    exit_queue();
}

void handler_parent(int sig){
    exit(0);
}