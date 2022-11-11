#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

enum Mode{ BLOCK, NUMBERS };
enum Mode mode = -1;

char* file_IN = NULL;
char* file_OUT = NULL;
char **line = NULL;
FILE *image = NULL;
FILE *image_out = NULL;
FILE* file_wynik = NULL;
long long int total_time_sec = 0;
long long int total_time_microsec = 0;
typedef struct Pair{
    int x, y;
} pair;

int amt_lines = 3;
int amt_pthread = 0;
short** vis = NULL;
int **matrix = NULL;
int width = 0;
int height = 0;

typedef struct Task{
    int ** matrix;
    int begin;
    int end;
} task;

void at_end();
void check_arguments(int argc, char** argv);
void process_image();
void* thread_function_numbers(void * arg);
void* thread_function_block(void* arg);
void revert_BLOCK(int** matrix);
void revert_NUMBERS(int** matrix);
void save_image();

int main(int argc, char** argv){

    atexit(at_end);
    check_arguments(argc, argv);

    amt_pthread = atoi(argv[1]);

    process_image();

    file_wynik = fopen("Times2.txt", "a");

    if(mode == BLOCK) {
        fprintf(file_wynik, "Program has %d threads and mode: %s\n", amt_pthread, "block");
    }
    else{
        fprintf(file_wynik, "Program has %d threads and mode: %s\n", amt_pthread, "numbers");
    }

    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);

    if (mode == BLOCK) {
        revert_BLOCK(matrix);
    } else if (mode == NUMBERS) {
        revert_NUMBERS(matrix);
    }

    clock_gettime(CLOCK_REALTIME, &stop);

    total_time_microsec = (total_time_microsec + (stop.tv_nsec - start.tv_nsec)/1000);
    total_time_sec = total_time_sec + stop.tv_sec - start.tv_sec + total_time_microsec/1000000;
    total_time_microsec = total_time_microsec%1000000;

    fprintf(file_wynik,"Total time: %lld s. and %lld microseconds.\n", total_time_sec, total_time_microsec);
    printf("Total time: %lld s. and %lld microseconds.\n", total_time_sec, total_time_microsec);
    fprintf(file_wynik, "\n");

    save_image();
    return 0;
}

void at_end(){
    if(file_IN != NULL){
        free(file_IN);
        file_IN = NULL;
    }

    if(file_OUT != NULL){
        free(file_OUT);
        file_OUT = NULL;
    }

    if(vis != NULL){
        for(int i = 0; i<height; i++){
            free(vis[i]);
        }
        free(vis);
    }

    if(matrix != NULL){
        for(int i = 0; i<height; i++){
            free(matrix[i]);
        }
        free(matrix);
    }

    if(line != NULL){
        for(int i = 0; i<amt_lines; i++){
            free(line[i]);
        }
        free(line);
    }

    if(image != NULL){
        fclose(image);
    }

    if(image_out != NULL){
        fclose(image_out);
    }

    if(file_wynik != NULL){
        fclose(file_wynik);
    }

}

void check_arguments(int argc, char** argv){

    if( argc != 5){
        printf("Error: Invalid amount of arguments.\n");
        exit(1);
    }

    for(int i = 0; i < strlen(argv[1]); i++){
        if(argv[1][i] < '0' || argv[1][i] > '9'){
            printf("Error: Invalid first argument: Should be a positive integer.\n");
            exit(1);
        }
    }

    char block[] = "block";
    char num[] = "numbers";

    if(strcmp(block, argv[2]) == 0){
        mode = BLOCK;
    }

    if(strcmp(num, argv[2]) == 0){
        mode = NUMBERS;
    }

    if(mode == -1){
        printf("Error: Second argument should be: block, numbers.\n");
        exit(1);
    }

    file_IN = calloc(256, sizeof(char));
    strcpy(file_IN, argv[3]);
    file_OUT = calloc(256, sizeof (char));
    strcpy(file_OUT, argv[4]);

}

void process_image() {
    image = fopen(file_IN, "r");
    if (image == NULL) {
        printf("Error: Cannot open image.\n");
        exit(1);
    }

    image_out = fopen(file_OUT, "w");
    if (image_out == NULL) {
        printf("Error: Cannot open image.\n");
        exit(1);
    }

    size_t len = 256;
    line = malloc(sizeof(char *) * amt_lines);
    for (int i = 0; i < amt_lines; i++) {
        line[i] = malloc(sizeof(char) * len);
    }
    int i = 0;
    while (i != amt_lines) {
        getline(&line[i], &len, image);
        int j = 0;
        int _len = strlen(line[i]);
        while (j < _len && (line[i][j] == ' ' || line[i][j] == '\t')) {
            j++;
        }
        if (j >= _len) {
            i--;
        } else if (line[i][j] == '#') {
            i--;
        }
        i++;
    }

    width = atoi(line[1]);
    height = atoi(strchr(line[1], ' '));

    matrix = calloc(height, sizeof(int *));
    for (int i = 0; i < height; i++) {
        matrix[i] = calloc(width, sizeof(int));
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fscanf(image, "%d", &matrix[i][j]);
        }
    }

}

