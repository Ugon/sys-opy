#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

int count = 0;
bool done = false;
bool go = false;

void usr2Start() {
	go = true;
}

void usr2EndTransmission() {
	done = true;	
}

void usr1Receive() {
	count++;
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

	if (sigaction(SIGUSR2, &usr2StartSigaction, NULL) != 0)                  { printf("sigaction error."); return -1; }

	pid_t potomekPid;

	printf("Potomek PID (fill in first): ");
	scanf("%d", &potomekPid);

	while(!go) {
		pause();
	}

	printf("Przodek starting sending signals.\n");
	
	for (int i = 0; i < numOfSignals; i++) {
		kill(potomekPid, SIGUSR1);
	}

	kill(potomekPid, SIGUSR2);
	printf("Przodek finished sending signals.\n");
	printf("Przodek sent %d signals.\n", numOfSignals);

	if (sigaction(SIGUSR1, &usr1ReceiveSigaction, NULL) != 0)                { printf("sigaction error."); return -1; }
	if (sigaction(SIGUSR2, &usr2EndTransmissionSigaction, NULL) != 0)        { printf("sigaction error."); return -1; }
	
	printf("Przodek starting receiving signals.\n");
		
	while(!done) {
		pause();
	}
		
	printf("Przodek finished receiving signals.\n");
	printf("Przodek received %d SIGUSR1 signals.\n", count);
		
	return 0;

}