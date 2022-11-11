#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int amt_cmd = 32;
int amt_pipe = 32;
int max_cmd_in_pipe = 8;
int file_size = 0;

void set_variables(char*);
void check_arg(int, char**);
void f_exec_pipe(char***);
void f_exec_pipe2(char***);
void exec_wrapper(char* cmd, char** args);

int main(int argv, char** argc){

    check_arg(argv, argc);

    set_variables(argc[1]);
    int ele_pos = 0;
    char** element_array = calloc(amt_cmd, sizeof(char*));
    int pipe_pos = 0;
    char** pipe_array = calloc(amt_pipe, sizeof(char*));

    int command_pos = 0;
    char** command = calloc(amt_cmd, sizeof(char*));
    int *pipe_command_pos = malloc(amt_cmd*sizeof(int));
    char**** pipe_command = calloc(amt_cmd, sizeof(char***));
    for(int i = 0; i<amt_cmd; i++){
        pipe_command[i] = calloc(max_cmd_in_pipe, sizeof(char**));
    }

    char*** exec_pipe = calloc(amt_pipe, sizeof(char**));
    for(int i = 0; i<amt_pipe; i++){
        exec_pipe[i] = calloc(max_cmd_in_pipe, sizeof(char*));
    }

    char**** exec_pipe_final = calloc(amt_pipe, sizeof(char***));
    for(int i = 0; i<amt_pipe; i++){
        exec_pipe_final[i] = calloc(max_cmd_in_pipe*max_cmd_in_pipe, sizeof(char**));
    }

    FILE* file = fopen(argc[1], "r");

    fseek(file, 0L, SEEK_END);
    file_size = ftell(file)+1;
    fseek(file, 0L, SEEK_SET);

    char* buffer = calloc(file_size ,sizeof(char));
    int f = fread(buffer, sizeof(char), file_size-1, file);
    if(f<0){
        printf("Error: Cannot read file.\n");
        exit(0);
    }
    buffer[file_size-1] = '\0';

    char* end = strchr(buffer, '\n');
    char* end2= end;
    char* begin = buffer;
    char* equal_ptr = strchr(begin, '=');
    while( (end = strchr(begin, '\n')) != NULL){
        equal_ptr = strchr(begin, '=');
        if(equal_ptr != NULL){
            element_array[ele_pos] = calloc(end - begin+1, sizeof (char));
            strncpy(element_array[ele_pos], begin,end - begin);
            element_array[ele_pos][end-begin] = '\0';
            ele_pos++;
        }
        else if(end - begin > 1){
            pipe_array[pipe_pos] = calloc(end - begin+1, sizeof (char));
            strncpy(pipe_array[pipe_pos], begin, end-begin);
            pipe_array[pipe_pos][end-begin] = '\0';
            pipe_pos++;
        }
        begin = ++end;
        end2 = end;
    }

    if(end2 - buffer < file_size - 2){
        begin = end2;
        end = strchr(begin , '\0');
        equal_ptr = strchr(end, '=');
        if(equal_ptr != NULL){
            element_array[ele_pos] = calloc(end - begin+1, sizeof (char));
            strncpy(element_array[ele_pos], begin,end - begin);
            element_array[ele_pos][end-begin] = '\0';
            ele_pos++;
        }
        else{
            pipe_array[pipe_pos] = calloc(end - begin+1, sizeof (char));
            strncpy(pipe_array[pipe_pos], begin, end-begin);
            pipe_array[pipe_pos][end-begin] = '\0';
            pipe_pos++;
        }
    }

    for(int i = 0; i<ele_pos; i++){
        begin = element_array[i];
        if( (end = strchr(begin, ' ')) != NULL){
            command[command_pos] = calloc(end-begin+1, sizeof (char));
            strncpy(command[command_pos], begin, end-begin);
            command[command_pos][end-begin] = '\0';
            command_pos++;
            end++;
        }
        begin = strchr(end, ' ');
        int cnt = 0;
        pipe_command[i][cnt]= calloc(max_cmd_in_pipe, sizeof(char*));
        int innerCnt = 0;
        char* innerBegin = strchr(begin, ' ');
        char* innerEnd = strchr(++innerBegin, ' ');
        if(innerEnd == NULL){
            innerEnd = strchr(innerBegin, '\0');
        }
        while( (end = strchr(++begin, '|')) != NULL){
            while(innerEnd < end){
                pipe_command[i][cnt][innerCnt] = calloc(innerEnd - innerBegin+1, sizeof (char));
                pipe_command_pos[i] = cnt;
                strncpy(pipe_command[i][cnt][innerCnt], innerBegin, innerEnd - innerBegin);
                pipe_command[i][cnt][innerCnt][innerEnd-innerBegin] = '\0';
                innerCnt++;
                innerBegin = ++innerEnd;
                innerEnd = strchr(innerBegin, ' ');
            }
            begin = ++end;
            cnt++;
            pipe_command[i][cnt]= calloc(max_cmd_in_pipe, sizeof(char*));
            innerBegin = strchr(begin, ' ');
            innerEnd = strchr(++innerBegin, ' ');
        }

        end = strchr(element_array[i], '\0');
        innerCnt = 0;
        while(innerEnd <= end){
            pipe_command[i][cnt][innerCnt] = calloc(innerEnd - innerBegin+1, sizeof (char));
            pipe_command_pos[i] = cnt;
            strncpy(pipe_command[i][cnt][innerCnt], innerBegin, innerEnd - innerBegin);
            pipe_command[i][cnt][innerCnt][innerEnd-innerBegin] = '\0';
            innerCnt++;
            if(*innerEnd == '\0'){
                break;
            }
            innerBegin = ++innerEnd;
            innerEnd = strchr(innerBegin, ' ');
            if(innerEnd == NULL){
                innerEnd = strchr(innerBegin, '\0');
            }
        }
    }

    for(int i = 0; i<pipe_pos; i++){
        begin = pipe_array[i];
        end = strchr(begin, '\0');
        char* innerBegin = begin;
        char* innerEnd = strchr(begin, ' ');
        if(innerEnd == NULL){
            innerEnd = strchr(innerBegin, '\0');
        }
        int cnt = 0;
        while(innerEnd <= end){
            exec_pipe[i][cnt] = calloc(innerEnd - innerBegin +1, sizeof (char));
            strncpy(exec_pipe[i][cnt], innerBegin, innerEnd- innerBegin);
            exec_pipe[i][cnt][innerEnd - innerBegin] = '\0';
            if(*innerEnd == '\0') break;
            innerBegin = ++innerEnd;
            if(innerBegin[0] == '|'){
                innerBegin++;
                innerBegin++;
            }
            innerEnd = strchr(innerBegin, ' ');
            if(innerEnd == NULL){
                innerEnd = strchr(innerBegin, '\0');
            }
            cnt++;
        }
    }

    for(int i= 0; i<pipe_pos; i++){
        int j = 0;
        int outerCnt = 0;
        while(j<32 && exec_pipe[i][j] != NULL){
            for(int k = 0; k< command_pos; k++){
                if(strcmp(command[k] , exec_pipe[i][j]) == 0){
                    int innerCnt = 0;
                    while(innerCnt < 32 && pipe_command_pos[k] >= innerCnt){
                        exec_pipe_final[i][outerCnt] = pipe_command[k][innerCnt];
                        outerCnt++;
                        innerCnt++;
                    }
                    break;
                }
            }
            j++;
        }
    }



    for(int i =0; i<pipe_pos; i++){
        printf("Pipe: %d\n", i);
        f_exec_pipe(exec_pipe_final[i]);
        printf("\n");
    }

    fclose(file);


    for(int i = 0; i<amt_cmd; i++){
        int j = 0;
        while(j < max_cmd_in_pipe && pipe_command[i][j] != NULL){
            int k = 0;
            while( k < max_cmd_in_pipe && pipe_command[i][j][k] != NULL){
                free(pipe_command[i][j][k++]);
            }
            free(pipe_command[i][j]);
            j++;
        }
        free(pipe_command[i]);
    }
    free(pipe_command);
    for(int i = 0; i<amt_pipe; i++){
        free(exec_pipe_final[i]);
    }
    free(exec_pipe_final);
    for(int i = 0; i<amt_pipe; i++){
        for(int j = 0; j<max_cmd_in_pipe; j++){
            if(exec_pipe[i][j] != NULL){
                free(exec_pipe[i][j]);
            }
        }
        free(exec_pipe[i]);
    }
    free(exec_pipe);
    for(int i =0; i<command_pos; i++){
        free(command[i]);
    }
    free(command);
    free(buffer);
    for(int i = 0; i<ele_pos; i++){
        free(element_array[i]);
    }
    free(element_array);
    for(int i = 0; i<pipe_pos; i++){
        free(pipe_array[i]);
    }
    free(pipe_array);
    free(pipe_command_pos);
    return 0;
}

