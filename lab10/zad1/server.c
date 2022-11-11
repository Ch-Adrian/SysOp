#include "common.h"

pthread_t pthread_connection;
pthread_t pthread_ping;
pthread_t pthread_add_client;
pthread_t pthread_menage_game;

pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t epoll_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ping_mutex = PTHREAD_MUTEX_INITIALIZER;

int epoll_sockets;
int sockfd;
int sockfd_un;
char* socket_name;
struct {
    struct {
        Client_data client[2];
        char field[9];
    } board[MAX_GAMES];
} games;

int delete_two_clients(int socket_fd);
void* menage_game_routine(void* arg);
void* add_client_routine(void* arg);
void* provide_connection(void* arg);
void manage_place_for_client(int new_fd, Client_type type, char* name);
void delete_client(int sock_fd);
void create_game(int board_id);
void* ping_routine(void* arg);
int check_win(int board_id);
int check_three(int board_id, int id1, int id2, int id3);
int check_draw(int board_id);

int ping_cells[MAX_CLIENTS];

void clean_mem(){
    close(sockfd);
//    printf("I am unlinking socket!\n");
    close(sockfd_un);
    unlink(socket_name);
}

void end_f(){
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){

    atexit(clean_mem);
    signal(SIGINT, end_f);
    srand(time(NULL));

    if(argc != 3){
        printf("Error: Invalid amount of arguments.\n");
        printf("Should be: port path_to_socket.\n");
        exit(1);
    }

    int port = atoi(argv[1]);
    if(!(port > 1024 && port < 65535)){
        printf("Error: Invalid port should be from 1024 to 65535.\n");
        exit(1);
    }

    socket_name = argv[2];

    for(int i = 0; i<MAX_GAMES; i++){
        for(int j = 0; j<2; j++){
            games.board[i].client[j].client_type = NONE_CLIENT;
            games.board[i].client[j].socket_fd = -1;
        }
        for(int j = 0; j<9; j++){
            games.board[i].field[j] = '1'+j;
        }
    }

    int yes = 1;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("setsockopt");
        exit(1);
    }

    int yes_un = 1;
    if((sockfd_un = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(sockfd_un, SOL_SOCKET, SO_REUSEADDR, &yes_un, sizeof(int)) == -1){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    printf("host: %d\n", INADDR_ANY);
    memset(&(my_addr.sin_zero), '\0', 8);

    if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1){
        perror("bind");
        exit(1);
    }

    if(listen(sockfd, MAX_BACKLOG) == -1){
        perror("listen");
        exit(1);
    }

    struct sockaddr_un my_addr_un;
    memset(&(my_addr_un), '\0', sizeof(my_addr_un));
    my_addr_un.sun_family = AF_UNIX;
    strncpy(my_addr_un.sun_path, socket_name, sizeof(my_addr_un.sun_path)-1);

    if(bind(sockfd_un, (struct sockaddr*)&my_addr_un, sizeof(my_addr_un)) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd_un, MAX_BACKLOG) == -1){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // EPOLL
    epoll_sockets = epoll_create1(0);

    struct epoll_event event1 = { .events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLPRI };
    Epoll_data* data1 = event1.data.ptr = malloc(sizeof (Epoll_data));
    data1->client_data.client_type = LOCAL_CLIENT;
    data1->client_data.socket_fd = sockfd_un;
    data1->event_type = SOCKET_EVENT;
    epoll_ctl(epoll_sockets, EPOLL_CTL_ADD, sockfd_un, &event1);

    struct epoll_event event2 = { .events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLPRI };
    Epoll_data* data2 = event2.data.ptr = malloc(sizeof (Epoll_data));
    data2->client_data.client_type = INTERNET_CLIENT;
    data2->client_data.socket_fd = sockfd;
    data2->event_type = SOCKET_EVENT;
    epoll_ctl(epoll_sockets, EPOLL_CTL_ADD, sockfd, &event2);

//    printf("sockfd: %d\n", sockfd);
//    printf("sockfd_un: %d\n", sockfd_un);

    // Threads
//    pthread_create(&pthread_connection, NULL, provide_connection, NULL);
    pthread_create(&pthread_ping, NULL, ping_routine, NULL);

    struct epoll_event events[10];
    int epoll_out= 0;

    while(1){
        printf("Epoll waiting... \n");
        pthread_mutex_lock(&epoll_mutex);
        if ((epoll_out = epoll_wait(epoll_sockets, events, 10, 500))== -1) {
            printf("Error: Epoll failed.\n");
            perror("epoll");
            continue;
        }
        pthread_mutex_unlock(&epoll_mutex);
        for (int i = 0; i < epoll_out; i++) {
            Epoll_data *data = events[i].data.ptr;

            if(data->event_type == SOCKET_EVENT){
//                printf("Socket event occured.\n");
                int new_fd;
                struct sockaddr* ptr_sockaddr;
                unsigned int addr_len = 0; //sizeof(struct sockaddr_in);

                if (data->client_data.client_type == INTERNET_CLIENT) {
                    addr_len = sizeof(struct sockaddr_in);
                }
                else if(data->client_data.client_type == LOCAL_CLIENT){
                    addr_len = sizeof(struct sockaddr_un);
                }

                ptr_sockaddr = malloc(addr_len);
                if ((new_fd = accept(data->client_data.socket_fd, ptr_sockaddr, &addr_len)) == -1) {
                    perror("accept");
                    continue;
                } else {
                    Thread_data *add_client_buf = malloc(sizeof (Thread_data));
                    add_client_buf->fd = new_fd;
                    add_client_buf->type = data->client_data.client_type;
//                    pthread_create(&pthread_add_client, NULL, add_client_routine, (void*)add_client_buf);
                    Message_data init_message;
//    printf("fd: %d\n", add_client_buf->fd);
                    int ret_recv = 0;
                    if ((ret_recv = recv(add_client_buf->fd, &init_message, sizeof(Message_data), 0)) == -1) {
                        perror("recv2");
                        continue;
                    }
                    if(init_message.message_type == INIT_CLIENT_MESS){
//        printf("Managing place...\n");
                        strncpy(add_client_buf->name, init_message.data.name, sizeof(init_message.data.name));
                        pthread_mutex_lock(&game_mutex);
                        manage_place_for_client(add_client_buf->fd, add_client_buf->type, add_client_buf->name);
                        pthread_mutex_unlock(&game_mutex);
                    }
//    printf("exit add_client_routine\n");
                    free(add_client_buf);
                }
                free(ptr_sockaddr);

            }
            else if(data->event_type == GAME_EVENT){
                printf("Game event occured.\n");
                if(events[i].events & EPOLLHUP){
                    printf("EPOLLHUP\n");
                    if(!delete_two_clients(data->client_data.socket_fd)){
                        printf("returned 0 continue\n");
                        continue;
                    }
                    printf("returned 1 contniued\n");
                    continue;
                }

                if (data->client_data.client_type == INTERNET_CLIENT) {
                    printf("internet client!!!!!!!!!!!!!!\n");
                }
                else if(data->client_data.client_type == LOCAL_CLIENT){
                    printf("local client!!!!!!!!\n");
                }
                Thread_data *thread_buf = malloc(sizeof (Thread_data));
                thread_buf->fd = data->client_data.socket_fd;
                thread_buf->type = data->client_data.client_type;
//                pthread_create(&pthread_menage_game, NULL, menage_game_routine, (void*)thread_buf);
                Message_data recv_message;
                int ret_recv = 0;
                printf("open recv\n");
                if ((ret_recv = recv(thread_buf->fd, &recv_message, sizeof(Message_data), 0)) == -1) {
                    perror("recv1");
                    continue;
                }
                if(recv_message.message_type == END_CLIENT_MESS){
                    if(!delete_two_clients(thread_buf->fd)){
                        continue;
                    }

                    printf("Client unregistered.\n");
                }
                else if(recv_message.message_type == MOVE_MESS){
                    char sign = recv_message.data.move.sign;
                    int place = recv_message.data.move.place - '1';
                    int board_id = -1;
//        int gamer_id = -1;
                    int second_fd = -1;
                    for(int i = 0; i< MAX_GAMES; i++){
                        if(games.board[i].client[0].socket_fd == thread_buf->fd){
                            board_id = i;
//                gamer_id = 0;
                            second_fd = games.board[i].client[1].socket_fd;
                            break;
                        }
                        else if(games.board[i].client[1].socket_fd == thread_buf->fd){
                            board_id = i;
//                gamer_id = 1;
                            second_fd = games.board[i].client[0].socket_fd;
                            break;
                        }
                    }

                    if(board_id != -1){
                        if(games.board[board_id].field[place] >= '1' && games.board[board_id].field[place]<= '9'){
                            games.board[board_id].field[place] = sign;
                        }
                        else{
                            printf("Error: Cannot place sign there.\n");
                            continue;
                        }

                        Message_data game_message1;
                        Message_data game_message2;
                        game_message1.message_type = GAME_STATE_MESS;
                        game_message2.message_type = GAME_STATE_MESS;

                        for(int i = 0; i<9; i++) {
                            game_message1.data.game_state.field[i] = games.board[board_id].field[i];
                            game_message2.data.game_state.field[i] = games.board[board_id].field[i];
                        }
                        game_message1.data.game_state.sign = sign;
                        if( game_message1.data.game_state.sign == 'X'){
                            game_message2.data.game_state.sign = 'O';
                        }
                        else{
                            game_message2.data.game_state.sign = 'X';
                        }
                        game_message1.data.game_state.turn = 0;
                        if(game_message1.data.game_state.turn){
                            game_message2.data.game_state.turn = 0;
                        }
                        else{
                            game_message2.data.game_state.turn = 1;
                        }

                        int winner = check_win(board_id);
                        if(winner == 1){
                            if(sign == 'X'){
                                game_message1.data.game_state.win = 1;
                                game_message2.data.game_state.win = 2;
                            }
                            else{
                                game_message1.data.game_state.win = 2;
                                game_message2.data.game_state.win = 1;
                            }
                        }
                        else if(winner == 2){
                            if(sign == 'O'){
                                game_message1.data.game_state.win = 1;
                                game_message2.data.game_state.win = 2;
                            }
                            else{
                                game_message1.data.game_state.win = 2;
                                game_message2.data.game_state.win = 1;
                            }
                        }
                        else if(check_draw(board_id)){
                            game_message1.data.game_state.win = 3;
                            game_message2.data.game_state.win = 3;
                        }
                        else{
                            game_message1.data.game_state.win = 0;
                            game_message2.data.game_state.win = 0;
                        }

                        if (send(thread_buf->fd, &game_message1, sizeof(Message_data), 0) == -1) {
                            perror("send");
                        }

                        if (send(second_fd, &game_message2, sizeof(Message_data), 0) == -1) {
                            perror("send");
                        }

                    }

                }
                else if(recv_message.message_type == PING_MESS){
                    printf("Got ping from %d.\n", thread_buf->fd);
                    pthread_mutex_lock(&ping_mutex);
                    printf("Cells               : ");
                    for(int i = 0; i<MAX_CLIENTS; i++){
                        printf("%d ", ping_cells[i]);
                    }
                    printf("\n");
                    for(int i = 0; i<MAX_CLIENTS; i++){
                        if(ping_cells[i] == thread_buf->fd){
                            ping_cells[i] = -1;
                            break;
                        }
                    }
                    printf("Cells   after            : ");
                    for(int i = 0; i<MAX_CLIENTS; i++){
                        printf("%d ", ping_cells[i]);
                    }
                    printf("\n");
                    pthread_mutex_unlock(&ping_mutex);
                }

                free(thread_buf);
            }
        }
        printf("usleep\n");
        usleep(500000);
    }

//    pthread_join(pthread_connection, NULL);
    pthread_join(pthread_ping, NULL);

    return 0;
}

