#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "common.h"

#define MAXDATASIZE 100

int sockfd, numbytes = 0;
char buf[MAXDATASIZE];
struct hostent *he;
struct sockaddr_un their_addr;

int main(int argc, char** argv){

    if(argc != 2){
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    if((he = gethostbyname(argv[1])) == NULL){
        perror("gethostbyname");
        exit(1);
    }

    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    memset(&(their_addr), '\0', sizeof(struct sockaddr_un));
    their_addr.sun_family = AF_UNIX;
    strncpy(their_addr.sun_path, SOCKET_NAME, sizeof(their_addr.sun_path) - 1);
//    their_addr.sin_family = AF_INET;
//    their_addr.sin_port = htons(PORT);
//    their_addr.sin_addr = *((struct in_addr*)he->h_addr);

    if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof (their_addr)) == -1){
        perror("recv");
        exit(1);
    }

    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("Received: %s", buf);
    close(sockfd);
    return 0;

    return 0;
}