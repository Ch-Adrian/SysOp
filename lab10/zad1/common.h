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
#include <pthread.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <time.h>

#define MAX_DATA_SIZE 30
#define MAX_BACKLOG 15
#define MAX_GAMES 8
#define MAX_CLIENTS 16

typedef enum Client_type { NONE_CLIENT, LOCAL_CLIENT, INTERNET_CLIENT, STDIN_CLIENT } Client_type;
typedef enum Event_type { NONE_EVENT, SOCKET_EVENT, GAME_EVENT } Event_type;
typedef enum Message_type { NONE_MESS, PING_MESS, INIT_CLIENT_MESS,
                            OK_MESS, FULL_MESS, NAME_MESS,
                            GAME_STATE_MESS, MOVE_MESS, END_MESS,
                            END_CLIENT_MESS} Message_type;

typedef struct Client_data{
    Client_type client_type;
    char name[MAX_DATA_SIZE];
    int socket_fd;
    void* ptr;
} Client_data;

typedef struct Epoll_data{
    Client_data client_data;
    Event_type event_type;
} Epoll_data;

typedef struct Game_state{
    char field[9];
    char turn;
    char sign;
    char win; // 1 - win, 2 - lose, 3 - draw
} Game_state;

typedef struct Message_data{
    Message_type message_type;
    union{
        char name[MAX_DATA_SIZE];
        Game_state game_state;
        struct{
            char sign;
            char place;
        }move;
    } data;
} Message_data;

typedef struct thread_data{
    int fd;
    Client_type type;
    char name[MAX_DATA_SIZE];
} Thread_data;