void* provide_connection(void* arg){

//    struct epoll_event events[10];
//    int epoll_out= 0;
//
//    while(1){
//        printf("Epoll waiting... \n");
//        pthread_mutex_lock(&epoll_mutex);
//        if ((epoll_out = epoll_wait(epoll_sockets, events, 10, 500))== -1) {
//            printf("Error: Epoll failed.\n");
//            perror("epoll");
//            continue;
//        }
//        pthread_mutex_unlock(&epoll_mutex);
//        for (int i = 0; i < epoll_out; i++) {
//            Epoll_data *data = events[i].data.ptr;
//
//            if(data->event_type == SOCKET_EVENT){
////                printf("Socket event occured.\n");
//                int new_fd;
//                struct sockaddr* ptr_sockaddr;
//                unsigned int addr_len = 0; //sizeof(struct sockaddr_in);
//
//                if (data->client_data.client_type == INTERNET_CLIENT) {
//                    addr_len = sizeof(struct sockaddr_in);
//                }
//                else if(data->client_data.client_type == LOCAL_CLIENT){
//                    addr_len = sizeof(struct sockaddr_un);
//                }
//
//                ptr_sockaddr = malloc(addr_len);
//                if ((new_fd = accept(data->client_data.socket_fd, ptr_sockaddr, &addr_len)) == -1) {
//                    perror("accept");
//                    continue;
//                } else {
//                    Thread_data *add_client_buf = malloc(sizeof (Thread_data));
//                    add_client_buf->fd = new_fd;
//                    add_client_buf->type = data->client_data.client_type;
////                    pthread_create(&pthread_add_client, NULL, add_client_routine, (void*)add_client_buf);
//                    Message_data init_message;
////    printf("fd: %d\n", add_client_buf->fd);
//                    int ret_recv = 0;
//                    if ((ret_recv = recv(add_client_buf->fd, &init_message, sizeof(Message_data), 0)) == -1) {
//                        perror("recv2");
//                        continue;
//                    }
//                    if(init_message.message_type == INIT_CLIENT_MESS){
////        printf("Managing place...\n");
//                        strncpy(add_client_buf->name, init_message.data.name, sizeof(init_message.data.name));
//                        pthread_mutex_lock(&game_mutex);
//                        manage_place_for_client(add_client_buf->fd, add_client_buf->type, add_client_buf->name);
//                        pthread_mutex_unlock(&game_mutex);
//                    }
////    printf("exit add_client_routine\n");
//                    free(add_client_buf);
//                }
//                free(ptr_sockaddr);
//
//            }
//            else if(data->event_type == GAME_EVENT){
//                printf("Game event occured.\n");
//                if(events[i].events & EPOLLHUP){
//                    printf("EPOLLHUP\n");
//                    if(!delete_two_clients(data->client_data.socket_fd)){
//                        printf("returned 0 continue\n");
//                        continue;
//                    }
//                    printf("returned 1 contniued\n");
//                    continue;
//                }
//
//                if (data->client_data.client_type == INTERNET_CLIENT) {
//                    printf("internet client!!!!!!!!!!!!!!\n");
//                }
//                else if(data->client_data.client_type == LOCAL_CLIENT){
//                    printf("local client!!!!!!!!\n");
//                }
//                Thread_data *thread_buf = malloc(sizeof (Thread_data));
//                thread_buf->fd = data->client_data.socket_fd;
//                thread_buf->type = data->client_data.client_type;
////                pthread_create(&pthread_menage_game, NULL, menage_game_routine, (void*)thread_buf);
//                Message_data recv_message;
//                int ret_recv = 0;
//                printf("open recv\n");
//                if ((ret_recv = recv(thread_buf->fd, &recv_message, sizeof(Message_data), 0)) == -1) {
//                    perror("recv1");
//                    continue;
//                }
//                if(recv_message.message_type == END_CLIENT_MESS){
//                    if(!delete_two_clients(thread_buf->fd)){
//                        continue;
//                    }
//
//                    printf("Client unregistered.\n");
//                }
//                else if(recv_message.message_type == MOVE_MESS){
//                    char sign = recv_message.data.move.sign;
//                    int place = recv_message.data.move.place - '1';
//                    int board_id = -1;
////        int gamer_id = -1;
//                    int second_fd = -1;
//                    for(int i = 0; i< MAX_GAMES; i++){
//                        if(games.board[i].client[0].socket_fd == thread_buf->fd){
//                            board_id = i;
////                gamer_id = 0;
//                            second_fd = games.board[i].client[1].socket_fd;
//                            break;
//                        }
//                        else if(games.board[i].client[1].socket_fd == thread_buf->fd){
//                            board_id = i;
////                gamer_id = 1;
//                            second_fd = games.board[i].client[0].socket_fd;
//                            break;
//                        }
//                    }
//
//                    if(board_id != -1){
//                        if(games.board[board_id].field[place] >= '1' && games.board[board_id].field[place]<= '9'){
//                            games.board[board_id].field[place] = sign;
//                        }
//                        else{
//                            printf("Error: Cannot place sign there.\n");
//                            continue;
//                        }
//
//                        Message_data game_message1;
//                        Message_data game_message2;
//                        game_message1.message_type = GAME_STATE_MESS;
//                        game_message2.message_type = GAME_STATE_MESS;
//
//                        for(int i = 0; i<9; i++) {
//                            game_message1.data.game_state.field[i] = games.board[board_id].field[i];
//                            game_message2.data.game_state.field[i] = games.board[board_id].field[i];
//                        }
//                        game_message1.data.game_state.sign = sign;
//                        if( game_message1.data.game_state.sign == 'X'){
//                            game_message2.data.game_state.sign = 'O';
//                        }
//                        else{
//                            game_message2.data.game_state.sign = 'X';
//                        }
//                        game_message1.data.game_state.turn = 0;
//                        if(game_message1.data.game_state.turn){
//                            game_message2.data.game_state.turn = 0;
//                        }
//                        else{
//                            game_message2.data.game_state.turn = 1;
//                        }
//
//                        int winner = check_win(board_id);
//                        if(winner == 1){
//                            if(sign == 'X'){
//                                game_message1.data.game_state.win = 1;
//                                game_message2.data.game_state.win = 2;
//                            }
//                            else{
//                                game_message1.data.game_state.win = 2;
//                                game_message2.data.game_state.win = 1;
//                            }
//                        }
//                        else if(winner == 2){
//                            if(sign == 'O'){
//                                game_message1.data.game_state.win = 1;
//                                game_message2.data.game_state.win = 2;
//                            }
//                            else{
//                                game_message1.data.game_state.win = 2;
//                                game_message2.data.game_state.win = 1;
//                            }
//                        }
//                        else if(check_draw(board_id)){
//                            game_message1.data.game_state.win = 3;
//                            game_message2.data.game_state.win = 3;
//                        }
//                        else{
//                            game_message1.data.game_state.win = 0;
//                            game_message2.data.game_state.win = 0;
//                        }
//
//                        if (send(thread_buf->fd, &game_message1, sizeof(Message_data), 0) == -1) {
//                            perror("send");
//                        }
//
//                        if (send(second_fd, &game_message2, sizeof(Message_data), 0) == -1) {
//                            perror("send");
//                        }
//
//                    }
//
//                }
//                else if(recv_message.message_type == PING_MESS){
//                    printf("Got ping from %d.\n", thread_buf->fd);
//                    pthread_mutex_lock(&ping_mutex);
//                    printf("Cells               : ");
//                    for(int i = 0; i<MAX_CLIENTS; i++){
//                        printf("%d ", ping_cells[i]);
//                    }
//                    printf("\n");
//                    for(int i = 0; i<MAX_CLIENTS; i++){
//                        if(ping_cells[i] == thread_buf->fd){
//                            ping_cells[i] = -1;
//                            break;
//                        }
//                    }
//                    printf("Cells   after            : ");
//                    for(int i = 0; i<MAX_CLIENTS; i++){
//                        printf("%d ", ping_cells[i]);
//                    }
//                    printf("\n");
//                    pthread_mutex_unlock(&ping_mutex);
//                }
//
//                free(thread_buf);
//            }
//        }
//        printf("usleep\n");
//        usleep(500000);
//    }
    pthread_exit(0);
}

