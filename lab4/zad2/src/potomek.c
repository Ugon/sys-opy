#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

int count = 0;
bool done = false;
bool confirmed = false;
pid_t przodekPid;

void usr2EndTransmission() {
	done = true;	
}

void usr1Receive() {
	count++;
	kill(przodekPid, SIGUSR2);
}

void usr2Confirm() {
	confirmed = true;
}

void send(sigset_t *oldmask) {
	confirmed = false;
	kill(przodekPid, SIGUSR1);
	while(!confirmed) {
		sigsuspend(oldmask);
	}
}

int main(int argc, const char* argv[]) {
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

	if (sigprocmask(SIG_BLOCK, &blocked, &oldmask) < 0)                         { printf("sigprocmask error."); return -1; }

	printf("Przodek PID (fill in second): ");
	scanf("%d", &przodekPid);

	kill(przodekPid, SIGUSR2); //tell przodek to start sending signals
	
	printf("Potomek starting receiving signals.\n");
	
	if (sigaction(SIGUSR1, &usr1ReceiveSigaction, NULL) != 0)                   { printf("sigaction error."); return -1; }
	if (sigaction(SIGUSR2, &usr2EndTransmissionSigaction, NULL) != 0)           { printf("sigaction error."); return -1; }	
	while(!done) {
		sigsuspend(&oldmask);
	}
	
	printf("Potomek finished receiving signals.\n");
	printf("Potomek received %d SIGUSR1 signals.\n", count);
	printf("Potomek starting sending signals back.\n");

	if (sigaction(SIGUSR2, &usr2ConfirmSigaction, NULL) != 0)                   { printf("sigaction error."); return -1; }
	for (int i = 0; i < count; i++) {
		send(&oldmask);
	}

	kill(przodekPid, SIGUSR2);
	printf("Potomek finished sending signals back.\n");

	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)                           { printf("sigprocmask error."); return -1; }
	return 0;
}