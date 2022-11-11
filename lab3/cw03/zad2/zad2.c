#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/times.h>


#define free_mem(ptr) free(ptr); ptr = NULL;
#define LD long double

void time_function(void (*fun)(int, char**), int argc, char** argv);
void apply_procedure(int argc, char** argv);
void check_arguments(int argc, char** argv);
void run_processes(double precision, int n);
void f_subprocess(int i, double inner_width, int n);
LD area(LD width, LD beg, LD end);
LD f(LD x);
char* get_file_name(int i);
char* int_to_string(int a);
void sum_results_in_files(int n);

int main(int argc, char** argv){

    time_function(apply_procedure, argc, argv);

    return 0;
}

void time_function(void (*fun)(int, char**),int argc, char** argv){
    struct tms tms_start, tms_end;
    clock_t clock_start, clock_end;
    if( (clock_start = times(&tms_start)) == -1){
        printf("ERROR: function times");
    }

    fun(argc, argv);

    if( (clock_end = times(&tms_end)) == -1){
        printf("ERROR: function times");
    }

    static long clktck = 0;

    if( (clktck = sysconf(_SC_CLK_TCK)) < 0){
        printf("ERROR: Function sysconf");
        clktck = 1;
    }
    clock_t real = clock_end - clock_start;
    printf("real time: %7.4f\n", real/(double)clktck);
    printf("user time: %7.4f\n", ((tms_end.tms_utime - tms_start.tms_utime)/(double)clktck));
    printf("system time: %7.4f\n", ((tms_end.tms_stime - tms_start.tms_stime)/(double)clktck));
    printf("\n");

    FILE * file = fopen("pomiary.txt", "a");
    fprintf(file, "real time: %7.4f\n", real/(double)clktck);
    fprintf(file, "user time: %7.4f\n", ((tms_end.tms_utime - tms_start.tms_utime)/(double)clktck));
    fprintf(file, "system time: %7.4f\n", ((tms_end.tms_stime - tms_start.tms_stime)/(double)clktck));
    fprintf(file, "\n");
    fclose(file);

}

void apply_procedure(int argc, char** argv){

    check_arguments(argc, argv);

    char *endptr;
    LD precision = strtod(argv[1], &endptr);
    int n = atoi(argv[2]);

    if(precision >= 1){
        printf("Error: First argument is too large. Should be less than 1.\n");
        exit(1);
    }

    FILE * file = fopen("pomiary.txt", "a");
    fprintf(file, "Precision: %s  no of processes: %s\n", argv[1], argv[2]);
    fclose(file);

    run_processes(precision, n);
    sum_results_in_files(n);
}

void check_arguments(int argc, char** argv){
    if(argc != 3){
        printf("Invalid amount of arguments.\n");
        exit(1);
    }
    int point = 0;
    for(int i = 0; i<strlen(argv[1]); i++){
        if(argv[1][i] == '.'){
            point++;
        }
        else if( argv[1][i] < '0' || argv[1][i] > '9'){
            printf("Error: First argument must be a positive float number.\n");
            exit(1);
        }
    }
    if(point > 1){
        printf("Error: First argument must be a positive float number.\n");
        exit(1);
    }

    for(int i = 0; i<strlen(argv[2]); i++){
        if( argv[2][i] < '0' || argv[2][i] > '9'){
            printf("Error: Second argument must be an positive integer.\n");
            exit(1);
        }
    }

}

void run_processes(double precision, int n){
    for(int i = 0; i<n; i++){
        pid_t p = fork();
        if( p == 0){
            f_subprocess(i,precision, n);
            return;
        }
    }

    for(int i = 0; i<n; i++){
        wait(NULL);
    }
}

void f_subprocess(int i, double inner_width, int n){
    LD result = 0;
    LD end = 0.9999999999;
    for(LD j = (i*inner_width); j<end; j+= inner_width*n){
        result += area(inner_width, j, j + inner_width);
    }

    char* file_name = get_file_name(i+1);

    FILE* file = fopen(file_name, "w");
    if(file == NULL){
        printf("Error: Cannot open file: %s\n", file_name);
        free(file_name);
        exit(1);
    }
    fprintf(file, "%.25Lf\n", result);

    fclose(file);
    free_mem(file_name);
    exit(0);
}

LD area(LD width, LD beg, LD end){
    LD height = ( f(beg) + f(end) ) / 2.;
    return width * height;
}

LD f(LD x){
    return 4/(x*x+1);
}

char* get_file_name(int i){
    char* str = int_to_string(i);
    char *file_name= calloc(50, sizeof(char));
    strcpy(file_name, "W");
    strcat(file_name, str);
    strcat(file_name, ".txt");

    free_mem(str);
    return file_name;
}

char* int_to_string(int a){
    char* number = calloc(256, sizeof(char));
    if(a <= 0){
        number[0]=48;
        number[1]='\0';
        return number;
    }
    int n = 0;
    int tmp = a;
    while(tmp){
        n++;
        tmp /= 10;
    }
    char num = 0;
    tmp = a;
    for(; n>0; n--){
        num = tmp%10;
        number[n-1] = num + 48;
        tmp /=10;
    }
    return number;
}

void sum_results_in_files(int n){
    LD result = 0;

    for(int i = 1; i<=n; i++){
        char* file_name = get_file_name(i);
        FILE* file = fopen(file_name, "r");
        if(file == NULL){
            printf("Error: Cannot open file: %s\n", file_name);
            exit(1);
        }
        LD value = 0;
        fscanf(file, "%Lf", &value);

        result += value;

        fclose(file);
        free_mem(file_name);
    }

    printf("Result: %.25Lf\n", result);
    FILE * file = fopen("pomiary.txt", "a");
    fprintf(file, "Result: %.25Lf\n", result);
    fclose(file);
}