int delete_two_clients(int socket_fd){
    printf("Deleting two clients!\n");
    int second_fd = -1;
    Message_data end_message;
    end_message.message_type = END_MESS;
    printf("up to enter game mutex\n");
    pthread_mutex_lock(&game_mutex);
    printf("inside game mutex\n");
    printf("mutex locked\n");
    int exist_fd = -1;
    for(int j = 0; j<MAX_GAMES; j++){
        if (games.board[j].client[0].socket_fd == socket_fd) {
            exist_fd = 1;
            break;
        } else if (games.board[j].client[1].socket_fd == socket_fd) {
            exist_fd = 1;
            break;
        }
    }
    if(exist_fd == -1) {
        printf("Cannot find descriptor\n");
        pthread_mutex_unlock(&game_mutex);
        return 0;
    }
    printf("Descriptor found\n");

    for(int j = 0; j<MAX_GAMES; j++){
        if (games.board[j].client[0].socket_fd == socket_fd) {
            if(games.board[j].client[1].socket_fd != -1){
                second_fd = games.board[j].client[1].socket_fd;
                printf("sending message to second\n");
                if (send(second_fd, &end_message, sizeof(Message_data), 0) == -1) {
                    perror("send");
                }
                printf("sent\n");
                break;
            }

        } else if (games.board[j].client[1].socket_fd == socket_fd) {
            if(games.board[j].client[0].socket_fd != -1){
                second_fd = games.board[j].client[0].socket_fd;
                printf("sending message to second\n");
                if (send(second_fd, &end_message, sizeof(Message_data), 0) == -1) {
                    perror("send");
                }
                printf("sent\n");
                break;
            }
        }
    }
    pthread_mutex_unlock(&game_mutex);
    printf("deleteing first\n");
    delete_client(socket_fd);
    if(second_fd != -1){
        printf("deleteing second\n");
        delete_client(second_fd);
    }
    printf("end\n");
    return 1;
}

