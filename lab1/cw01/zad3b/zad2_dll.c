#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "memory_block_operations.h"

const char commandCreate[13] = "create_table";
const char commandWC[9] = "wc_files";
const char commandRemove[13] = "remove_block";

typedef struct Array_of_tables{
    Tab** the_array_of_tables;
    int array_size;
    int tabWsk;
    int tabSize;
} ATables;

typedef struct TileFileReport{
    FILE* file;
    clock_t rtime;
    clock_t utime;
    clock_t stime;
} TFReport;

ATables* initATables(void* handle);
TFReport* initTFReport();
void start_report(int argc, char** argv, TFReport* report);
void deallocateATables(ATables* t, void* handle);
void report_error(char* mess, char* mess2);
short case_create(char* argv, ATables* a_tables,void* handle);
int files_amount(int j, int argc, char** argv);
void exec_wc(int amt, char** files, ATables* a_tables, TFReport* report,void* handle);
void report_to_file(TFReport * report, char* message);
void close_report(TFReport* report);
void time_to_file(clock_t real, struct tms* start, struct tms* stop, TFReport* report);
void report_remove_block(clock_t real, struct tms* start, struct tms* stop, TFReport* report, char* block_nr);

int main(int argc, char** argv){

    void* handle = dlopen("./libmemory_block_operations.so", RTLD_LAZY);

    ATables* a_tables = initATables(handle);
    TFReport* report = initTFReport();
    start_report(argc, argv, report);

    for(int i = 1; i<argc; i+=2){

        if(!strcmp(commandCreate, argv[i])){
            if(i+1 >= argc) {
                report_error("Invalid amount of arguments: "," Perhaps command create_table should have one. ");
                deallocateATables(a_tables, handle);
                exit(0);
            }
            if(case_create(argv[i+1], a_tables, handle) == EXIT_ERROR) break;
        }
        else if(!strcmp(commandWC, argv[i])){
            int amt = files_amount(i+1, argc, argv);

            char** files = calloc(amt, sizeof(char*));
            for(int k = i+1; k <= i+amt; k++){
                files[k-i-1] = argv[k];
            }

            exec_wc(amt, files, a_tables, report,handle);

            free_mem(files);
            i += amt-1;
        }
        else if(!strcmp(commandRemove, argv[i])){
            if(i+1 >= argc) {
                report_error("Invalid amount of arguments: "," Perhaps command remove_block should have one. ");
                deallocateATables(a_tables, handle);
                exit(0);
            }
            struct tms tms_start, tms_end;
            clock_t clock_start, clock_end;
            if( (clock_start = times(&tms_start)) == -1){
                report_error("function times", "");
            }

                int number = atoi(argv[i+1]);
                if(number <= 0 || number > a_tables->tabSize){
                    report_error("Invalid id number in command( first block is 1 ): ", argv[i]);
                    break;
                }
                number -= 1;
                short (*remove_block_from_table)(int , Tab*) = dlsym(handle, "remove_block_from_table");

                if(remove_block_from_table(number, a_tables->the_array_of_tables[a_tables->tabWsk]) == EXIT_ERROR){
                    report_error("","");
                    deallocateATables(a_tables, handle);
                    exit(0);
                }

            if( (clock_end = times(&tms_end)) == -1){
                report_error("function times", "");
            }
            report_remove_block( (clock_end - clock_start), &tms_start, &tms_end, report, argv[i+1]);
        }
        else{
            report_error("Invalid command: ", argv[i]);
            deallocateATables(a_tables, handle);
            exit(0);
        }
    }

    struct tms tms_start, tms_end;
    clock_t clock_start, clock_end;
    if( (clock_start = times(&tms_start)) == -1){
        report_error("function times", "");
    }

    deallocateATables(a_tables, handle);
    
    if( (clock_end = times(&tms_end)) == -1){
        report_error("function times", "");
    }
    report->rtime += (clock_end - clock_start);
    report->utime += (tms_end.tms_utime - tms_start.tms_utime);
    report->stime += (tms_end.tms_stime - tms_start.tms_stime);

    report_to_file(report, "Time allocating and deallocating all blocks: ");
    static long clktck = 0;
    if( (clktck = sysconf(_SC_CLK_TCK)) < 0){
        report_error("Function sysconf","");
        clktck = 1;
    }

    printf("real time: %7.4f\n", (report->rtime/(double)clktck));
    printf("user time: %7.4f\n", (report->utime /(double)clktck));
    printf("system time: %7.4f\n", (report->stime/(double)clktck));
    printf("\n");



    fprintf(report->file, "real time: %7.4f\n", (report->rtime/(double)clktck));
    fprintf(report->file, "user time: %7.4f\n", (report->utime /(double)clktck));
    fprintf(report->file, "system time: %7.4f\n", (report->stime/(double)clktck));
    fprintf(report->file, "\n");

    close_report(report);
    dlclose(handle);
    return 0;
}

TFReport* initTFReport(){
    TFReport* r = calloc(1, sizeof(TFReport));
    r->file = fopen("raport2.txt", "w");
    r->rtime = 0;
    r->utime = 0;
    r->stime = 0;
    return r;
}

ATables* initATables(void* handle){
    Tab* (*create_table)(int) = dlsym(handle, "create_table");
    ATables*t = calloc(1, sizeof(ATables)); 
    t->array_size = 100;
    t->the_array_of_tables = calloc(t->array_size, sizeof(Tab*));
    t->tabWsk = 0;
    t->tabSize = 20;
    t->the_array_of_tables[t->tabWsk] = create_table(t->tabSize);
    return t;
}

