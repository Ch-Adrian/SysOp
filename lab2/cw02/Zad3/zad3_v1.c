#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define free_mem(ptr) free(ptr); ptr = NULL;

typedef struct file_counter{
    int* amount;
} FCounter;

char* get_working_directory(char * directory);
void show_file_info(char* working_directory, FCounter* fCounter, unsigned char d_type);
void count_file(struct stat* dir_stat, FCounter* fCounter);
void voyager(char* directory, FCounter* fCounter);
void show_dir_content(char* directory, FCounter* fCounter);
void show_FCounter(FCounter* fCounter);

int main(int argc, char** argv){

    if(argc > 2){
        printf("Invalid amount of arguments. Should be one: directory");
        exit(1);
    }


    FCounter* fCounter = calloc(1, sizeof (FCounter));
    fCounter->amount = calloc(7, sizeof (int));
    fCounter->amount[3]++;

    voyager(argv[1], fCounter);
    show_FCounter(fCounter);

    free_mem(fCounter->amount);
    free_mem(fCounter);

    return 0;
}

void show_dir_content(char* directory, FCounter* fCounter){
    DIR* dir = opendir(directory);
    if(dir == NULL){
        printf("Error: Cannot open directory: %s\n", directory);
        return;
    }

    char* working_directory = get_working_directory(directory);

    struct dirent* entry;

    while((entry= readdir(dir))){
        if(!(*entry->d_name == '.' || strcmp(entry->d_name, "..") == 0)){
            char* inner_directory = calloc(1024, sizeof(char));
            strcpy(inner_directory, working_directory);
            if(inner_directory[strlen(inner_directory)-1] != '/'){
                strcat(inner_directory, "/");
            }
            strcat(inner_directory, entry->d_name);
            show_file_info(inner_directory, fCounter, entry->d_type);
            free_mem(inner_directory);
        }
    }

    if(closedir(dir) == -1){
        perror("Cannot close directory.");
        exit(1);
    }
    free_mem(working_directory);
}

void voyager(char* directory, FCounter* fCounter){

    show_dir_content(directory, fCounter);

    DIR* dir = opendir(directory);
    if(dir == NULL){
        printf("Error: Cannot open directory: %s\n", directory);
        return;
    }

    char* working_directory = get_working_directory(directory);

    struct dirent* entry;

    while((entry= readdir(dir))){
        if(!(*entry->d_name == '.' || strcmp(entry->d_name, "..") == 0)){
            if(entry->d_type == DT_DIR){
                char* inner_directory = calloc(1024, sizeof(char));
                strcpy(inner_directory, working_directory);
                if(inner_directory[strlen(inner_directory)-1] != '/'){
                    strcat(inner_directory, "/");
                }
                strcat(inner_directory, entry->d_name);
                voyager(inner_directory, fCounter);
                free_mem(inner_directory);
            }
        }
    }

    if(closedir(dir) == -1){
        perror("Cannot close directory.");
        exit(1);
    }
    free_mem(working_directory);
}

char* get_working_directory(char * directory){
    char* working_directory = calloc(256, sizeof(char));

    if(directory[0] == '/'){
        strcpy(working_directory, directory);
    }
    else{
        char* out = getcwd(working_directory, 256);
        if(out == NULL){
            printf("Cannot get current working directory.");
            free_mem(working_directory);
            exit(1);
        }
        char * tmp_dir = calloc(256, sizeof(char));
        strcpy(tmp_dir, directory);
        int len = strlen(tmp_dir);
        for(int i = 1; i<=len; i++){
            tmp_dir[i-1] = tmp_dir[i];
        }
        strcat(working_directory, tmp_dir);
        free_mem(tmp_dir);
    }
    return working_directory;
}

void show_file_info(char* working_directory, FCounter* fCounter, unsigned char d_type){

    struct stat* dir_stat = calloc(1, sizeof(struct stat));
    int outStat = lstat(working_directory, dir_stat);
    if(outStat == -1){
        printf("Cannot view statistics about directory: %s\n", working_directory);
        free_mem(dir_stat);
        return;
    }
    count_file(dir_stat, fCounter);
    printf("Sciezka: %s\n", working_directory );

    nlink_t link = dir_stat->st_nlink;
    printf("Liczba dowiazan: %ld\n", link);

    char type[256] = "";
    switch(dir_stat->st_mode & S_IFMT){
        case S_IFREG:
            strcpy(type, "Regular file.");
            break;
        case S_IFBLK:
            strcpy(type, "Block device");
            break;
        case S_IFCHR:
            strcpy(type, "Character device");
            break;
        case S_IFDIR:
            strcpy(type, "Directory");
            break;
        case S_IFIFO:
            strcpy(type, "Named pipe(FIFO)");
            break;
        case S_IFLNK:
            strcpy(type, "Symbolic link");
            break;
        case S_IFSOCK:
            strcpy(type, "UNIX domain socket");
            break;
    }
    printf("Rodzaj pliku: %s\n", type);

    off_t off = dir_stat->st_size;
    printf("Rozmiar w bajtach: %ld\n", off);

    struct timespec atime = dir_stat->st_atim;
    char buff[256];
    strftime(buff, sizeof(buff), "%D %T", gmtime (&atime.tv_sec));
    printf("Data ostatniego dostepu: %s\n", buff);
    atime = dir_stat->st_mtim;
    strftime(buff, sizeof(buff), "%D %T", gmtime (&atime.tv_sec));
    printf("Data ostatniej modyfikacji: %s\n", buff);
    printf("\n");

    free_mem(dir_stat);
}

void count_file(struct stat* dir_stat, FCounter* fCounter){

    switch(dir_stat->st_mode & S_IFMT){
        case S_IFREG:
            fCounter->amount[0]++;
            break;
        case S_IFBLK:
            fCounter->amount[1]++;
            break;
        case S_IFCHR:
            fCounter->amount[2]++;
            break;
        case S_IFDIR:
            fCounter->amount[3]++;
            break;
        case S_IFIFO:
            fCounter->amount[4]++;
            break;
        case S_IFLNK:
            fCounter->amount[5]++;
            break;
        case S_IFSOCK:
            fCounter->amount[6]++;
            break;
    }
}

void show_FCounter(FCounter* fCounter){
    printf("Regular files: %d\n", fCounter->amount[0]);
    printf("Block devices: %d\n", fCounter->amount[1]);
    printf("Character devices: %d\n", fCounter->amount[2]);
    printf("Directories: %d\n", fCounter->amount[3]);
    printf("Named pipes(FIFO): %d\n", fCounter->amount[4]);
    printf("Symbolic links: %d\n", fCounter->amount[5]);
    printf("Sockets: %d\n", fCounter->amount[6]);
}