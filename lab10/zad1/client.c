#include "common.h"

pthread_t pthread_response;

int epoll_sockets;
int sockfd, numbytes = 0;
char* message;
size_t mess_size = 100;

enum mode{ NONE_MODE, INET_MODE, UNIX_MODE } mode;

char board_format[] = "=============\n| %c | %c | %c |\n=============\n| %c | %c | %c |\n=============\n| %c | %c | %c |\n=============\n";
char is_turn_format[] = "You are: %c   It is Your turn, please choose position (1,...,9) or enter 'X' to exit:\n";
char no_turn_format[] = "You are: %c   It is not Your turn, please wait for opponent or enter 'X' to exit:\n";

Game_state *game_state = NULL;

void check_args(int argc, char** argv);
void render_game();
void* game_routine(void* arg);

void clean_mem(){
//    printf("Cleaning memory.\n");
    close(sockfd);
    free(message);
    free(game_state);
}

void end_f(){
    Message_data end_client;
    end_client.message_type = END_CLIENT_MESS;
    if (send(sockfd, &end_client,  sizeof (Message_data), 0) == -1) {
        perror("send");
        exit(1);
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){

    check_args(argc, argv);

    atexit(clean_mem);
//    signal(SIGINT, end_f);

    message = calloc(100, sizeof(char));
    game_state = malloc(sizeof(Game_state));

    if(mode == INET_MODE){
        struct sockaddr_in addr_in;
        memset(&addr_in, 0, sizeof(addr_in));
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = htons(atoi(argv[4]));

        if (inet_pton(AF_INET, argv[3], &addr_in.sin_addr) <= 0) {
            printf("Invalid address\n");
            exit(0);
        }

        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            perror("socket");
            exit(1);
        }

//        printf("Connecting ...\n");
        if(connect(sockfd, (struct sockaddr *)&addr_in, sizeof (addr_in)) == -1){
            perror("connect");
            exit(1);
        }
    }
    else if(mode == UNIX_MODE){
        struct sockaddr_un addr_un;
        memset(&(addr_un), '\0', sizeof(addr_un));
        addr_un.sun_family = AF_UNIX;
        strncpy(addr_un.sun_path, argv[3], sizeof (addr_un.sun_path)-1);

        if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
            perror("socket");
            exit(1);
        }

//        printf("Connecting ...\n");
        if(connect(sockfd, (struct sockaddr *)&addr_un, sizeof (addr_un)) == -1){
            perror("connect");
            exit(1);
        }
    }

    Message_data init_message = {.message_type = INIT_CLIENT_MESS};
//    printf("name: %s\n", argv[1]);
    strncpy(init_message.data.name, argv[1], strlen(argv[1]));
//    printf("init name: %s\n", init_message.data.name);

//    printf("Sending initial message.\n");
    if (send(sockfd, &init_message,  sizeof (Message_data), 0) == -1) {
        perror("send");
        exit(1);
    }
//    printf("Waiting for response...\n");
    if ((numbytes = recv(sockfd, &init_message, sizeof(Message_data), 0)) == -1) {
        perror("recv");
        exit(1);
    }

    if(init_message.message_type == OK_MESS){
        printf("Client connected to server.\n");
        printf("Waiting for opponent...\n");
    }
    else if(init_message.message_type == NAME_MESS){
        printf("Client cannot connect to server, because of already used nickname.\n");
        exit(1);
    }
    else if(init_message.message_type == FULL_MESS){
        printf("Client cannot connect to server, because server is full.\n");
        exit(1);
    }
    else{
        printf("Client cannot connect to server!\n");
        exit(1);
    }

    epoll_sockets = epoll_create1(0);

    struct epoll_event event_in = { .events = EPOLLIN | EPOLLET | EPOLLHUP  | EPOLLPRI };
    Epoll_data* epoll_data_in = event_in.data.ptr = malloc(sizeof (Epoll_data));
    epoll_data_in->client_data.client_type = STDIN_CLIENT;
    epoll_data_in->client_data.socket_fd = STDIN_FILENO;
    epoll_data_in->event_type = SOCKET_EVENT;
    epoll_ctl(epoll_sockets, EPOLL_CTL_ADD, STDIN_FILENO, &event_in);

    struct epoll_event event_socket= { .events = EPOLLIN | EPOLLET | EPOLLHUP  | EPOLLPRI };
    Epoll_data * epoll_data = event_socket.data.ptr = malloc(sizeof (Epoll_data));
    epoll_data->client_data.socket_fd  = sockfd;
    epoll_data->event_type = SOCKET_EVENT;

    if(mode == INET_MODE){
        epoll_data->client_data.client_type  = INTERNET_CLIENT;
    }
    else if(mode == UNIX_MODE){
        epoll_data->client_data.client_type  = LOCAL_CLIENT;
    }

    epoll_ctl(epoll_sockets, EPOLL_CTL_ADD, sockfd, &event_socket);

    struct epoll_event events[10];
    while(1){
        int epoll_out;
//        printf("epoll wait\n");
        if ((epoll_out = epoll_wait(epoll_sockets, events, 10, 500))== -1) {
//            if(errno == EINTR){
//                continue;
//            }
//            printf("Error: Epoll failed.\n");
//            perror("epoll");
            continue;
        }

        for (int i = 0; i < epoll_out; i++) {
            Epoll_data* data = events[i].data.ptr;
//            printf("Event: %d\n", events[i].events);

            if(data->client_data.client_type == STDIN_CLIENT){
//                int len = getline(&message, &mess_size, stdin);
                    int len = read(STDIN_FILENO,message, mess_size);
//                printf("len: %d, message: %s ",len,  message);
                if(len != 2){
                    printf("Please enter following: 1...9 or X .\n");
                    continue;
                }

                if(message[0] == 'X' || message[0] == 'x'){
                    end_f();
                }
                else if(game_state->turn == 0){
                    printf("Wait for opponent move...\n");
                    continue;
                }
                else if(message[0] >= '1' && message[0] <= '9'){
                    if(game_state->field[message[0]-'1'] == 'X' || game_state->field[message[0]-'1'] == 'O'){
                        printf("You cannot move here. This place is taken!\n");
                        continue;
                    }
                    Message_data move_message;
                    move_message.message_type = MOVE_MESS;
                    move_message.data.move.place = message[0];
                    move_message.data.move.sign = game_state->sign;
                    if (send(sockfd, &move_message, sizeof(Message_data), 0) == -1) {
                        perror("send");
                    }
                    continue;
                }
                else{
                    printf("Please enter following: 1...9 or X .\n");
                    continue;
                }

            }
            else{
                printf("goe event\n");
                if(data->client_data.client_type == INTERNET_CLIENT){

                }
                else if(data->client_data.client_type == LOCAL_CLIENT){

                }
                Thread_data* thread_data = malloc(sizeof (Thread_data));
                thread_data->fd = data->client_data.socket_fd;

//                pthread_create(&pthread_response, NULL, game_routine, (void*)thread_data);
                Message_data recv_message;
                if ((numbytes = recv(thread_data->fd, &recv_message, sizeof(Message_data), 0)) == -1) {
                    perror("recv");
//                    exit(1);
                    continue;
                }
                printf("Received: %d\n", recv_message.message_type);
                if(recv_message.message_type == PING_MESS){
                    Message_data ping_message;
                    ping_message.message_type = PING_MESS;
                    printf("Ping was sent.\n");
                    if (send(thread_data->fd, &ping_message, sizeof(Message_data), 0) == -1) {
                        perror("send");
                        //exit(1);
                        continue;
                    }
                }
                else if(recv_message.message_type == GAME_STATE_MESS){

                    strncpy(game_state->field, recv_message.data.game_state.field, 9);
                    game_state->sign = recv_message.data.game_state.sign;
                    game_state->turn = recv_message.data.game_state.turn;
                    game_state->win = recv_message.data.game_state.win;

                    render_game();
                }
                else if(recv_message.message_type == END_MESS){
                    printf("Game has stopped! You have to connect again.\n");
                    exit(0);
                }

                free(thread_data);
            }
        }
    }

    return 0;
}

