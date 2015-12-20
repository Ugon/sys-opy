#include <stdio.h>
#include <stdlib.h>

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
	
	FILE *file = fopen(fileName, "r+");
	if (file == NULL)                                                                   { printf("Open error\n");  return 0; }

	unsigned char *smaller = malloc(recordSize * sizeof(unsigned char));
	unsigned char *bigger  = malloc(recordSize * sizeof(unsigned char));

	for(int last = recordNum - 1; last > 0; last--) {
		if (fseek(file, 0, SEEK_SET) != 0)                                              { printf("Seek error\n");  return 0; }
		if (fread(bigger, sizeof(unsigned char), recordSize, file) != recordSize)       { printf("Read error\n");  return 0; }
		for (int j = 1; j <= last; j++) {
			if (fread(smaller, sizeof(unsigned char), recordSize, file) != recordSize)  { printf("Read error\n");  return 0; }
			if ((int) smaller[0] > (int) bigger[0]) swap(&smaller, &bigger);
			if (fseek(file, - 2 * recordSize, SEEK_CUR) != 0)                           { printf("Seek error\n");  return 0; }
			if (fwrite(smaller, sizeof(unsigned char), recordSize, file) != recordSize) { printf("Write error\n"); return 0; }
			if (fseek(file, recordSize, SEEK_CUR) != 0)                                 { printf("Seek error\n");  return 0; }
		}
		if (fseek(file, -recordSize, SEEK_CUR) != 0)                                    { printf("Seek error\n");  return 0; }
		if (fwrite(bigger, sizeof(unsigned char), recordSize, file) != recordSize)      { printf("Write error\n"); return 0; }
	}

	if (fclose(file) != 0)                                                              { printf("Close error\n"); return 0; }

	return 0;
}