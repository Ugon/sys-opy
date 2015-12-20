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

#define RECORD_SIZE 1024

#define gettid() syscall(SYS_gettid)

int numberOfThreads;
char *fileName;
int numberOfRecordsRead;
char *wordToFind;

int fileDescriptor;

pthread_t *threads;

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t countMutex = PTHREAD_MUTEX_INITIALIZER;

int count = 0;

void* searchRecord(void *wtf /*wordToFind*/){
	if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0)                  { printf("pthread_setcancelstate() fail, %s", strerror(errno)); return NULL; } 
	char *buff = malloc(RECORD_SIZE - sizeof(int));

	if(pthread_mutex_lock(&waitMutex) != 0)                                        { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return NULL; }
	if(pthread_mutex_unlock(&waitMutex) != 0)                                      { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }

	while(8) {
		if(pthread_mutex_lock(&fileMutex) != 0)                                    { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return NULL; }
		
		for(int r = 0; r < numberOfRecordsRead; r++) {		
			int recordId;
			int bytesRead = read(fileDescriptor, &recordId, sizeof(int));
			if(bytesRead == -1)                                                    { printf("read() fail, %s", strerror(errno)); return NULL; }
	
			if(bytesRead == 0) {
				if(pthread_mutex_unlock(&fileMutex) != 0)                          { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }
				goto done;
			}
	
			if(read(fileDescriptor, buff, RECORD_SIZE - sizeof(int)) == -1)        { printf("read() fail, %s", strerror(errno)); return NULL; }
			
			char *foundWord = strstr(buff, wtf);
	
			if(foundWord != NULL){
				printf("TID: %ld, record ID: %d\n", gettid(), recordId);
			}	
		}
		
		if(pthread_mutex_unlock(&fileMutex) != 0)                                  { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }
	}

	done:
	free(buff);
	
	if(pthread_mutex_lock(&countMutex) != 0)                                       { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return NULL; }
	count--;
	if(pthread_mutex_unlock(&countMutex) != 0)                                     { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }

	return NULL;
}

int main(int argc, char **argv){
	if(argc != 5)                                                                 
		{ printf("Usage: ./finder numberOfThreads fileName numberOfRecordsRead wordToFind\n"); return -1; }

	numberOfThreads = atoi(argv[1]);
	fileName = argv[2];
	numberOfRecordsRead = atoi(argv[3]);
	wordToFind = argv[4];

	threads = malloc( numberOfThreads * sizeof(pthread_t) );

	fileDescriptor = open(fileName, O_RDONLY);
	if(fileDescriptor == -1)                                                       { printf("open() fail, %s", strerror(errno)); return -1; }

	if(pthread_mutex_lock(&waitMutex) != 0)                                        { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return -1; }

	pthread_attr_t attr;
	if (pthread_attr_init(&attr) != 0)                                             { printf("pthread_attr_init() fail, %s", strerror(errno)); return -1; }
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)          { printf("pthread_attr_setdetachstate() fail, %s", strerror(errno)); return -1; }

	if(pthread_mutex_lock(&countMutex) != 0)                                       { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return -1; }
	for(int i = 0; i < numberOfThreads; i++){
		if(pthread_create(&(threads[i]), &attr, searchRecord, wordToFind) != 0)    { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
		count++;
	}
	if(pthread_mutex_unlock(&countMutex) != 0)                                     { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return -1; }

	if(pthread_mutex_unlock(&waitMutex) != 0)                                      { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return -1; }

	while(count != 0);

	free(threads);
	close(fileDescriptor);
	
	return 0;
}