void check_args(int argc, char** argv){

    if(argc != 4 && argc != 5){
        printf("Error: Invalid amount of arguments.\n");
        printf("Arguments are: name [INET|UNIX] [IPv4 PORT|path]\n");
        exit(1);
    }

    if(strlen(argv[1]) >= MAX_DATA_SIZE){
        printf("Error: Name is too long. Name should have less than %d characters.\n", MAX_DATA_SIZE);
        exit(1);
    }

    if(strcmp(argv[2], "INET") == 0){
        mode = INET_MODE;
    }
    else if(strcmp(argv[2], "UNIX") == 0){
        mode = UNIX_MODE;
    }
    else{
        printf("Error: Invalid second argument. Should be INET or UNIX.\n");
        exit(1);
    }

    if(mode == INET_MODE){
        int port = atoi(argv[4]);
        if(!(port > 1024 && port < 65535)){
            printf("Error: Invalid port should be from 1024 to 65535.\n");
            exit(1);
        }
    }

}

void render_game(){

//    system("clear");
    printf("\e[1;1H\e[2J");
    printf(board_format,
           game_state->field[0],
           game_state->field[1],
           game_state->field[2],
           game_state->field[3],
           game_state->field[4],
           game_state->field[5],
           game_state->field[6],
           game_state->field[7],
           game_state->field[8]);

    if(game_state->win == 1){
        printf("You have won!!!\n");
        end_f();
    }
    else if(game_state->win == 2){
        printf("You have lost!!!\n");
        end_f();
    }
    else if(game_state->win == 3){
        printf("Draw!!!\n");
        end_f();
    }
    else{
        if(game_state->turn){
            printf(is_turn_format, game_state->sign);
        }
        else{
            printf(no_turn_format, game_state->sign);
        }
    }
}

void* game_routine(void* arg){

    Thread_data *thread_data = (Thread_data*) arg;
    Message_data recv_message;
    if ((numbytes = recv(thread_data->fd, &recv_message, sizeof(Message_data), 0)) == -1) {
        perror("recv");
        exit(1);
    }
            printf("Received: %d\n", recv_message.message_type);
    if(recv_message.message_type == PING_MESS){
        Message_data ping_message;
        ping_message.message_type = PING_MESS;
                printf("Ping was sent.\n");
        if (send(thread_data->fd, &ping_message, sizeof(Message_data), 0) == -1) {
            perror("send");
            exit(1);
        }
    }
    else if(recv_message.message_type == GAME_STATE_MESS){

        strncpy(game_state->field, recv_message.data.game_state.field, 9);
        game_state->sign = recv_message.data.game_state.sign;
        game_state->turn = recv_message.data.game_state.turn;
        game_state->win = recv_message.data.game_state.win;

        render_game();
    }
    else if(recv_message.message_type == END_MESS){
        printf("Game has stopped! You have to connect again.\n");
        exit(0);
    }

    free(thread_data);
    pthread_exit(0);
}