void* menage_game_routine(void* arg){
    printf("manage_game_routine\n");
    Thread_data *thread_buf = (Thread_data*) arg;
    Message_data recv_message;
    int ret_recv = 0;
    printf("open recv\n");
    if ((ret_recv = recv(thread_buf->fd, &recv_message, sizeof(Message_data), 0)) == -1) {
        perror("recv1");
        free(thread_buf);
        pthread_exit(0);
    }
    if(recv_message.message_type == END_CLIENT_MESS){
        if(!delete_two_clients(thread_buf->fd)){
            free(thread_buf);
            pthread_exit(0);
        }

        printf("Client unregistered.\n");
    }
    else if(recv_message.message_type == MOVE_MESS){
        char sign = recv_message.data.move.sign;
        int place = recv_message.data.move.place - '1';
        int board_id = -1;
//        int gamer_id = -1;
        int second_fd = -1;
        for(int i = 0; i< MAX_GAMES; i++){
            if(games.board[i].client[0].socket_fd == thread_buf->fd){
                board_id = i;
//                gamer_id = 0;
                second_fd = games.board[i].client[1].socket_fd;
                break;
            }
            else if(games.board[i].client[1].socket_fd == thread_buf->fd){
                board_id = i;
//                gamer_id = 1;
                second_fd = games.board[i].client[0].socket_fd;
                break;
            }
        }

        if(board_id != -1){
            if(games.board[board_id].field[place] >= '1' && games.board[board_id].field[place]<= '9'){
                games.board[board_id].field[place] = sign;
            }
            else{
                printf("Error: Cannot place sign there.\n");
                free(thread_buf);
                pthread_exit(0);
            }

            Message_data game_message1;
            Message_data game_message2;
            game_message1.message_type = GAME_STATE_MESS;
            game_message2.message_type = GAME_STATE_MESS;

            for(int i = 0; i<9; i++) {
                game_message1.data.game_state.field[i] = games.board[board_id].field[i];
                game_message2.data.game_state.field[i] = games.board[board_id].field[i];
            }
            game_message1.data.game_state.sign = sign;
            if( game_message1.data.game_state.sign == 'X'){
                game_message2.data.game_state.sign = 'O';
            }
            else{
                game_message2.data.game_state.sign = 'X';
            }
            game_message1.data.game_state.turn = 0;
            if(game_message1.data.game_state.turn){
                game_message2.data.game_state.turn = 0;
            }
            else{
                game_message2.data.game_state.turn = 1;
            }

            int winner = check_win(board_id);
            if(winner == 1){
                if(sign == 'X'){
                    game_message1.data.game_state.win = 1;
                    game_message2.data.game_state.win = 2;
                }
                else{
                    game_message1.data.game_state.win = 2;
                    game_message2.data.game_state.win = 1;
                }
            }
            else if(winner == 2){
                if(sign == 'O'){
                    game_message1.data.game_state.win = 1;
                    game_message2.data.game_state.win = 2;
                }
                else{
                    game_message1.data.game_state.win = 2;
                    game_message2.data.game_state.win = 1;
                }
            }
            else if(check_draw(board_id)){
                game_message1.data.game_state.win = 3;
                game_message2.data.game_state.win = 3;
            }
            else{
                game_message1.data.game_state.win = 0;
                game_message2.data.game_state.win = 0;
            }

            if (send(thread_buf->fd, &game_message1, sizeof(Message_data), 0) == -1) {
                perror("send");
            }

            if (send(second_fd, &game_message2, sizeof(Message_data), 0) == -1) {
                perror("send");
            }

        }

    }
    else if(recv_message.message_type == PING_MESS){
        printf("Got ping from %d.\n", thread_buf->fd);
        pthread_mutex_lock(&ping_mutex);
        printf("Cells               : ");
        for(int i = 0; i<MAX_CLIENTS; i++){
            printf("%d ", ping_cells[i]);
        }
        printf("\n");
        for(int i = 0; i<MAX_CLIENTS; i++){
            if(ping_cells[i] == thread_buf->fd){
                ping_cells[i] = -1;
                break;
            }
        }
        printf("Cells   after            : ");
        for(int i = 0; i<MAX_CLIENTS; i++){
            printf("%d ", ping_cells[i]);
        }
        printf("\n");
        pthread_mutex_unlock(&ping_mutex);
    }

    free(thread_buf);
    pthread_exit(0);
}

