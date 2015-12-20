#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	FILE *f1, *f2;

	if ((f1 = popen("ls -l | grep ^d", "r")) == NULL)                   { printf("popen error \n"); return -1; }
	if ((f2 = fopen("folders.txt", "w")) == NULL)                       { printf("popen error \n"); return -1; }
	
	char *buf;
	size_t n = 0;
	while (getline(&buf, &n, f1) > 0) {
		fprintf(f2, "%s", buf);
	}

	pclose(f1);
	fclose(f2);
	return 0;
}