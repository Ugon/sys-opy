#define _BSD_SOURCE  //warning: implicit declaration of function ‘fchmod’
#define _XOPEN_SOURCE 500

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

#define RECORD_SIZE 1024

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("This prgram will generate a file filled with records.\n");
		printf("Argument 1: file name.\n");
		printf("Argument 2: number of records.\n");
		return -1;
	}

	char *fileName = argv[1];
	int recordNum = atoi(argv[2]);

	int descriptor = open(fileName, O_WRONLY | O_CREAT | O_TRUNC);

	srand(time(NULL));

	char *line = malloc(RECORD_SIZE - sizeof(int));
	for (int i = 0; i < recordNum; i++) {
		write(descriptor, &i, sizeof(int));
		for (int j = 0; j < RECORD_SIZE - sizeof(int) - 1; j++) {
			line[j] = (unsigned char) (rand() % 5 /*26*/ + 97); //a b c d e letters only
		}
		line[RECORD_SIZE - sizeof(int) - 1] = '\0';
		write(descriptor, line, RECORD_SIZE - sizeof(int));
	}
	free(line);

	fchmod(descriptor, S_IRUSR | S_IWUSR);
	close(descriptor);

	return 0;
}