int check_draw(int board_id){
    for(int i =0 ; i<9; i++){
        if(games.board[board_id].field[i] != 'X' && games.board[board_id].field[i] != 'O'){
            return 0;
        }
    }
    return 1;
}

int check_win(int board_id){
    printf("check_win\n");
    int ret_val = -1;
    if((ret_val = check_three(board_id, 0, 1, 2)) != 0)
    {
        return ret_val;
    }
    else if((ret_val = check_three(board_id, 3, 4, 5)) != 0)
    {
        return ret_val;
    }
    else if((ret_val = check_three(board_id, 6, 7, 8)) != 0)
    {
        return ret_val;
    }
    else if((ret_val = check_three(board_id, 0, 4, 8)) != 0)
    {
        return ret_val;
    }
    else if((ret_val = check_three(board_id, 2, 4, 6)) != 0)
    {
        return ret_val;
    }
    else if((ret_val = check_three(board_id, 0, 3, 6)) != 0)
    {
        return ret_val;
    }
    else if((ret_val = check_three(board_id, 1, 4, 7)) != 0)
    {
        return ret_val;
    }
    else if((ret_val = check_three(board_id, 2, 5, 8)) != 0)
    {
        return ret_val;
    }
    return 0;
}

int check_three(int board_id, int id1, int id2, int id3){
    printf("check_three\n");
    if(games.board[board_id].field[id1] == games.board[board_id].field[id2] &&
       games.board[board_id].field[id1] == games.board[board_id].field[id3])
    {
        if(games.board[board_id].field[id1] == 'X'){
            return 1;
        }
        else{
            return 2;
        }
    }
    return 0;
}

