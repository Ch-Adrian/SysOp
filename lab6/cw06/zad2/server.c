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
#include <time.h>
#include <mqueue.h>

mqd_t server = -1;
int client_queue[AMOUNT_OF_QUEUES];
int client_queue_pid[AMOUNT_OF_QUEUES];
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
int check_client_id(int client_id_to);

int main(int argc, char** argv){
    atexit(exit_function);
    signal(SIGINT, handler);

    file_session = fopen("server_session.txt", "w");

    for(int i = 0; i<AMOUNT_OF_QUEUES; i++){
        client_queue[i] = -1;
    }

    home_path = malloc(50*sizeof (char));
    strcpy(home_path, PATH_TO_SERVER);

    struct mq_attr mqAttr;
    mqAttr.mq_maxmsg = QUEUE_SIZE;
    mqAttr.mq_msgsize = sizeof (struct msgbuf );

    if((server = mq_open(home_path, O_RDONLY | O_CREAT | O_EXCL, 0666, &mqAttr)) == -1){
        printf("Error: Cannot create server queue.\n");
        printf("%d", errno);
        exit(1);
    }

    while(1){
        struct msgbuf structmsgbuf;
        if(mq_receive(server, (char*) &structmsgbuf, sizeof(struct msgbuf), NULL) == -1){
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
    char client_name[MSGLEN];
    strcpy(client_name, mess->mtext);
    int client_q = -1;
    if( (client_q = mq_open(client_name, O_WRONLY)) == -1){
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
        if(mq_send(client_q, (char*)&message_to_client, sizeof(struct msgbuf), 3) == -1){
            printf("Error: Cannot send message.\n");
            return;
        }
    }
    else{
        printf("Inquiry to give access to the client has been accepted.\n");
        client_queue[nextQueue] = client_q;
        client_queue_pid[nextQueue] = mess->client_id;
        message_to_client.mtype = _INIT;
        message_to_client.client_id = nextQueue;
        strcpy(message_to_client.mtext, "true");
        if(mq_send(client_q, (char*)&message_to_client, sizeof(struct msgbuf), 3) == -1){
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
            printf("Client: %d\n", client_queue_pid[i]);
            sprintf(mini_txt, "Client: %d\n", client_queue_pid[i]);
            strcat(txt, mini_txt);
        }
    }

    struct msgbuf msgbuf_to_one;
    msgbuf_to_one.mtype=_LIST;
    msgbuf_to_one.client_id = mess->client_id;
    
    strcpy(msgbuf_to_one.mtext, txt);
    printf("Sending list message to client: %d\n", mess->client_id);
    int client_id = check_client_id(mess->client_id);
    if(client_queue[client_id] != -1){
        if (mq_send(client_queue[client_id], (char*)&msgbuf_to_one, sizeof(struct msgbuf), 3) == -1) {
            printf("Error: Cannot send message to %d client.\n", client_id);
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
            if (mess->client_id != client_queue_pid[i]) {
                if (mq_send(client_queue[i], (char*)&msgbuf_to_all, sizeof(struct msgbuf), 1) == -1) {
                    printf("Error: Cannot send message to %d client.\n", client_queue_pid[i]);
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
    if(client_id_to< 0){
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
    if(mess->client_id < 0){
        printf("Error: Client id is not correct.\n");
        return;
    }
    struct msgbuf msgbuf_to_one;
    msgbuf_to_one.mtype = _2ONE;
    msgbuf_to_one.client_id = mess->client_id;
    strcpy(msgbuf_to_one.mtext, txt);
    printf("Sending message to one client: %d\n", client_id_to);
    client_id_to = check_client_id(client_id_to);
    if(client_id_to != -1){
        if (mq_send(client_queue[client_id_to], (char*)&msgbuf_to_one, sizeof(struct msgbuf), 1) == -1) {
            printf("Error: Cannot send message to %d client.\n", client_queue_pid[client_id_to]);
        }
    }
    else{
        printf("Error: Client of this ID doesn't exists.\n");
        return;
    }
}

int check_client_id(int client_pid){
    for(int i = 0; i<AMOUNT_OF_QUEUES; i++){
        if(client_queue_pid[i] == client_pid){
            return i;
        }
    }
    return -1;
}

void stop(struct msgbuf *mess){
    printf("Removing client %d from list.\n", mess->client_id);
    if(mess->client_id < 0){
        printf("Error: Client id is not correct.\n");
        return;
    }
    int client_id = check_client_id(mess->client_id);
    if(client_id == -1){
        printf("Error: Cannot find client.\n");
        return;
    }

    if(mq_close(client_queue[client_id]) == -1){
        printf("Error: Cannot close client queue.\n");
        return;
    }
    client_queue[client_id] = -1;
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
            if(mq_send(client_queue[i], (char*)&msgbuf_stop, sizeof (struct msgbuf), 4) == -1){
                printf("Error: Cannot send stop request to the client: %d.\n", i);
            }
        }
    }
    while(client_queue_exist()){
        if(mq_receive(server, (char*)&msgbuf_stop, sizeof(struct msgbuf), NULL) == -1){
            printf("Error: Cannot receive message.\n");
            exit(1);
        }
        save_to_file(&msgbuf_stop);
        handle_message(&msgbuf_stop);
    }
    printf("Stopping server queue.\n");
    if(server != -1) {
        if(mq_close(server) == -1){
            printf("Cannot close server queue\n");
        }
        else{
            printf("Server queue closed.\n");
        }
        if(mq_unlink(home_path) == -1){
            printf("Error: Cannot remove server queue.\n");
        }
        else{
            printf("Server queue removed.\n");
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
    free(home_path);
    home_path = NULL;
}

void handler(int sig){
    exit(0);
}