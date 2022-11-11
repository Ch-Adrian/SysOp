#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define free_mem(ptr) free(ptr); ptr = NULL;

typedef struct file_counter{
    int* amount;
} FCounter;

char* get_working_directory(const char * directory);
void show_file_info(const struct stat* dir_stat, const char* working_directory);
void show_FCounter(FCounter* fCounter);
void count_file(const struct stat* dir_stat, FCounter* fCounter);
int fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
FCounter* fCounter;

int main(int argc, char** argv){

    if(argc > 2){
        printf("Invalid amount of arguments. Should be one: directory");
        exit(1);
    }

    fCounter = calloc(1, sizeof (FCounter));
    fCounter->amount = calloc(7, sizeof (int));

    if(nftw(argv[1], fn, 100, FTW_DEPTH | FTW_PHYS) == -1){
        perror("nftw");
        exit(EXIT_FAILURE);
    }
    show_FCounter(fCounter);

    free_mem(fCounter->amount);
    free_mem(fCounter);

    return 0;
}

int fn(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf){

    show_file_info(sb, fpath);
    count_file(sb, fCounter);
    return 0;
}

void show_file_info(const struct stat* dir_stat, const char* working_directory){

    char* tmp_dir = get_working_directory(working_directory);
    printf("Sciezka: %s\n", tmp_dir );
    free_mem(tmp_dir);

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

    char buff[256];
    strftime(buff, sizeof(buff), "%D %T", gmtime (&dir_stat->st_atime));
    printf("Data ostatniego dostepu: %s\n", buff);
    strftime(buff, sizeof(buff), "%D %T", gmtime (&dir_stat->st_atime));
    printf("Data ostatniej modyfikacji: %s\n", buff);
    printf("\n");
}

void count_file(const struct stat* dir_stat, FCounter* fCounter){

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

char* get_working_directory(const char * directory){
    char* working_directory = calloc(1024, sizeof(char));

    if(directory[0] == '/'){
        strcpy(working_directory, directory);
    }
    else{
        char* out = getcwd(working_directory, 1024);
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


