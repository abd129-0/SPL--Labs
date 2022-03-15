#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>


void signalHandle(int sig){
	fprintf(stderr, "Recieved signal: %s\n", strsignal(sig));

	if(sig == SIGCONT)
		signal(SIGTSTP, signalHandle);
	if(sig == SIGTSTP){
		signal(SIGINT, signalHandle);
		signal(SIGCONT, signalHandle);
	}
	signal(sig, SIG_DFL);
	raise(sig);
}

int main(int argc, char **argv){ 
	printf("Starting the program: %d\n", getpid());

	signal(SIGTSTP, signalHandle);
	signal(SIGCONT, signalHandle);
	signal(SIGINT, signalHandle);

	while(1) {
		sleep(2);
	}

	return 0;
}