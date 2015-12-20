#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

volatile int count = 0;
volatile bool done = false;
volatile bool go = false;

void usr2Start() {
	go = true;
}

void usr1Receive() {
	count++;
}

void usr2EndTransmission() {
	done = true;	
}

int main(int argc, const char* argv[]) {
	if(argc != 2)                                                            { printf("USAGE: ./przodek num_of_signals\n"); return -1; }

	int numOfSignals = atoi(argv[1]);

	struct sigaction usr2StartSigaction;
	usr2StartSigaction.sa_handler = usr2Start;

	struct sigaction usr1ReceiveSigaction;
	usr1ReceiveSigaction.sa_handler = usr1Receive;

	struct sigaction usr2EndTransmissionSigaction;
	usr2EndTransmissionSigaction.sa_handler = usr2EndTransmission;

	union sigval val;
	val.sival_int = 0;

	sigset_t oldmask, blocked;
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGRTMIN);
	sigaddset(&blocked, SIGRTMIN + 1);

	if (sigaction(SIGRTMIN + 1, &usr2StartSigaction, NULL) != 0)             { printf("sigaction error."); return -1; }

	pid_t potomekPid;

	printf("Potomek PID (fill in first): ");
	scanf("%d", &potomekPid);

	if (sigprocmask(SIG_BLOCK, &blocked, &oldmask) < 0)                      { printf("sigprocmask error."); return -1; }
	while(!go) {
		sigsuspend(&oldmask);
	}

	printf("Przodek starting sending signals.\n");
	
	for (int i = 0; i < numOfSignals; i++) {
		sigqueue(potomekPid, SIGRTMIN, val);
	}

	sigqueue(potomekPid, SIGRTMIN + 1, val);

	printf("Przodek finished sending signals.\n");
	printf("Przodek sent %d signals.\n", numOfSignals);

	if (sigaction(SIGRTMIN, &usr1ReceiveSigaction, NULL) != 0)               { printf("sigaction error."); return -1; }
	if (sigaction(SIGRTMIN + 1, &usr2EndTransmissionSigaction, NULL) != 0)   { printf("sigaction error."); return -1; }
	
	printf("Przodek starting receiving signals.\n");
		
	while(!done) {
		sigsuspend(&oldmask);
	}
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)                        { printf("sigprocmask error."); return -1; }
		
	printf("Przodek finished receiving signals.\n");
	printf("Przodek received %d SIGRTMIN signals.\n", count);
		
	return 0;
}