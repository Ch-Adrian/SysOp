#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <sys/file.h>
#include <stdlib.h>

void check_args(int, char**);

int main(int argc, char** argv){

    check_args(argc,argv);

    int N = atoi(argv[4]);
    char* cpy_buf = calloc(N+strlen(argv[2]) +2, sizeof(char));

    int file = open(argv[3], O_RDONLY);
    if(file <0){
        printf("Error: Cannot open file.\n");
        exit(1);
    }

    int fd = open(argv[1], O_WRONLY);
    if(fd < 0){
        printf("Error: Cannot open fifo file.\n");
        exit(1);
    }
    char* buffer = calloc(N+1, sizeof(char));
    int no_chars = 1;

    while( (no_chars = read(file, buffer, N)) != 0){

        if(no_chars == 0){
            break;
        }
        strcpy(cpy_buf, argv[2]);
        strcat(cpy_buf, ".");

        if(no_chars <= N){
            int i = no_chars;
            while ( i<=N){
                buffer[i] = '\0';
                i++;
            }
        }

        strncat(cpy_buf, buffer, N);
        write(fd, cpy_buf, strlen(cpy_buf));
        int sleep_time = 1+ rand()%2;
        sleep(sleep_time);
    }


    free(buffer);
    close(fd);
    close(file);
    free(cpy_buf);
    return 0;
}

void check_args(int argc, char** argv){

    if(argc != 5){
        printf("Error: Invalid amount of arguments.\n");
        exit(0);
    }

    for(int i = 0; i < strlen(argv[2]); i++){
        if( argv[2][i] < '0' || argv[2][i] > '9'){
            printf("Error: Second argument should be a positive integer.\n");
            exit(0);
        }
    }


    FILE* f = fopen(argv[3], "r");
    if(f == NULL){
        printf("Error: Cannot open file.\n");
        exit(0);
    }
    fclose(f);

    for(int i = 0; i < strlen(argv[4]); i++){
        if( argv[4][i] < '0' || argv[4][i] > '9'){
            printf("Error: Second argument should be a positive integer.\n");
            exit(0);
        }
    }

}