void start_report(int argc, char** argv, TFReport* report){
    char mess[500] = "Executed command: ";
    strcat(mess, argv[0]);
    report_to_file(report, mess);
    strcpy(mess, "With parameters: ");
    for(int i = 1; i<argc; i++){
        if(strlen(mess) + strlen(argv[i]) + 5 < 500){
            strcat(mess, argv[i]);
            strcat(mess, " ");
        }
        else{
            strcat(mess, " ...");
            break;
        }
    }
    strcat(mess, "\n");
    report_to_file(report, mess);
}

void deallocateATables(ATables* t, void* handle){

    short (*deallocate_table)(Tab*) = dlsym(handle, "deallocate_table");

    for(int i = 0; i<t->array_size; i++){
        if(t->the_array_of_tables[i] != NULL){
            deallocate_table(t->the_array_of_tables[i]);
        }
    }
    free_mem(t->the_array_of_tables);
    free_mem(t);
}

void report_error(char* mess, char* mess2){
    char message[500] = "ERROR: ";
    strcat(message, mess);
    strcat(message, mess2);
    strcat(message, "\n");
    printf("%s\n",message);
}

short case_create(char* argv, ATables* a_tables,void* handle){

    Tab* (*create_table)(int) = dlsym(handle, "create_table");

    if(a_tables->tabWsk == 99){
        report_error("No space for next table","");
        return EXIT_ERROR;
    }
    else{
        a_tables->tabWsk++;
        int number = atoi(argv);
        if(number == 0){
            report_error("Invalid argument of create_table: ",argv);
            return EXIT_ERROR;
        }
        a_tables->tabSize = number;
        a_tables->the_array_of_tables[a_tables->tabWsk] = create_table(a_tables->tabSize);
    }
    return EXIT_OK;
}

int files_amount(int jBeg, int argc, char** argv){
    int j = jBeg;
    while(j<argc && !(!strcmp(commandCreate, argv[j]) || 
            !strcmp(commandRemove, argv[j]) || 
            !strcmp(commandWC, argv[j]))){
            j++;
    }
    return j-jBeg;
}

void exec_wc(int amt, char** files, ATables* a_tables, TFReport* report, void* handle){

    struct tms tms_start, tms_end;
    clock_t clock_start, clock_end;
    char message[500]="";
    if( (clock_start = times(&tms_start)) == -1){
        report_error("function times", "");
    }

    char* (*exec_cmd_wc_on_files)(int, char**) = dlsym(handle, "exec_cmd_wc_on_files");

    char* tmp_file = exec_cmd_wc_on_files(amt, files);
    //  for(int i = 0; i<100; i++){
    //     for(int j = 0; j< 100; j++){
    //             system("cd /usr/include; grep _POSIX_SOURCE */*.h > /dev/null");
    //     }
    // }

    if( (clock_end = times(&tms_end)) == -1){
        report_error("function times", "");
    }
    strcat(message, "Executed command wc on files: ");
    for(int i = 0 ; i<amt; i++){
        if(files[i] != NULL){
            strcat(message, files[i]);
            strcat(message, " ");
        }
    }
    strcat(message, "\nTime:");
    report_to_file(report, message);
    time_to_file((clock_end - clock_start), &tms_start, &tms_end, report);

    if( (clock_start = times(&tms_start)) == -1){
        report_error("function times", "");
    }

    short (*create_block_from_file)(char*, Tab*) = dlsym(handle, "create_block_from_file");

    if(tmp_file != NULL){
        if(create_block_from_file(tmp_file, a_tables->the_array_of_tables[a_tables->tabWsk]) == EXIT_ERROR){
            report_error("","");
        }
    }


    if( (clock_end = times(&tms_end)) == -1){
        report_error("function times", "");
    }
    strcpy(message, "Created block using files: ");
    for(int i = 0 ; i<amt; i++){
        if(files[i] != NULL){
            strcat(message, files[i]);
            strcat(message, " ");
        }
    }
    strcat(message, "\nTime:");
    report_to_file(report, message);
    time_to_file((clock_end - clock_start), &tms_start, &tms_end, report);
    report->rtime += (clock_end - clock_start);
    report->utime += (tms_end.tms_utime - tms_start.tms_utime);
    report->stime += (tms_end.tms_stime - tms_start.tms_stime);
}

void report_to_file(TFReport * report, char message[]){
    printf("%s\n", message);
    fprintf(report->file, "%s\n", message);
}

void close_report(TFReport* report){
    fclose(report->file);
    free_mem(report);
}

void time_to_file(clock_t real, struct tms* start, struct tms* stop, TFReport* report){
    static long clktck = 0;

    if( (clktck = sysconf(_SC_CLK_TCK)) < 0){
        report_error("Function sysconf","");
        clktck = 1;
    }

    printf("real time: %7.4f\n", real/(double)clktck);
    printf("user time: %7.4f\n", ((stop->tms_utime - start->tms_utime)/(double)clktck));
    printf("system time: %7.4f\n", ((stop->tms_stime - start->tms_stime)/(double)clktck));
    printf("\n");

    fprintf(report->file, "real time: %7.4f\n", real/(double)clktck);
    fprintf(report->file, "user time: %7.4f\n", ((stop->tms_utime - start->tms_utime)/(double)clktck));
    fprintf(report->file, "system time: %7.4f\n", ((stop->tms_stime - start->tms_stime)/(double)clktck));
    fprintf(report->file, "\n");
}

void report_remove_block(clock_t real, struct tms* start, struct tms* stop, TFReport* report, char* block_nr){
    char message[500]="";
    strcat(message, "Removed block nr ");
    strcat(message, block_nr);
    strcat(message, ": ");
    strcat(message, "\nTime:");
    report_to_file(report, message);
    time_to_file( real, start, stop, report);
    report->rtime += real;
    report->utime += (stop->tms_utime - start->tms_utime);
    report->stime += (stop->tms_stime - start->tms_stime);
}