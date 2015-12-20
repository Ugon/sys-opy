#define _GNU_SOURCE //warning: implicit declaration of function ‘asprintf’
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

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

int analyzeDir(const char *path, const char *permissions) {
	struct stat   statbuf;
	struct dirent *dent;

	if (stat(path, &statbuf) != 0)                               { printf("Stat error\n");           return -1; };
	if (S_ISDIR(statbuf.st_mode) == 0)                                                               return -1; //if trying to analize not-directory -> error

	DIR *dir = opendir(path);
	if (dir == NULL)                                             { printf("Open dir error\n");       return -1; }

	while ((dent = readdir(dir))) {
		char *filePath;
		asprintf(&filePath, "%s/%s", path, dent -> d_name);
		
		if (strcmp(dent -> d_name,  ".") == 0 ||
			strcmp(dent -> d_name, "..") == 0 ) {
			continue;
		}

		if (stat(filePath, &statbuf) != 0)                       { printf("Stat error\n");           return -1; }
		if (S_ISDIR(statbuf.st_mode) && analyzeDir(filePath, permissions) == -1)                     return -1;
		if (S_ISREG(statbuf.st_mode)) {
			if (strcmp(permissions, checkPermissions(filePath)) == 0) {
				printf("%s\t%10ldB\t%s\n", permissions, statbuf.st_size, filePath);
			}
		}
		
		free(filePath);		
	}

	if (closedir(dir) != 0)                                      { printf("Close dir error\n");      return -1; }
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
	char *permissions = argv[2];

	if(analyzeDir(path, permissions) == -1)                      { printf("Error occured\n");        return 0; }
	return 0;
}