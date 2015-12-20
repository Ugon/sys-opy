#define _GNU_SOURCE //asprintf

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>

char getDirDescription(const char *rootpath, const char *filename) {
	char *path;
	asprintf(&path, "%s/%s", rootpath, filename);
	
	struct stat statbuf;
	stat(path, &statbuf);
	
	return (S_ISDIR(statbuf.st_mode)) ? 'd' : '-';
}

void ls(const char *rootpath, const int output) {
	DIR *dir = opendir(rootpath);
	if (dir == NULL)                                             { printf("Open dir error\n"); }
	
	struct dirent *dent;
	while ((dent = readdir(dir))) {
		char *fileName = dent -> d_name;
		
		if (fileName[0] == '.') continue;

		char dir = getDirDescription(rootpath, fileName);
		write(output, &dir, sizeof(char));
	}

	if (closedir(dir) != 0)                                      { printf("Close dir error\n"); }

}

void grep(const int input, const int output) {
	char ch;
	
	while (read(input, &ch, 1)) {
		if (ch == 'd') {
			char c = '1';
			write(output, &c, sizeof(char));
		}
	}
}

void counter(const int input, const int output) {
	int count = 0;
	char ch;
	while (read(input, &ch, sizeof(char))) {
		if(ch == '1') count++;
	}
	write(output, &count, sizeof(int));
}

void m(const int input) {
	int number;
	read(input, &number, sizeof(int));
	printf("%d\n", number);
}

int main() {
	pid_t lsPID, grepPID, counterPID;
	int lsToGrep[2], grepToCounter[2], counterToMain[2];

	if (pipe(counterToMain) < 0)                                         { printf("pipe error\n");      return -1; }
	if (pipe(grepToCounter) < 0)                                         { printf("pipe error\n");      return -1; }
	if (pipe(lsToGrep) < 0)                                              { printf("pipe error\n");      return -1; }
	
	if ((counterPID = fork()) < 0)                                       { printf("fork error\n");      return -1; }
	else if (counterPID == 0) {	
//COUNTER
		close(counterToMain[0]);
		close(grepToCounter[1]);
		close(lsToGrep[0]);
		close(lsToGrep[1]);

		counter(grepToCounter[0], counterToMain[1]);

		close(grepToCounter[0]);
		close(counterToMain[1]);
	}
	else {
		if ((grepPID = fork()) < 0)                                      { printf("fork error\n");      return -1; }
		else if (grepPID == 0) {
//GREP
			close(counterToMain[0]);
			close(counterToMain[1]);
			close(grepToCounter[0]);
			close(lsToGrep[1]);

			grep(lsToGrep[0], grepToCounter[1]);
			
			close(lsToGrep[0]);
			close(grepToCounter[1]);
			}
		else {
			if ((lsPID = fork()) < 0)                                    { printf("fork error\n");      return -1; }
			else if (lsPID == 0) {
//LS				
				close(counterToMain[0]);
				close(counterToMain[1]);
				close(grepToCounter[0]);
				close(grepToCounter[1]);
				close(lsToGrep[0]);

				char path[1024];
				getcwd(path, 1024);
				ls(path, lsToGrep[1]);

				close(lsToGrep[1]);
			}
			else {
//MAIN
				close(counterToMain[1]);
				close(grepToCounter[0]);
				close(grepToCounter[1]);
				close(lsToGrep[0]);
				close(lsToGrep[1]);

				m(counterToMain[0]);

				close(counterToMain[0]);
			}
		}
	}
}
