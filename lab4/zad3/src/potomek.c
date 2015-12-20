#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

volatile int count = 0;
volatile bool done = false;

void usr1Receive() {
	count++;
}

void usr2EndTransmission() {
	done = true;	
}

int main(int argc, const char* argv[]) {
	struct sigaction usr1ReceiveSigaction;
	usr1ReceiveSigaction.sa_sigaction = usr1Receive;
	usr1ReceiveSigaction.sa_flags = SA_SIGINFO;

	struct sigaction usr2EndTransmissionSigaction;
	usr2EndTransmissionSigaction.sa_handler = usr2EndTransmission;

	union sigval val;
	val.sival_int = 0;

	sigset_t oldmask, blocked;
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGRTMIN);
	sigaddset(&blocked, SIGRTMIN + 1);

	if (sigaction(SIGRTMIN, &usr1ReceiveSigaction, NULL) != 0)                { printf("sigaction error."); return -1; }
	if (sigaction(SIGRTMIN + 1, &usr2EndTransmissionSigaction, NULL) != 0)    { printf("sigaction error."); return -1; }

	pid_t przodekPid;

	printf("Przodek PID (fill in second): ");
	scanf("%d", &przodekPid);

	sigqueue(przodekPid, SIGRTMIN + 1, val);
	
	printf("Potomek starting receiving signals.\n");
	
	if (sigprocmask(SIG_BLOCK, &blocked, &oldmask) < 0)                      { printf("sigprocmask error."); return -1; }
	while(!done) {
		sigsuspend(&oldmask);
	}
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)                        { printf("sigprocmask error."); return -1; }
	
	printf("Potomek finished receiving signals.\n");
	printf("Potomek received %d SIGRTMIN signals.\n", count);
	printf("Potomek starting sending signals back.\n");

	for (int i = 0; i < count; i++) {
		sigqueue(przodekPid, SIGRTMIN, val);
	}
	sigqueue(przodekPid, SIGRTMIN + 1, val);

	printf("Potomek finished sending signals back.\n");

	return 0;

}