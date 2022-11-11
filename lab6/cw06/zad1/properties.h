

#define SERVER_Q_NO 1234
#define AMOUNT_OF_QUEUES 20

#define _STOP 1
#define _INIT 3
#define _LIST 2
#define _2ALL 4
#define _2ONE 5
#define _ABSMIN -10

#define MSGLEN 256

struct msgbuf {
    long mtype;
    char mtext[MSGLEN];
    int client_id;
};