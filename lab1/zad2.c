#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "source/memory_block_operations.h"

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

ATables* initATables();
TFReport* initTFReport();
void deallocateATables(ATables* t);
void report_error(char* mess, char* mess2);
short case_create(char* argv, ATables* a_tables);
int files_amount(int j, int argc, char** argv);
void exec_wc(int amt, char** files, ATables* a_tables, TFReport* report);
void report_to_file(TFReport * report, char* message);
void close_report(TFReport* report);
void time_to_file(clock_t real, struct tms* start, struct tms* stop, TFReport* report);

int main(int argc, char** argv){

    ATables* a_tables = initATables();
    TFReport* report = initTFReport();
    char mess[200] = "Executed command: ";
    strcat(mess, argv[0]);
    report_to_file(report, mess);
    strcpy(mess, "With parameters: ");
    for(int i = 1; i<argc; i++){
        strcat(mess, argv[i]);
        strcat(mess, " ");
    }
    strcat(mess, "\n");
    report_to_file(report, mess);

    for(int i = 1; i<argc; i+=2){

        if(!strcmp(commandCreate, argv[i])){
            if(case_create(argv[i+1], a_tables) == EXIT_ERROR) break;
        }
        else if(!strcmp(commandWC, argv[i])){
            int amt = files_amount(i+1, argc, argv);

            char** files = calloc(amt, sizeof(char*));
            for(int k = i+1; k <= i+amt; k++){
                files[k-i-1] = argv[k];
            }

            exec_wc(amt, files, a_tables, report);

            free_mem(files);
            i += amt-1;
        }
        else if(!strcmp(commandRemove, argv[i])){

            struct tms tms_start, tms_end;
            clock_t clock_start, clock_end;
            char message[300]="";
            if( (clock_start = times(&tms_start)) == -1){
                report_error("function times", "");
            }

                int number = atoi(argv[i+1]);
                if(number < 0 || number >= a_tables->tabSize){
                    report_error("Invalid size in command: ", argv[i]);
                    break;
                }
                if(remove_block_from_table(number, a_tables->the_array_of_tables[a_tables->tabWsk]) == EXIT_ERROR){
                    report_error("","");
                    exit(0);
                }

            if( (clock_end = times(&tms_end)) == -1){
                report_error("function times", "");
            }
            strcat(message, "Removed block nr ");
            strcat(message, argv[i+1]);
            strcat(message, ": ");
            strcat(message, "\nTime:");
            report_to_file(report, message);
            time_to_file((clock_end - clock_start), &tms_start, &tms_end, report);
            report->rtime += (clock_end - clock_start);
            report->utime += (tms_end.tms_utime - tms_start.tms_utime);
            report->stime += (tms_end.tms_stime - tms_start.tms_stime);

        }
        else{
            report_error("Invalid command: ", argv[i]);
            exit(0);
        }
    }

    struct tms tms_start, tms_end;
    clock_t clock_start, clock_end;
    if( (clock_start = times(&tms_start)) == -1){
        report_error("function times", "");
    }
    deallocateATables(a_tables);
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
    fprintf(report->file, "real time: %7.4f\n", (report->rtime/(double)clktck));
    fprintf(report->file, "user time: %7.4f\n", (report->utime /(double)clktck));
    fprintf(report->file, "system time: %7.4f\n", (report->stime/(double)clktck));
    fprintf(report->file, "\n");

    close_report(report);
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

ATables* initATables(){
    ATables*t = calloc(1, sizeof(ATables)); 
    t->array_size = 100;
    t->the_array_of_tables = calloc(t->array_size, sizeof(Tab*));
    t->tabWsk = 0;
    t->tabSize = 20;
    t->the_array_of_tables[t->tabWsk] = create_table(t->tabSize);
    return t;
}

void deallocateATables(ATables* t){

    for(int i = 0; i<t->array_size; i++){
        if(t->the_array_of_tables[i] != NULL){
            deallocate_table(t->the_array_of_tables[i]);
        }
    }
    free_mem(t->the_array_of_tables);
    free_mem(t);
}

void report_error(char* mess, char* mess2){
    char message[200] = "ERROR: ";
    strcat(message, mess);
    strcat(message, mess2);
    strcat(message, "\n");
    printf("%s\n",message);
}

short case_create(char* argv, ATables* a_tables){
    if(a_tables->tabWsk == 99){
        report_error("No space for next table","");
        return EXIT_ERROR;
    }
    else{
        a_tables->tabWsk++;
        a_tables->tabSize = atoi(argv);
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

void exec_wc(int amt, char** files, ATables* a_tables, TFReport* report){

    struct tms tms_start, tms_end;
    clock_t clock_start, clock_end;
    char message[300]="";
    if( (clock_start = times(&tms_start)) == -1){
        report_error("function times", "");
    }

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
    fprintf(report->file, "real time: %7.4f\n", real/(double)clktck);
    fprintf(report->file, "user time: %7.4f\n", ((stop->tms_utime - start->tms_utime)/(double)clktck));
    fprintf(report->file, "system time: %7.4f\n", ((stop->tms_stime - start->tms_stime)/(double)clktck));
    fprintf(report->file, "\n");
}