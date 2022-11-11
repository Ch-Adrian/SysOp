#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define free_mem(ptr) free(ptr); ptr = NULL;

void voyager(char*, char*, int, char*, int);
void find_string_in_file(char* file_name, char* string_to_find, char* fullpath);
void check_arguments(int, char**);
void str_reverse(char* str);

int main( int argc, char* argv[]){

    check_arguments(argc, argv);
    int max_depth = atoi(argv[3]);
    char* fullpath = malloc(1024 * sizeof(char));
    strcpy(fullpath, argv[1]);

    voyager(argv[1], argv[2], 1, fullpath, max_depth);

    free_mem(fullpath);
    return 0;
}

void check_arguments(int argc, char** argv){
    if(argc != 4){
        printf("Invalid amount of arguments.\n");
        exit(1);
    }
    if((argv[1][strlen(argv[1])-1] == '/' || argv[1][strlen(argv[1])-1] == '\\') && (strlen(argv[1]) != 1)){
        argv[1][strlen(argv[1])-1] = '\0';
    }

    DIR* dir = opendir(argv[1]);
    if(dir == NULL){
        printf("Error: Cannot open directory: %s\n", argv[1]);
        exit(1);
    }

    if(closedir(dir) == -1){
        printf("Error: Cannot close directory.");
        exit(1);
    }

    if (!((argv[1][0] == '.' && argv[1][1] == '/') || (argv[1][0] == '/') || (argv[1][0] == '.'))){
        printf("Error: First argument must be relative path.");
        exit(1);
    }

    for(int i = 0; i<strlen(argv[3]); i++){
        if( argv[3][i] < '0' || argv[3][i] > '9'){
            printf("Error: Third argument must be an positive integer.\n");
            exit(1);
        }
    }

}

void voyager(char* file_name, char* string_to_find, int deep, char* fullpath, int max_depth){
    if(deep > max_depth) return;
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    char * ptr;

    int s = lstat(fullpath, &statbuf);
    if (s < 0){
        printf("Error: lstat doesn't work.");
        return;
    }

    if (S_ISDIR(statbuf.st_mode) == 0){
        find_string_in_file(file_name, string_to_find, fullpath);
        return;
    }

    if( (dp = opendir(fullpath)) == NULL){
        printf("Error: Cannot open directory.");
        return;
    }

    int len = strlen(fullpath);
    fullpath[len] = '/';
    fullpath[len+1] = '\0';
    ptr = fullpath + strlen(fullpath);

    while ( (dirp = readdir(dp)) != NULL){

        if ( (strcmp(dirp->d_name, ".") == 0) || (strcmp(dirp->d_name, "..") == 0)) continue;

        strcpy(ptr, dirp->d_name);

        pid_t p = fork();
        if( p == 0) {
            voyager(dirp->d_name, string_to_find, deep+1, fullpath, max_depth);
            exit(0);
        }
        else{
            wait(NULL);
        }

    }

    if (closedir(dp) < 0){
        printf("Error: Cannot close directory.");
        return;
    }
}

void find_string_in_file(char* file_name, char* string_to_find, char* fullpath){
    if(strlen(file_name) > 4
       && file_name[strlen(file_name)-1] == 't'
       && file_name[strlen(file_name)-2] == 'x'
       && file_name[strlen(file_name)-3] == 't'
       && file_name[strlen(file_name)-4] == '.'){

        int len = strlen(string_to_find);
        char* buf = calloc(2*len + len-1 + 1, sizeof(char));
        char* outer_buf = calloc(len*2 + 1, sizeof(char));
        char* inner_buf = calloc(len + 1, sizeof(char));
        strcpy(inner_buf, string_to_find);
        int buf_ptr = 0;

        FILE* file = fopen(fullpath, "r");
        while(!feof(file)) {
            int s = fread(outer_buf, sizeof(char), 2*len, file);
            if( s == 0){
                break;
            }
            outer_buf[s] = '\0';

            for(int i = 0; i<s; i++){
                buf[buf_ptr++] = outer_buf[i];
            }

            for(int i =0; i<buf_ptr - len+1; i++){
                for(int j =i; j< len + i; j++){
                    inner_buf[j-i] = buf[j];
                    inner_buf[j-i+1] = '\0';
                }
                if(strcmp(inner_buf, string_to_find) == 0){
                    printf("%s %d %s\n", fullpath, getpid(), file_name);
                }
            }
            for(int i = buf_ptr - len+1; i < buf_ptr; i++){
                if(i < 0) break;
                buf[i - buf_ptr + len - 1] = buf[i];
                buf[i - buf_ptr + len] = '\0';
            }
            buf_ptr = strlen(buf);

        }

        fclose(file);
        free_mem(outer_buf);
        free_mem(buf);
        free_mem(inner_buf);
    }
}