void set_variables(char* name_file){

    int amt_equal = 0;
    int amt_end_line = 0;
    FILE* tmp_file = fopen(name_file, "r");

    fseek(tmp_file, 0L, SEEK_END);
    file_size = ftell(tmp_file)+1;
    fseek(tmp_file, 0L, SEEK_SET);

    char* tmp_buffer = calloc(file_size ,sizeof(char));

    int tmp_f = fread(tmp_buffer, sizeof(char), file_size-1, tmp_file);
    if(tmp_f<0){
        printf("Error: Cannot read file.\n");
        exit(0);
    }
    tmp_buffer[file_size-1] = '\0';
    char* tmp_ptr = tmp_buffer;
    char* end = strchr(tmp_buffer, '\0');
    while(tmp_ptr < end){
        char* equal = strchr(tmp_ptr, '=');
        if(equal != NULL){
            amt_equal++;
        }
        amt_end_line++;
        tmp_ptr = strchr(++tmp_ptr, '\n');
        if(tmp_ptr == NULL){
            tmp_ptr = strchr(tmp_buffer, '\0');
        }
        if(end-tmp_ptr < 2) break;
    }

    amt_cmd = amt_equal+1;
    amt_pipe = amt_end_line - amt_equal + 1;
    free(tmp_buffer);
    fclose(tmp_file);
}

