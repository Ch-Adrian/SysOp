#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

void check_args(int, char**);
char* give_buffer_from_file(char*);
char* find_line(char*, int);

int main(int argc, char** argv){

	check_args(argc, argv);

    char* buffer_in = give_buffer_from_file(argv[1]);
    char* buffer_out = give_buffer_from_file(argv[3]);

    int line = atoi(argv[2]);

    char* out_line = find_line(buffer_out, line);
    if(out_line == NULL){
        printf("Cannot find line!\n");
        exit(0);
    }
    if(strcmp(buffer_in, out_line) == 0){
        printf("Correct!\n");
    }
    else{
        printf("Error!\n");
    }

    free(buffer_in);
    free(buffer_out);
    free(out_line);
	return 0;
}


void check_args(int argc, char** argv){

	if(argc != 4){
		printf("Error: Invalid amount of arguments!");
		exit(0);
	}

    FILE* file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Error: Cannot open a file.\n");
        exit(0);
    }
    fclose(file);

    for(int i = 0; i<strlen(argv[2]); i++){
        if( argv[2][i] < '0' || argv[2][i] > '9'){
            printf("Error: Second argument must be an positive integer.\n");
            exit(1);
        }
    }

    file = fopen(argv[3], "r");
    if(file == NULL){
        printf("Error: Cannot open a file.\n");
        exit(0);
    }
    fclose(file);

}

char* give_buffer_from_file(char* file) {

    int fd = open(file, O_RDONLY);
    flock(fd, LOCK_EX);

    off_t size_file_in = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char *buffer = calloc(size_file_in + 1, sizeof(char));

    int cnt = read(fd, buffer, size_file_in);
    if(cnt == 0){
        printf("Error: Cannot read file.\n");
        exit(0);
    }
    buffer[size_file_in] = '\0';

    flock(fd, LOCK_UN);
    close(fd);
    return buffer;
}

char* find_line(char* buffer, int line){

    char* current_pos = buffer;
    int current_line = 1;
    while(current_line < line){
        current_pos = strchr(current_pos, '\n');
        if(current_pos < 0){
            printf("Error: Cannot find line.\n");
            return NULL;
        }
        current_pos++;
        current_line++;
        if(current_pos - buffer >= strlen(buffer)){
            return NULL;
        }
    }
    char* end_line = strchr(current_pos, '\n');
    if(end_line < 0){
        end_line = strchr(current_pos, '\0');
    }
    char* output = calloc(end_line - current_pos+1, sizeof (char));
    strncpy(output, current_pos,end_line - current_pos);
    output[end_line-current_pos] = '\0';
    return output;
}
