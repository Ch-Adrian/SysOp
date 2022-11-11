#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv){
	
	if(argc > 2 || argc == 1 ){
		printf("One argument of type int is required.\n");
		exit(1);
	}

    for(int i = 0; i<strlen(argv[1]); i++){
        if( argv[1][i] < '0' || argv[1][i] > '9'){
            printf("Error: First argument must be an positive integer.\n");
            exit(1);
        }
    }

    int n = atoi(argv[1]);

    if( n == 0 && argv[1][0] != '0'){
		printf("One argument of type int is required.\n");
		return 0;
	}

	pid_t M = getpid();

	printf("Proces macierzysty, pid: %d\n", M);
	for(int i =0; i<n; i++){
		pid_t pid = fork();
		if( pid == 0){
			printf("Jestem procesem potomnym, pid: %d proces macierzysty ma pid: %d\n", getpid(), getppid());
			return 0;
		}
	}
	return 0;
}