void* add_client_routine(void* arg){
//    printf("add_client_routine\n");
//    Thread_data *add_client_buf = (Thread_data*) arg;
//    Message_data init_message;
////    printf("fd: %d\n", add_client_buf->fd);
//    int ret_recv = 0;
//    if ((ret_recv = recv(add_client_buf->fd, &init_message, sizeof(Message_data), 0)) == -1) {
//        perror("recv2");
//        free(add_client_buf);
//        pthread_exit(0);
//    }
//    if(init_message.message_type == INIT_CLIENT_MESS){
////        printf("Managing place...\n");
//        strncpy(add_client_buf->name, init_message.data.name, sizeof(init_message.data.name));
//        pthread_mutex_lock(&game_mutex);
//        manage_place_for_client(add_client_buf->fd, add_client_buf->type, add_client_buf->name);
//        pthread_mutex_unlock(&game_mutex);
//    }
////    printf("exit add_client_routine\n");
//    free(add_client_buf);
//    pthread_exit(0);
}

void delete_client(int sock_fd){
    printf("delete_client\n");
    pthread_mutex_lock(&game_mutex);
    int tmp_fd = -1;
    int board_id = -1;
    int client_id = -1;

    for (int j = 0; j < MAX_GAMES; j++) {
        if (games.board[j].client[0].socket_fd == sock_fd) {
            tmp_fd = sock_fd;
            board_id = j;
            client_id = 0;
            break;
        } else if(games.board[j].client[1].socket_fd == sock_fd) {
            tmp_fd = sock_fd;
            board_id = j;
            client_id = 1;
            break;
        }
    }
    if(tmp_fd == -1){
        printf("Error: Cannot delete client.\n");
    }
    else{
        games.board[board_id].client[client_id].socket_fd = -1;
        games.board[board_id].client[client_id].client_type = NONE_CLIENT;
        games.board[board_id].client[client_id].name[0] = '\0';
        pthread_mutex_lock(&epoll_mutex);
        epoll_ctl(epoll_sockets, EPOLL_CTL_DEL, tmp_fd, NULL);
        pthread_mutex_unlock(&epoll_mutex);
//        if(games.board[board_id].client[client_id].ptr != NULL){
//            free(games.board[board_id].client[client_id].ptr);
//            games.board[board_id].client[client_id].ptr = NULL;
//        }
        close(tmp_fd);
        printf("Client deleted: %d\n", sock_fd);
    }
    pthread_mutex_unlock(&game_mutex);
}

