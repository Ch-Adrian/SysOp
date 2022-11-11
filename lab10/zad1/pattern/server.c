#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/un.h>
#include "common.h"

#define BACKLOG 10

int sockfd, new_fd;
struct sockaddr_un my_addr;
struct sockaddr_un their_addr;
unsigned int sin_size;
struct sigaction sa;
int yes = 1;

void sigchld_handler(int s){
    while(wait(NULL)>0);
}

void un_link(){
    printf("I am unlinking socket!\n");
    close(sockfd);
    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){

    signal(SIGINT, un_link);

    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    memset(&(my_addr), '\0', sizeof(my_addr));
    my_addr.sun_family = AF_UNIX;

    printf("Size sun_family: %lu\n", sizeof(my_addr.sun_family));
    printf("Size sun_path: %lu\n", sizeof(my_addr.sun_path));
    printf("Size my_addr: %lu\n", sizeof(my_addr));
    printf("Size sockaddr_un: %lu\n", sizeof(struct sockaddr_un));
    printf("Size string: %lu\n", sizeof(SOCKET_NAME));
    printf("String %s\n", SOCKET_NAME);

    strncpy(my_addr.sun_path, SOCKET_NAME, sizeof(my_addr.sun_path)-1);
    printf("Address %s\n", my_addr.sun_path);
//    my_addr.sin_port =htons(MYPORT);
//    my_addr.sin_addr.s_addr = INADDR_ANY;
    printf("host: %d\n", INADDR_ANY);


    if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd, BACKLOG) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }

    while(1){
        sin_size =sizeof(struct sockaddr_un);
        if((new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size)) == -1){
            perror("accept");
            continue;
        }

        printf("Server: got connection from %hu\n", their_addr.sun_family);

        if(!fork()){
            close(sockfd);
            if(send(new_fd, "Hello World!\n", 14, 0) == -1){
                perror("send");
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);

    }
    close(sockfd);
    unlink(SOCKET_NAME);

    return 0;
}