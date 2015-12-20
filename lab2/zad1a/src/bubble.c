#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void swap(unsigned char **a, unsigned char **b) {
	unsigned char *temp = *a;
	*a = *b;
	*b = temp;
}

int main(int argc, char *argv[]){
	if(argc != 4){
		printf("This prgram will sort a file filled with records.\n");
		printf("Argument 1: file name.\n");
		printf("Argument 2: record size.\n");
		printf("Argument 3: number of records.\n");
	}

	char *fileName = argv[1];
	int recordSize = atoi(argv[2]);
	int recordNum = atoi(argv[3]);

	int descriptor = open(fileName, O_RDWR);
	if (descriptor == -1)                                                          { printf("Open error\n");  return 0; }

	unsigned char *smaller = malloc(recordSize * sizeof(unsigned char));
	unsigned char *bigger  = malloc(recordSize * sizeof(unsigned char));

	for(int last = recordNum - 1; last > 0; last--) {
		if (lseek(descriptor, 0, SEEK_SET) == -1)                                  { printf("Seek error\n");  return 0; }
		if (read(descriptor, bigger, recordSize) != recordSize)                    { printf("Read error\n");  return 0; }
		for (int j = 1; j <= last; j++) {
			if (read(descriptor, smaller, recordSize) != recordSize)               { printf("Read error\n");  return 0; }
			if ((int) smaller[0] > (int) bigger[0]) swap(&smaller, &bigger);
			if (lseek(descriptor, - 2 * recordSize, SEEK_CUR) == -1)               { printf("Seek error\n");  return 0; }
			if (write(descriptor, smaller, recordSize) != recordSize)              { printf("Write error\n"); return 0; }
			if (lseek(descriptor, recordSize, SEEK_CUR) == -1)                     { printf("Seek error\n");  return 0; }
		}
		if (lseek(descriptor, -recordSize, SEEK_CUR) == -1)                        { printf("Seek error\n");  return 0; }
		if (write(descriptor, bigger, recordSize) != recordSize)                   { printf("Write error\n"); return 0; }
	}

	if (close(descriptor) != 0)                                                    { printf("Close error\n"); return 0; }

	return 0;
}