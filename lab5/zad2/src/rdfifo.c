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

	mkfifo(name, S_IRUSR | S_IWUSR);

	int fifoDesc = open(name, O_RDONLY, NULL);

	char msg[MSG_SIZE];

	printf("Serwer odbierajacy dane. Serwer zakonczy prace, kiedy wszyscy klienci sie rozlacza.\n");
	printf("CZAS ODBIORU            -   PID - CZAS WYSLANIA           - WIADOMOSC\n");
	
	while(read(fifoDesc, msg, MSG_SIZE) > 0) {
		printf("%23s - %s\n", currTime(), msg);
	}

	remove(name);
	return 0;
}