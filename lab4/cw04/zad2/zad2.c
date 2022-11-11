#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void handler(int signo, siginfo_t* info, void *context){

    printf("\nNumer sygnalu %d\n", info->si_signo);
    printf("PID procesu wysylajacego: %d\n", info->si_pid);
    long clktck = 0;
    switch(signo) {
        case SIGUSR1:
            printf("Handler otrzymal SIGUSR1.\n");
            printf("Handler wysyla SIGUSR1.\n");
            raise(SIGUSR1);
            break;
        case SIGUSR2:
            printf("Handler otrzymal SIGUSR2, dodatkowa wiadomosc: %d.\n", info->si_value.sival_int);
            break;
        case SIGCHLD:

            if( (clktck = sysconf(_SC_CLK_TCK)) < 0){
                printf("ERROR: Function sysconf");
                clktck = 1;
            }
            printf("Handler otrzymal SIGCHLD.\n");
            printf("Czas uzytkownika: %7.4f\n", info->si_utime/(double)clktck);
            printf("Czas systemowy: %7.4f\n", info->si_stime/(double)clktck);
            break;
        case SIGSEGV:
            printf("Handler otrzymal SIGSEGV.\n");
            printf("Miejsce w pamieci: %p\n", info->si_addr);
            break;
    }
}

int main(int argc, char** argv){

    struct sigaction act = { 0 };

    act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP | SA_RESETHAND;
    act.sa_sigaction = &handler;
    if( sigaction(SIGUSR1, &act, NULL) == -1){
        printf("Blad: Sigaction.\n");
        exit(0);
    }
    if( sigaction(SIGUSR2, &act, NULL) == -1){
        printf("Blad: Sigaction.\n");
        exit(0);
    }
    if( sigaction(SIGCHLD, &act, NULL) == -1){
        printf("Blad: Sigaction.\n");
        exit(0);
    }
    if( sigaction(SIGSEGV, &act, NULL) == -1){
        printf("Blad: Sigaction.\n");
        exit(0);
    }

    printf("\nScenariusz 1: \n");

    printf("\tProces potomny stopuje i wysyla sygnal SIGCHLD do procesu przodka,\n\tale flaga SA_NOCLDSTOP powstrzymuje obsluge sygnalu przez handler.\n\tDodatkowo wysyla swoj PID do procesu macierzystego przez signal SIGUSR2.\n");
    printf("\n");
    pid_t p = fork();
    if(p == 0){
        printf("Proces potomny stopuje\n");
        union sigval value;
        value.sival_int = getpid();
        if (sigqueue(getppid(), SIGUSR2, value) < 0) {
            printf("Blad: Kill.\n");
            exit(0);
        }
        raise(SIGSTOP);
        return 0;
    }
    sleep(1);
    printf("\nScenariusz 2: \n");

    printf("\tProces potomny zamyka sie z sygnalem SIGSEGV i wysyla sygnal SIGCHLD\n\tdo procesu przodka. Dodatkowo wykonuje czasochlonna operacje, by pokazac czas procesu.\n");
    printf("\n");
    p = fork();
    if(p == 0){
        printf("PID procesu potomnego: %d\n", getpid());
        printf("Proces potomny zamyka sie\n");
        char* z = malloc(100*sizeof (char));
        for(int i = 0; i<1000; i++){
            for(int j =0; j<1000; j++){
                for(int k = 0; k<100; k++){
                    z[k] = 'l';
                }
            }
        }
        free(z);
        char *c = "Hello";
        c[10] = 'z';
        return 0;
    }
    sleep(1);
    printf("\nScenariusz 3: \n");

    printf("\tProces wysyla sygnal SIGUSR1,\n\tnastepnie handler go obsluguje i wysyla ponownie sygnal SIGUSR1,\n\tale dzieki fladze SA_RESETHAND przywracane jest domyslne wykonanie handlera i handler nie wysyla ponownie sygnalu.\n\tWtedy sygnal SIGUSR1 jest nieobslugiwany dla zobrazowania dzialania flagi.\n");
    printf("\tPID procesu macierzystego: %d\n", getpid());
    printf("\n");
    raise(SIGUSR1);
    sleep(1);

    return 0;
}