void check_arg(int argv, char** argc){
    if(argv != 2){
        printf("Error: Invalid amount of arguments.\n");
        exit(0);
    }

    FILE* f = fopen(argc[1], "r");
    if(f == NULL){
        printf("Error: Connot open a file.\n");
        exit(0);
    }
    fclose(f);

}

void exec_wrapper(char* cmd, char** args){

    int i = 0;
    while(args[i] != NULL){
        int len = 0;
        while(args[i][len] != '\0'){
            len++;
        }
        if(args[i][0] == '\'' && args[i][len-1] == '\''){
            for(int j = 0; j+1<len; j++){
                args[i][j] = args[i][j+1];
            }
            args[i][len-2] = '\0';
        }

        i++;
    }
    execvp(cmd, args);
}

void f_exec_pipe(char*** pipe_arr){
    int len_fd = 0;
    while(len_fd < 32 && pipe_arr[len_fd] != NULL){
        len_fd++;
    }
    int** fd = calloc(len_fd, sizeof(int*));
    for(int i = 0; i<len_fd-1; i++){
        fd[i] = calloc(2, sizeof(int));
        pipe(fd[i]);
    }

    fd[len_fd-1] = calloc(2, sizeof(int));
    fd[len_fd-1][0] = STDIN_FILENO;
    fd[len_fd-1][1] = STDOUT_FILENO;

    if(len_fd == 1){
        pid_t pid = fork();
        if(pid == 0){
            execvp(pipe_arr[0][0], pipe_arr[0]);
            exit(0);
        }
    }
    else{
        pid_t pid = fork();
        if(pid == 0){
            for(int i = 0; i<len_fd; i++){
                if(i != 0 && i != len_fd-1){
                    close(fd[i][0]);
                    close(fd[i][1]);
                }
            }
            close(fd[0][0]);
            dup2(fd[len_fd-1][0], STDIN_FILENO);
            dup2(fd[0][1], STDOUT_FILENO);
            exec_wrapper(pipe_arr[0][0], pipe_arr[0]);
            exit(0);
        }
        for(int i = 0; i<len_fd-2; i++){
            pid = fork();
            if(pid == 0){
                for(int j = 0; j<len_fd; j++){
                    if(i != j && i+1 != j && j != len_fd-1) {
                        close(fd[j][0]);
                        close(fd[j][1]);
                    }
                }
                close(fd[i][1]);
                close(fd[i+1][0]);
                dup2(fd[i][0], STDIN_FILENO);
                dup2(fd[i + 1][1], STDOUT_FILENO);
                exec_wrapper(pipe_arr[i+1][0], pipe_arr[i+1]);
                exit(0);
            }
        }
        pid = fork();
        if(pid == 0){
            for(int j = 0; j<len_fd-2; j++){
                close(fd[j][0]);
                close(fd[j][1]);
            }
            close(fd[len_fd-2][1]);
            dup2(fd[len_fd-2][0], STDIN_FILENO);
            dup2(fd[len_fd-1][1], STDOUT_FILENO);
            exec_wrapper(pipe_arr[len_fd-1][0], pipe_arr[len_fd-1]);
            exit(0);
        }
    }
    for(int j = 0; j<len_fd-1; j++){
        close(fd[j][0]);
        close(fd[j][1]);
    }
    for(int i=0; i<len_fd; i++){
        wait(NULL);
    }
    for(int i = 0; i<len_fd; i++){
        free(fd[i]);
    }
    free(fd);
}