#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

int count = 0;
bool done = false;
bool go = false;
bool confirmed = false;
pid_t potomekPid;

void usr2Start() {
	go = true;
}

void usr2EndTransmission() {
	done = true;	
}

void usr1Receive() {
	union sigval val;
	val.sival_int = 0;

	count++;
	sigqueue(potomekPid, SIGUSR2, val);
}

void usr2Confirm() {
	confirmed = true;
}

void send(sigset_t *oldmask) {
	union sigval val;
	val.sival_int = 0;

	confirmed = false;
	sigqueue(potomekPid, SIGUSR1, val);
	while(!confirmed) {
		sigsuspend(oldmask);
	}
}

int main(int argc, const char* argv[]) {
	if(argc != 2) {
		printf("USAGE: ./przodek num_of_signals\n");
		return -1;
	}

	int numOfSignals = atoi(argv[1]);

	struct sigaction usr2StartSigaction;
	usr2StartSigaction.sa_handler = usr2Start;

	struct sigaction usr1ReceiveSigaction;
	usr1ReceiveSigaction.sa_handler = usr1Receive;

	struct sigaction usr2EndTransmissionSigaction;
	usr2EndTransmissionSigaction.sa_handler = usr2EndTransmission;

	struct sigaction usr2ConfirmSigaction;
	usr2ConfirmSigaction.sa_handler = usr2Confirm;

	sigset_t oldmask, blocked;
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGUSR1);
	sigaddset(&blocked, SIGUSR2);

	union sigval val;
	val.sival_int = 0;

	if (sigprocmask(SIG_BLOCK, &blocked, &oldmask) < 0)                      { printf("sigprocmask error."); return -1; }

	printf("Potomek PID (fill in first): ");
	scanf("%d", &potomekPid);

	if (sigaction(SIGUSR2, &usr2StartSigaction, NULL) != 0)                  { printf("sigaction error."); return -1; }
	while(!go) {
		sigsuspend(&oldmask);
	}

	printf("Przodek starting sending signals.\n");
	
	if (sigaction(SIGUSR2, &usr2ConfirmSigaction, NULL) != 0)                { printf("sigaction error."); return -1; }
	for (int i = 0; i < numOfSignals; i++) {
		send(&oldmask);
	}

	sigqueue(potomekPid, SIGUSR2, val);

	printf("Przodek finished sending signals.\n");
	printf("Przodek sent %d signals.\n", numOfSignals);

	printf("Przodek starting receiving signals.\n");
	
	if (sigaction(SIGUSR1, &usr1ReceiveSigaction, NULL) != 0)                { printf("sigaction error."); return -1; }
	if (sigaction(SIGUSR2, &usr2EndTransmissionSigaction, NULL) != 0)        { printf("sigaction error."); return -1; }	
	while(!done) {
		sigsuspend(&oldmask);
	}
		
	printf("Przodek finished receiving signals.\n");
	printf("Przodek received %d SIGUSR1 signals.\n", count);
		
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)                        { printf("sigprocmask error."); return -1; }
	return 0;

}