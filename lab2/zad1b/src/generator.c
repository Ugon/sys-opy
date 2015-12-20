#define _BSD_SOURCE  //warning: implicit declaration of function ‘fchmod’
#define _XOPEN_SOURCE 500

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	if(argc != 4){
		printf("This prgram will generate a file filled with random bytes.\n");
		printf("Argument 1: file name.\n");
		printf("Argument 2: record size.\n");
		printf("Argument 3: number of records.\n");
	}

	char *fileName = argv[1];
	int recordSize = atoi(argv[2]);
	int recordNum = atoi(argv[3]);

	int descriptor = open(fileName, O_WRONLY | O_CREAT | O_TRUNC);

	srand(time(NULL));

	char *line = malloc((recordSize) * sizeof(unsigned char));
	for (int i = 0; i < recordNum; i++) {
		for (int j = 0; j < recordSize - 1; j++) {
			line[j] = (unsigned char) (rand() % 94 + 33);
		}
		line[recordSize - 1] = '\n';
		write(descriptor, line, recordSize);
	}
	free(line);

	fchmod(descriptor, S_IRWXU | S_IRWXG | S_IRWXO);
	close(descriptor);

	return 0;
}