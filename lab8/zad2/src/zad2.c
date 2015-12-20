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

void* spam(void *arg){
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

	sigset_t oldmask, blocked;
	sigemptyset(&blocked);
	if(strcmp(sigName, "SIGUSR1") == 0){
		sigaddset(&blocked, SIGUSR1);
	}
	else if(strcmp(sigName, "SIGTERM") == 0){
		sigaddset(&blocked, SIGTERM);
	}
	else if(strcmp(sigName, "SIGKILL") == 0){
		sigaddset(&blocked, SIGKILL);
	}
	else if(strcmp(sigName, "SIGSTOP") == 0){
		sigaddset(&blocked, SIGSTOP);
	}
	else{
		printf("Wrong signal name\n");
		return -1;
	}

	if (pthread_sigmask(SIG_BLOCK, &blocked, &oldmask) < 0)                  { printf("pthread_sigmask() fail, %s", strerror(errno)); return -1; }

	sleep(3);

	printf("\n\n\n");
	if(strcmp(sigName, "SIGUSR1") == 0){
		printf("Raising SIGUSR1\n");
		raise(SIGUSR1);
	}
	else if(strcmp(sigName, "SIGTERM") == 0){
		printf("Raising SIGTERM\n");
		raise(SIGTERM);
	}
	else if(strcmp(sigName, "SIGKILL") == 0){
		printf("Raising SIGKILL\n");
		raise(SIGKILL);
	}
	else if(strcmp(sigName, "SIGSTOP") == 0){
		printf("Raising SIGSTOP\n");
		raise(SIGSTOP);
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