#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void signal_handler(int signal_number)
{
	int num1 = 55;
	int num2 = 0;
	int num3 = 30;
    int *p = NULL;
	switch (signal_number) {
		case SIGCHLD:
			printf("SIGCHLD\n");
			alarm(1);  
            sleep(1);
            fflush(stdout);
            break; 
        case SIGALRM:
            printf("SIGALRM\n");
            *p = 10;
            fflush(stdout);
            break;
        case SIGSEGV:
            printf("SIGSEGV\n");
            raise(SIGUSR1);
            fflush(stdout);
            break;    
		case SIGUSR1: 
			printf("SIGUSR1\n");
			fflush(stdout);
			num3++;
			num3 = num1 / num2;
			fflush(stdout);
			break;
		case SIGFPE:
			printf("SIGFPE\n");
			fflush(stdout);
            exit(1);
            break; 
    }
}

int main()
{
	int status;
	signal (SIGCHLD, signal_handler); 
    signal(SIGALRM, signal_handler);
    signal(SIGSEGV, signal_handler);
	signal (SIGUSR1, signal_handler);
	signal (SIGFPE, signal_handler);

	if (!(fork())) { 
		exit(1);
	}
	wait(&status);
}

