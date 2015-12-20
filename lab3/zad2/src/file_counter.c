#define _GNU_SOURCE 500 //warning: implicit declaration of function ‘asprintf’
#include <stdlib.h>
#include <stdbool.h>
#include <ftw.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int analyzeDir(const char *progname, const char *path, bool wait) {
	struct stat   statbuf;
	struct dirent *dent;

	if (stat(path, &statbuf) != 0)                               { printf("Stat error\n");           return -1; };
	if (S_ISDIR(statbuf.st_mode) == 0)                                                               return -1; //if trying to analize not-directory -> error

	DIR *dir = opendir(path);
	if (dir == NULL)                                             { printf("Open dir error\n");       return -1; }
	
	int counter = 0;
	int lastChild = 0;
	pid_t *children = malloc(100 * sizeof(pid_t));

	while ((dent = readdir(dir))) {
		if (strcmp(dent -> d_name,  ".") == 0 ||
			strcmp(dent -> d_name, "..") == 0 ) {
			continue;
		}

		char *filePath;
		asprintf(&filePath, "%s/%s", path, dent -> d_name);

		if (stat(filePath, &statbuf) != 0)                       { printf("Stat error\n");           return -1; }
		if (S_ISREG(statbuf.st_mode)) counter++;
		if (S_ISDIR(statbuf.st_mode)) {
			children[lastChild] = vfork();
			if (children[lastChild] < 0)                         { printf("fork error\n");           return -1; }
			else if (children[lastChild] == 0) {
				if (execl(progname, progname, filePath, wait ? "-w" : NULL, NULL) == -1)
					                                             { printf("Exec error\n");           return -1; } }
			else lastChild++;
		}
		free(filePath);		
	}

	if (wait) sleep(15);

	for(int i = 0; i < lastChild; i++) {
		int exitStatus;
		waitpid(children[i], &exitStatus, 0 /*options*/);
		counter += WEXITSTATUS(exitStatus);
	}

	if (closedir(dir) != 0)                                      { printf("Close dir error\n");      return -1; }
	
	return counter;
}


int main (int argc, char *argv[]) {
	if (argc < 2 || argc > 3) {
		printf("USAGE: ./file_counter path [-w]");
		return -1; 
	}

	bool wait = false;
	char *progname;
	char *path;

	progname = argv[0];
	path = argv[1];
	if (argc == 3 && strcmp(argv[2], "-w") == 0) {
		wait = true;
	}

	int ret = analyzeDir(progname, path, wait);

#ifdef PARTIAL_RESULTS
	printf("%4d files in directory: \t%s\n", ret, path);
#endif

	_exit(ret);
}