void save_image(){
    fprintf(image_out, "%s", line[0]);
    fprintf(image_out, "%s", line[1]);
    fprintf(image_out, "%s", line[2]);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (j == width - 1) {
                fprintf(image_out, "%d\n", matrix[i][j]);
            } else {
                fprintf(image_out, "%d ", matrix[i][j]);
            }
        }
    }
}

void* thread_function_numbers(void * arg){
    task* _task = (task*) arg;

    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);

    for(int i =0 ;i < height; i++){
        for(int j = 0; j<width; j++){
            if( _task->matrix[i][j] >= _task->begin && _task->matrix[i][j] < _task->end && vis[i][j] == 0){
                _task->matrix[i][j] = 255 - _task->matrix[i][j];
                vis[i][j] = 1;
            }
        }
    }

    clock_gettime(CLOCK_REALTIME, &stop);
    char* str = malloc(100*sizeof(char));
    sprintf(str, "%ld sec. %ld microseconds.", stop.tv_sec-start.tv_sec, (stop.tv_nsec - start.tv_nsec)/1000);
//    printf("%s\n", str);
    total_time_sec += stop.tv_sec-start.tv_sec;
    total_time_microsec += (stop.tv_nsec - start.tv_nsec)/1000;
    pthread_exit((void*) str);
}

void* thread_function_block(void* arg){
    task* _task = (task*) arg;
    struct timespec start, stop;
    clock_gettime(CLOCK_REALTIME, &start);

    for(int i =0 ;i < height; i++){
        for(int j = _task->begin; j<=_task->end; j++){
            _task->matrix[i][j] = 255 - _task->matrix[i][j];
        }
    }
    clock_gettime(CLOCK_REALTIME, &stop);
    char* str = malloc(100*sizeof(char));
    sprintf(str, "%ld sec. %ld microseconds.", stop.tv_sec-start.tv_sec, (stop.tv_nsec - start.tv_nsec)/1000);
    total_time_sec += stop.tv_sec-start.tv_sec;
    total_time_microsec += (stop.tv_nsec - start.tv_nsec)/1000;
    pthread_exit((void*) str);
}

void revert_BLOCK(int** matrix){

    pthread_t thread[amt_pthread];
    task* tasks = calloc(amt_pthread, sizeof(task));

    for(int i = 0; i<amt_pthread; i++){
        tasks[i].matrix = matrix;
        tasks[i].begin = (i)*(width/amt_pthread);
        if(i == amt_pthread - 1){
            tasks[i].end = width-1;
        }
        else {
            tasks[i].end = (i+1)*(width/amt_pthread)-1;
        }
        pthread_create(&(thread[i]), NULL, thread_function_block, (void*) &tasks[i]);
    }
    void* str[amt_pthread];
    for(int i =0 ;i<amt_pthread; i++){
        pthread_join(thread[i], &str[i]);
        fprintf(file_wynik, "Thread %d: %s\n", i, (char*)str[i]);
        free(str[i]);
    }

    free(tasks);
}

void revert_NUMBERS(int** matrix){

    int amt_share = 255/amt_pthread;
    pthread_t thread[amt_pthread];
    task* tasks = calloc(amt_pthread, sizeof(task));
    vis = calloc(height, sizeof (short*));

    for(int i = 0; i<height; i++){
        vis[i] = calloc(width, sizeof (short));
    }
    for(int i = 0; i<height; i++){
        for(int j =0; j<width; j++){
            vis[i][j] = 0;
        }
    }

    for(int i = 0; i<amt_pthread; i++){
        tasks[i].matrix = matrix;
        tasks[i].begin = amt_share * i;
        if(i == amt_pthread - 1){
            tasks[i].end = 256;
        }
        else {
            tasks[i].end = amt_share * (i + 1);
        }
        pthread_create(&(thread[i]), NULL, thread_function_numbers, (void*) &tasks[i]);
    }

    void* str[amt_pthread];
    for(int i =0 ;i<amt_pthread; i++){
        pthread_join(thread[i], &str[i]);
        fprintf(file_wynik, "Thread %d: %s\n", i, (char*)str[i]);
        free(str[i]);
    }

    free(tasks);
}
