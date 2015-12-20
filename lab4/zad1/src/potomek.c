#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

int count = 0;
bool done = false;

void usr2EndTransmission() {
	done = true;	
}

void usr1usr1Receive() {
	count++;
}

int main(int argc, const char* argv[]) {
	struct sigaction usr1ReceiveSigaction;
	usr1ReceiveSigaction.sa_handler = usr1usr1Receive;

	struct sigaction usr2EndTransmissionSigaction;
	usr2EndTransmissionSigaction.sa_handler = usr2EndTransmission;

	if (sigaction(SIGUSR1, &usr1ReceiveSigaction, NULL) != 0)             { printf("sigaction error."); return -1; }
	if (sigaction(SIGUSR2, &usr2EndTransmissionSigaction, NULL) != 0)     { printf("sigaction error."); return -1; }

	pid_t przodekPid;

	printf("Przodek PID (fill in second): ");
	scanf("%d", &przodekPid);

	kill(przodekPid, SIGUSR2);
	
	printf("Potomek starting receiving signals.\n");
	
	while(!done) {
		pause();
	}
	
	printf("Potomek finished receiving signals.\n");
	printf("Potomek received %d SIGUSR1 signals.\n", count);
	printf("Potomek starting sending signals back.\n");

	for (int i = 0; i < count; i++) {
		kill(przodekPid, SIGUSR1);
	}

	kill(przodekPid, SIGUSR2);
	printf("Potomek finished sending signals back.\n");

	return 0;

}