void manage_place_for_client(int new_fd, Client_type type, char* name){

    printf("managing place for client %s\n", name);
    int board_id = -1;
    int client_id = -1;
    int name_error = 0;
    for(int j = 0; j<MAX_GAMES; j++){
        if(games.board[j].client[0].socket_fd != -1 && strcmp(games.board[j].client[0].name, name) == 0){
            name_error = 1;
            break;
        }
        if(games.board[j].client[1].socket_fd != -1 && strcmp(games.board[j].client[1].name, name) == 0){
            name_error = 1;
            break;
        }
    }

    if(name_error){
        printf("Error: There is already a client with such a name.\n");
        Message_data name_message;
        name_message.message_type = NAME_MESS;
        if (send(new_fd, &name_message, sizeof(Message_data), 0) == -1) {
            perror("send");
        }
//        printf("message has been sent\n");
        return;
    }

    for (int j = 0; j < MAX_GAMES; j++) {
//        printf("j: %d\n", j);
//        printf("address: %p\n", (void*)&games);
//        printf("fd: %d\n", games.board[j].client[0].socket_fd);
        if (games.board[j].client[0].socket_fd == -1) {
            board_id = j;
            client_id = 0;
            break;
        } else if(games.board[j].client[1].socket_fd == -1) {
            board_id = j;
            client_id = 1;
            break;
        }
    }

    if(board_id == -1){

        printf("Error: Board is full.\n");

        Message_data full_message;
        full_message.message_type = FULL_MESS;
        if (send(new_fd, &full_message, sizeof(Message_data), 0) == -1) {
            perror("send");
        }
    }
    else {
//        printf("creating epoll...\n");
//        printf("Client id: %d\n", client_id);
//        printf("Board id: %d\n", board_id);
        strncpy(games.board[board_id].client[client_id].name, name, strlen(name));
        struct epoll_event event_inner = {.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLPRI};
//        printf("dziala\n");
        Epoll_data *data_inner = event_inner.data.ptr = malloc(sizeof(Epoll_data));
//        printf("Epoll_data size: %ld !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", sizeof (Epoll_data));
//        printf("dziala\n");
        data_inner->client_data.client_type = games.board[board_id].client[client_id].client_type = type;
//        printf("dziala\n");
        data_inner->client_data.socket_fd = games.board[board_id].client[client_id].socket_fd = new_fd;
//        printf("dziala\n");
        data_inner->event_type = GAME_EVENT;
//        printf("dziala\n");
        games.board[board_id].client[client_id].ptr = data_inner;

        pthread_mutex_lock(&epoll_mutex);
        epoll_ctl(epoll_sockets, EPOLL_CTL_ADD, new_fd, &event_inner);
        pthread_mutex_unlock(&epoll_mutex);

        printf("Client accepted!\n");

        Message_data ok_message;
        ok_message.message_type = OK_MESS;
        if (send(new_fd, &ok_message, sizeof(Message_data), 0) == -1) {
            perror("send");
        }

        create_game(board_id);

    }
}

