#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/file.h>
#include <time.h>

void check_args(int, char**);

int read_fifo(int fd, char* buffer, int N){
    char* buf_num = calloc(2, sizeof(char));
    int cnt = 0;
    int line = 0;
    while( (cnt = read(fd,buf_num, sizeof(char))) != 0){
        buf_num[1] = '\0';
        if(buf_num[0] != '.' && (buf_num[0] < '0' || buf_num[0] > '9')){
            free(buf_num);
            return -1;
        }
        if(cnt == 0) {
            free(buf_num);
            return -1;
        }

        if(buf_num[0] == '.') break;
        line = 10*line + (buf_num[0] - '0');

    }
    if(cnt == 0) {
        free(buf_num);
        return -1;
    }

    cnt = read(fd, buffer, N);
    if(cnt == 0){
        free(buf_num);
        return -1;
    }

    free(buf_num);

    return line;
}

void write_file(int fd, char* buffer, int N, int line){

    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* file_buffer = calloc(size, sizeof(char));
    int size_file_buffer_end =size + N+line+1 ;
    char* file_buffer_end = calloc(size_file_buffer_end, sizeof(char));
    int amt_chars = read(fd, file_buffer, size);
    if(amt_chars > 0 && file_buffer[amt_chars-1] == '\0'){
        int tmp = amt_chars-1;
        while( tmp>0 && file_buffer[tmp] == '\0') {
            size -= 1;
            size_file_buffer_end -= 1;
            tmp-=1;
        }
    }
    char * line_position = file_buffer;
    char* ptr = file_buffer;
    int amt_of_lines = 0;
    for(int i = 1; 1; i++){
        line_position = strchr(line_position, '\n');
        if(line_position == NULL){
            break;
        }
        else{
            amt_of_lines++;
        }
        line_position++;
        if(line_position - file_buffer >= size-1) break;
    }

    if(amt_of_lines == line){
        ptr = strrchr(file_buffer, '\n');
        strncpy(file_buffer_end, file_buffer, ptr - file_buffer);
        strncat(file_buffer_end, buffer, N);
        strcat(file_buffer_end, "\n");
    }
    else if( amt_of_lines < line){

        strcpy(file_buffer_end, file_buffer);
        int diff = line - amt_of_lines - 1;
        for(int i = 0; i<diff; i++){
            strcat(file_buffer_end, "\n");
        }
        strncat(file_buffer_end, buffer, N);
        strcat(file_buffer_end, "\n");
    }
    else{
        ptr = file_buffer;
        for(int i = 0; i<line; i++){
            ptr = strchr(ptr, '\n');
            if( i != line-1) ptr++;
        }
        strncpy(file_buffer_end, file_buffer, ptr - file_buffer);
        strncat(file_buffer_end, buffer,N);
        strcat(file_buffer_end, ptr);
    }

    lseek(fd, 0, SEEK_SET);
    write(fd, file_buffer_end, strlen(file_buffer_end)+1);

    free(file_buffer);
    free(file_buffer_end);
}

int main(int argc, char** argv){

    check_args(argc, argv);

    int N = atoi(argv[3]);
    int fd = open(argv[1], O_RDONLY);
    if(fd == 0){
        printf("Error: Cannot open fifo file.\n");
    }

    char* buffer = calloc(N+1, sizeof(char));

    int file = open(argv[2], O_RDWR | O_CREAT);
    if(file == 0){
        printf("Error: Cannot open file.\n");
    }

    int line = 0;
    while (line != -1) {
        line = read_fifo(fd, buffer, N);
        if (line == -1) break;

        flock(file, LOCK_EX);
        write_file(file, buffer, N, line);
        flock(file, LOCK_UN);

    }


    close(file);
    free(buffer);
    close(fd);

    return 0;
}

void check_args(int argc, char** argv){

    if(argc != 4){
        printf("Error: Invalid amount of arguments.\n");
        exit(0);
    }

    FILE* f = fopen(argv[2], "r");
    if(f == NULL){
        printf("Error: Cannot open file.\n");
        exit(0);
    }
    fclose(f);

    for(int i = 0; i < strlen(argv[3]); i++){
        if( argv[3][i] < '0' || argv[3][i] > '9'){
            printf("Error: Second argument should be a positive integer.\n");
            exit(0);
        }
    }
}