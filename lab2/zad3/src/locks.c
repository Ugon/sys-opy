#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[]) {
	if (argc != 2)                                          { printf("Usage: ./locks filename\n"); return 0; }
	
	char *fileName = argv[1];
	int descriptor = open(fileName, O_RDWR);
	if (descriptor == -1)                                   { printf("Open error\n");              return 0; }

	printf("This program lets you play with locks in a file.\n");
	printf("Command list:\n");
	printf("showlocks                                  - prints a list of currentlu locked characters\n");
	printf("lock      character_position [read, write] - locks a character on a given position\n");
	printf("unlock    character_position               - unlocks a character on a given position\n");
	printf("read      character_position               - reads a character on a given position\n");
	printf("write     character_position char          - writes a character to a given position\n");
	printf("exit                                       - exit\n");

	char command[100];
	char extra[100];
	int position;
	struct flock flk;
	long last;

	last = lseek(descriptor, 0, SEEK_END);

	for(printf("> "), scanf("%s", command); strcmp(command, "exit") != 0; printf("> "), scanf("%s", command)) {
		if (strcmp("lock", command) == 0) {
			scanf("%d %s", &position, extra);
			if      (position > last)                           { printf("Out of file\n");   continue; }
			if      (strcmp("read",  extra) == 0) flk.l_type = F_RDLCK;
			else if (strcmp("write", extra) == 0) flk.l_type = F_WRLCK;
			else                                                { printf("Wrong command\n"); continue; }
			
			flk.l_start = position;
			flk.l_whence = SEEK_SET;
			flk.l_len = 1;

			if ( fcntl(descriptor, F_SETLK, &flk) == -1 )         printf("FAIL\n");
			else                                                  printf("OK\n");
		}

		else if (strcmp("unlock",    command) == 0) {
			scanf("%d", &position);
			if (position > last)                                { printf("Out of file\n");   continue; }

			flk.l_type = F_UNLCK;
			flk.l_start = position;
			flk.l_whence = SEEK_SET;
			flk.l_len = 1;

			if ( fcntl(descriptor, F_SETLK, &flk) == -1 )         printf("FAIL\n");
			else                                                  printf("OK\n");
		}

		else if (strcmp("showlocks", command) == 0) {
			for (long i = 0; i <= last; i++) {
				flk.l_type = F_WRLCK;
				flk.l_start = i;
				flk.l_len = 1;
				flk.l_whence = SEEK_SET;

				if ( fcntl(descriptor, F_GETLK, &flk) == -1 )   { printf("Error occured\n"); continue; }
				
				if (flk.l_type != F_UNLCK) 
					printf("pos: %3ld\t\ttype: %s\t\tpid: %d\n", i, (flk.l_type == F_RDLCK ? "read " : "write"), flk.l_pid);
			}
		}

		else if (strcmp("read",      command) == 0) {
			scanf("%d", &position);
			if (position > last)                                { printf("Out of file\n");   continue; }
			char ch;
			if (lseek(descriptor, position, SEEK_SET) == -1)    { printf("Seek error\n");    continue; }
            if (read(descriptor, &ch, 1) != 1)      { printf("Read error\n");    continue; }
            printf("%c\n", ch);
		}

		else if (strcmp("write",     command) == 0) {
			char ch;
			scanf("%d %c", &position, &ch);
			if (position > last)                                { printf("Out of file\n");   continue; }
			if (lseek(descriptor, position, SEEK_SET) == -1)    { printf("Seek error\n");    continue; }
            if (write(descriptor, &ch, 1) != 1)     { printf("Write error\n");   continue; }
		}

		else {
			printf("Wrong command\n");
		}
	}
}