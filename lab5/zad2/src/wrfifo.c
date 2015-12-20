#define _GNU_SOURCE //asprintf

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#define MSG_SIZE 1024

char *currTime() {
	time_t rawtime;
	struct tm *timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	char* formatted = asctime(timeinfo);
	formatted[strlen(formatted) - 2] = '\0';
	return formatted;
}

int main(int argc, char* argv[]) {
	if (argc != 2) { printf("USAGE: ./rdfifo name\n"); return -1; }

	char *name = argv[1];

	int fifoDesc = open(name, O_WRONLY, O_APPEND);

	char msg[MSG_SIZE];
	char quitwr[8] = "quit-wr\0";

	printf("Klient wysylajacy wiadomosci. Klient zakonczy prace jesli sprobojemy nadac wiadomosc: \"quit-wr\"\n");
	printf("Wiadomosc: ");
	scanf("%s", msg);
	while(strcmp(msg, quitwr)) {
		char *toSend;
		asprintf(&toSend, "%5d - %23s - %s", (int) getpid(), currTime(), msg);
		write(fifoDesc, toSend, MSG_SIZE);
		memset(msg, 0, MSG_SIZE);
		free(toSend);
		printf("Wiadomosc: ");
		scanf("%s", msg);
	}

	close(fifoDesc);
	return 0;
}