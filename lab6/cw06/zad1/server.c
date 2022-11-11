#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "properties.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include<time.h>

int server = -1;
int client_queue[AMOUNT_OF_QUEUES];
char* home_path;
FILE* file_session;
int is_stopping = 0;

void init(struct msgbuf *mess);
void list(struct msgbuf *mess);
void all(struct msgbuf *mess);
void one(struct msgbuf *mess);
void stop(struct msgbuf *mess);

int getNextSpaceForClient();
void handle_message(struct msgbuf *mess);
void exit_queue();
void handler(int sig);
int client_queue_exist();
void save_to_file(struct msgbuf *mess);
void exit_function();

int main(int argc, char** argv){
    atexit(exit_function);
    signal(SIGINT, handler);

    file_session = fopen("server_session.txt", "w");

    for(int i = 0; i<AMOUNT_OF_QUEUES; i++){
        client_queue[i] = -1;
    }

    home_path = getenv("HOME");
    if(home_path == NULL){
        printf("Error: Cannot get environment variable HOME.\n");
        exit(1);
    }

    key_t key_s = ftok(home_path, SERVER_Q_NO);

    if((server = msgget(key_s, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        printf("Error: Cannot create server queue.\n");
        exit(1);
    }

    while(1){
        struct msgbuf structmsgbuf;
        if(msgrcv(server, &structmsgbuf, sizeof(struct msgbuf)- sizeof(long), _ABSMIN, 0) == -1){
            printf("Error: Cannot receive message.\n");
            exit(1);
        }
        save_to_file(&structmsgbuf);
        handle_message(&structmsgbuf);
    }
    return 0;
}

void init(struct msgbuf *mess){
    printf("New client is connecting...\n");
    int client_key = atoi(mess->mtext);
    int client_q = -1;
    if( (client_q = msgget(client_key, 0)) == -1){
        printf("Error: Cannot get access to client.\n");
        return;
    }
    struct msgbuf message_to_client;
    int nextQueue = getNextSpaceForClient();
    if(is_stopping || nextQueue == -1){
        if(!is_stopping) printf("Inquiry to give access to the client has been refused: No more space.\n");
        else {
            printf("Inquiry to give access to the client has been refused: Server is stopping.\n");
        }
        message_to_client.mtype = _INIT;
        strcpy(message_to_client.mtext , "false");
        if(msgsnd(client_q, &message_to_client, sizeof(struct msgbuf) - sizeof(long), 0) == -1){
            printf("Error: Cannot send message.\n");
            return;
        }
    }
    else{
        printf("Inquiry to give access to the client has been accepted.\n");
        client_queue[nextQueue] = client_q;
        message_to_client.mtype = _INIT;
        message_to_client.client_id = nextQueue;
        strcpy(message_to_client.mtext, "true");
        if(msgsnd(client_q, &message_to_client, sizeof(struct msgbuf) - sizeof(long), 0) == -1){
            printf("Error: Cannot send message.\n");
            return;
        }
    }
}

void save_to_file(struct msgbuf *mess){
    time_t t;
    time(&t);
    fprintf(file_session, "Date: %s Client's id: %d Message: %s\n", ctime(&t), mess->client_id, mess->mtext);
}

int getNextSpaceForClient(){
    for(int i = 0; i<AMOUNT_OF_QUEUES; i++){
        if(client_queue[i] == -1){
            return i;
        }
    }
    return -1;
}

void list(struct msgbuf *mess){
    printf("Writing list of available clients...:\n");
    char txt[MSGLEN];
    char mini_txt[20];
    strcpy(txt, "");
    for(int i = 0; i < AMOUNT_OF_QUEUES; i++){
        if(client_queue[i] != -1) {
            printf("Client: %d\n", i);
            sprintf(mini_txt, "Client: %d\n", i);
            strcat(txt, mini_txt);
        }
    }

    struct msgbuf msgbuf_to_one;
    msgbuf_to_one.mtype=_LIST;
    msgbuf_to_one.client_id = mess->client_id;
    
    strcpy(msgbuf_to_one.mtext, txt);
    printf("Sending list message to client: %d\n", mess->client_id);
    if(client_queue[mess->client_id] != -1){
        if (msgsnd(client_queue[mess->client_id], &msgbuf_to_one, sizeof(struct msgbuf) - sizeof(long), 0) == -1) {
            printf("Error: Cannot send message to %d client.\n", mess->client_id);
        }
    }
    else{
        printf("Error: Client of this ID doesn't exists.\n");
    }
}

void all(struct msgbuf *mess){
    printf("Sending message to all clients.\n");
    struct msgbuf msgbuf_to_all;
    msgbuf_to_all.mtype=_2ALL;
    strcpy(msgbuf_to_all.mtext, mess->mtext);
    msgbuf_to_all.client_id = mess->client_id;
    for(int i = 0; i<AMOUNT_OF_QUEUES; i++){
        if(client_queue[i] != -1) {
            if (mess->client_id != i) {
                if (msgsnd(client_queue[i], &msgbuf_to_all, sizeof(struct msgbuf) - sizeof(long), 0) == -1) {
                    printf("Error: Cannot send message to %d client.\n", i);
                }
            }
        }
    }
}

void one(struct msgbuf *mess){
    if(mess->mtext[0] < '0' || mess->mtext[0] >'9'){
        printf("Error: Cannot recognize client id.\n");
        return;
    }
    int client_id_to = atoi(mess->mtext);
    if(client_id_to< 0 || client_id_to> AMOUNT_OF_QUEUES){
        printf("Error: Client id: %d is not correct.\n", client_id_to);
        return;
    }
    char* txt = strchr(mess->mtext, ' ');
    if(txt != NULL){
        txt = txt+1;
    }
    else{
        printf("Error: Cannot find message.\n");
        return;
    }
    if(mess->client_id < 0 || mess->client_id >= AMOUNT_OF_QUEUES){
        printf("Error: Client id is not correct.\n");
        return;
    }
    struct msgbuf msgbuf_to_one;
    msgbuf_to_one.mtype=_2ONE;
    msgbuf_to_one.client_id = mess->client_id;
    strcpy(msgbuf_to_one.mtext, txt);
    printf("Sending message to one client: %d\n", client_id_to);
    if(client_queue[client_id_to] != -1){
        if (msgsnd(client_queue[client_id_to], &msgbuf_to_one, sizeof(struct msgbuf) - sizeof(long), 0) == -1) {
            printf("Error: Cannot send message to %d client.\n", client_id_to);
        }
    }
    else{
        printf("Error: Client of this ID doesn't exists.\n");
        return;
    }
}

void stop(struct msgbuf *mess){
    printf("Removing client %d from list.\n", mess->client_id);
    if(mess->client_id < 0 || mess->client_id >=AMOUNT_OF_QUEUES){
        printf("Error: Client id is not correct.\n");
        return;
    }
    client_queue[mess->client_id] = -1;
    printf("Client %d has been removed.\n", mess->client_id);
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
            all(mess);
            break;
        case _2ONE:
            one(mess);
            break;
        case _STOP:
            stop(mess);
            break;
        default:
            break;
    }
}

void exit_queue(){
    is_stopping = 1;
    struct msgbuf msgbuf_stop;
    msgbuf_stop.mtype = _STOP;
    for(int i = 0; i<AMOUNT_OF_QUEUES; i++){
        if(client_queue[i] != -1){
            if(msgsnd(client_queue[i], &msgbuf_stop, sizeof (struct msgbuf) - sizeof(long), 0) == -1){
                printf("Error: Cannot send stop request to the client: %d.\n", i);
            }
        }
    }
    while(client_queue_exist()){
        if(msgrcv(server, &msgbuf_stop, sizeof(struct msgbuf)- sizeof(long), _ABSMIN, 0) == -1){
            printf("Error: Cannot receive message.\n");
            exit(1);
        }
        save_to_file(&msgbuf_stop);
        handle_message(&msgbuf_stop);
    }
    printf("Stopping server queue.\n");
    if(server != -1) {
        if (msgctl(server, IPC_RMID, 0) == -1) {
            printf("Error: Cannot delete queue.\n");
        }
    }
}

int client_queue_exist(){
    for(int i = 0; i<AMOUNT_OF_QUEUES; i++){
        if(client_queue[i] != -1){
            return 1;
        }
    }
    return 0;
}

void exit_function(){
    exit_queue();
    fclose(file_session);
}

void handler(int sig){
    exit(0);
}