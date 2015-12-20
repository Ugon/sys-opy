#define _BSD_SOURCE //warning: implicit declaration of function ‘realpath’
#include <ftw.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

char* checkPermissions(const char *path) {
	struct stat statbuf;
	stat(path, &statbuf);
	
	char *permissions = malloc(11 * sizeof(char));
	permissions[0] = (S_ISDIR(statbuf.st_mode))   ? 'd' : '-';
	permissions[1] = (statbuf.st_mode & S_IRUSR)  ? 'r' : '-';    
	permissions[2] = (statbuf.st_mode & S_IWUSR)  ? 'w' : '-';     
	permissions[3] = (statbuf.st_mode & S_IXUSR)  ? 'x' : '-';     
	permissions[4] = (statbuf.st_mode & S_IRGRP)  ? 'r' : '-';     
	permissions[5] = (statbuf.st_mode & S_IWGRP)  ? 'w' : '-';     
	permissions[6] = (statbuf.st_mode & S_IXGRP)  ? 'x' : '-';     
	permissions[7] = (statbuf.st_mode & S_IROTH)  ? 'r' : '-';     
	permissions[8] = (statbuf.st_mode & S_IWOTH)  ? 'w' : '-';     
	permissions[9] = (statbuf.st_mode & S_IXOTH)  ? 'x' : '-';     
	permissions[10] = '\0';

	return permissions;
}

char *perms;
int fn (const char *filePath, const struct stat *sb, int typeflag) {
	if (strcmp(perms, checkPermissions(filePath)) == 0) {
		char *rpath = malloc(512* sizeof(char));
		realpath(filePath, rpath);
		printf("%s\n", rpath);
		free(rpath);
	}
	return 0;
}

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("This prgram will analize normal filed in gived directory and return list of files that match given access permissions.\n");
		printf("Argument 1: path.\n");
		printf("Argument 2: access permissions (drwxrwxrwx).\n");
		return 0;
	}

	char *path = argv[1];
	perms = argv[2];

	if(ftw(path, &fn, 10) != 0) printf("Error occured\n");
	return 0;
}