void create_game(int board_id){
    printf("create_gaem\n");
    if(games.board[board_id].client[0].socket_fd == -1 ||
       games.board[board_id].client[1].socket_fd == -1){
        return;
    }

    for(int j = 0; j<9; j++){
        games.board[board_id].field[j] = '1'+j;
    }

    Message_data game_message1;
    Message_data game_message2;
    game_message1.message_type = GAME_STATE_MESS;
    game_message2.message_type = GAME_STATE_MESS;
    for(int i = 0; i<9; i++) {
        game_message1.data.game_state.field[i] = '1'+i;
        game_message2.data.game_state.field[i] = '1'+i;
    }
    game_message1.data.game_state.sign = rand()%2 ? 'X' : 'O';
    if( game_message1.data.game_state.sign == 'X'){
        game_message2.data.game_state.sign = 'O';
    }
    else{
        game_message2.data.game_state.sign = 'X';
    }
    game_message1.data.game_state.turn = rand()%2;
    if(game_message1.data.game_state.turn){
        game_message2.data.game_state.turn = 0;
    }
    else{
        game_message2.data.game_state.turn = 1;
    }
    game_message1.data.game_state.win = 0;
    game_message2.data.game_state.win = 0;

    if (send(games.board[board_id].client[0].socket_fd, &game_message1, sizeof(Message_data), 0) == -1) {
        perror("send");
    }

    if (send(games.board[board_id].client[1].socket_fd, &game_message2, sizeof(Message_data), 0) == -1) {
        perror("send");
    }


}

void* ping_routine(void* arg) {
    int amt_cells = 0;
    pthread_mutex_lock(&ping_mutex);
    for(int i = 0; i<MAX_CLIENTS; i++){
        ping_cells[i] = -1;
    }
    pthread_mutex_unlock(&ping_mutex);

    while(1) {

//        printf("ping routine up to lock\n");
        pthread_mutex_lock(&game_mutex);
        pthread_mutex_lock(&ping_mutex);
//        printf("ping routine locked\n");
        amt_cells = 0;
        for(int i = 0; i<MAX_CLIENTS; i++){
            ping_cells[i] = -1;
        }
        for (int j = 0; j < MAX_GAMES; j++) {
            if (games.board[j].client[0].socket_fd != -1) {
                printf("PING sent to %d.\n", games.board[j].client[0].socket_fd );
                Message_data ping_message;
                ping_message.message_type = PING_MESS;
                ping_cells[amt_cells++] = games.board[j].client[0].socket_fd;
                if (send(games.board[j].client[0].socket_fd, &ping_message, sizeof(Message_data), 0) == -1) {
                    perror("send");
                }
            }
            if(games.board[j].client[1].socket_fd != -1) {
                printf("PING sent to %d.\n", games.board[j].client[1].socket_fd );
                Message_data ping_message;
                ping_message.message_type = PING_MESS;
                ping_cells[amt_cells++] = games.board[j].client[1].socket_fd;
                if (send(games.board[j].client[1].socket_fd, &ping_message, sizeof(Message_data), 0) == -1) {
                    perror("send");
                }
            }
        }
        pthread_mutex_unlock(&ping_mutex);
        pthread_mutex_unlock(&game_mutex);
//        printf("SLEEP\n");
        sleep(5);

        pthread_mutex_lock(&ping_mutex);
        printf("Cells               : ");
        for(int i = 0; i<amt_cells; i++){
            printf("%d ", ping_cells[i]);
        }
        printf("\n");
        for(int i = 0; i<amt_cells; i++){
            if(ping_cells[i] != -1){
                printf("ping not received...\n");
                delete_two_clients(ping_cells[i]);
            }
        }
        pthread_mutex_unlock(&ping_mutex);

    }
    pthread_exit(0);
}