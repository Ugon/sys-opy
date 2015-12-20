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

void cleanup(){
	if(pthread_mutex_unlock(&fileMutex) != 0)                                      { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); }
}

void* searchRecord(void *wtf /*wordToFind*/){
	if(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) != 0)                  { printf("pthread_setcanceltype() fail, %s", strerror(errno)); return NULL; }
	pthread_cleanup_push(cleanup, NULL);

	char *buff = malloc(RECORD_SIZE - sizeof(int));

	if(pthread_mutex_lock(&waitMutex) != 0)                                        { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return NULL; }
	if(pthread_mutex_unlock(&waitMutex) != 0)                                      { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }

	while(8) {
		if(pthread_mutex_lock(&fileMutex) != 0)                                    { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return NULL; }
		
		pthread_testcancel();

		int cancel_state;
		if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state) != 0)     { printf("pthread_setcancelstate() fail, %s", strerror(errno)); return NULL; } 
		
		for(int r = 0; r < numberOfRecordsRead; r++) {
			
			int recordId;
			int bytesRead = read(fileDescriptor, &recordId, sizeof(int));
			if(bytesRead == -1)                                                    { printf("read() fail, %s", strerror(errno)); return NULL; }
	
			if(bytesRead == 0) {
				for(int i = 0; i < numberOfThreads; i++){
					pthread_cancel(threads[i]);
				}
			}
	
			if(read(fileDescriptor, buff, RECORD_SIZE - sizeof(int)) == -1)        { printf("read() fail, %s", strerror(errno)); return NULL; }
			
			char *foundWord = strstr(buff, wtf);
	
			if(foundWord != NULL){	
				for(int i = 0; i < numberOfThreads; i++){
					pthread_cancel(threads[i]);
				}
				
				printf("TID: %ld, record ID: %d\n", gettid(), recordId);
			}	
		}
		
		if(pthread_setcancelstate(cancel_state, NULL) != 0)                        { printf("pthread_setcancelstate() fail, %s", strerror(errno)); return NULL; } 

		if(pthread_mutex_unlock(&fileMutex) != 0)                                  { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }
	}

	free(buff);

	pthread_cleanup_pop(1);
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

	for(int i = 0; i < numberOfThreads; i++){
		if((pthread_create(&(threads[i]), NULL, searchRecord, wordToFind)) != 0)   { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
	}

	if(pthread_mutex_unlock(&waitMutex) != 0)                                      { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return -1; }

	for(int i = 0; i < numberOfThreads; i++){
		if(pthread_join(threads[i], NULL) != 0)                                    { printf("pthread_join() fail, %s", strerror(errno)); return -1; }
	}

	free(threads);
	close(fileDescriptor);
	return 0;
}