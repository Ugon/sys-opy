#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>

#define gettid() syscall(SYS_gettid)

int numberOfThreads;
char* sigName;

pthread_t *threads;

bool done = false;

pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;

void signalHandler(int signo){
	printf("PID: %d, TID: %ld received signal %d\n", (int) getpid(), gettid(), signo);
}

void* spam(void *arg){
	if(SIG_ERR == signal(SIGUSR1, signalHandler))                            { printf("signal() fail, %s", strerror(errno)); return NULL; }
	if(SIG_ERR == signal(SIGTERM, signalHandler))                            { printf("signal() fail, %s", strerror(errno)); return NULL; }

	int iteration = 0;
	while(8) {
		printf("%02d | TID running: %lu\n", iteration++, gettid());
		sleep(1);
	}

	return NULL;
}

int main(int argc, char **argv){
	if(argc != 3)                                                                 
		{ printf("Usage: ./finder numberOfThreads sigName\n"); return -1; }

	numberOfThreads = atoi(argv[1]);
	sigName = argv[2];

	threads = malloc( numberOfThreads * sizeof(pthread_t) );

	for(int i = 0; i < numberOfThreads; i++){
		if(pthread_create(&(threads[i]), NULL, spam, NULL) != 0)             { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
	}

	sleep(3);

	printf("\n\n\n");
	if(strcmp(sigName, "SIGUSR1") == 0){
		printf("Killing SIGUSR1\n");
		for(int i = 0; i < numberOfThreads; i++){
			if(i % 2 == 0){
				if(pthread_kill(threads[i], SIGUSR1) != 0)             { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
			}
		}
	}
	else if(strcmp(sigName, "SIGTERM") == 0){
		printf("Killing SIGTERM\n");
		for(int i = 0; i < numberOfThreads; i++){
			if(i % 2 == 0){
				if(pthread_kill(threads[i], SIGTERM) != 0)             { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
			}
		}
	}
	else if(strcmp(sigName, "SIGKILL") == 0){
		printf("Killing SIGKILL\n");
		for(int i = 0; i < numberOfThreads; i++){
			if(i % 2 == 0){
				if(pthread_kill(threads[i], SIGKILL) != 0)             { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
			}
		}
	}
	else if(strcmp(sigName, "SIGSTOP") == 0){
		printf("Killing SIGSTOP\n");
		for(int i = 0; i < numberOfThreads; i++){
			if(i % 2 == 0){
				if(pthread_kill(threads[i], SIGSTOP) != 0)             { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
			}
		}
	}
	else{
		printf("Wrong signal name\n");
	}
	printf("\n\n\n");

	sleep(1);
	printf("M  | TID running: %lu\n", gettid());
	sleep(2);
	
	return 0;
}