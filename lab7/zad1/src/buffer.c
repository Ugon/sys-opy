#include "defs.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

bool interrupted = false;

void interruptHandler(int signo) {
	interrupted = true;
}

int main() {
	signal(SIGINT, interruptHandler);
	
	key_t memoryKey, semaphoreKey;
	int memoryId, semaphoreId;
	struct memoryStruct *sharedMemory;
	
	memoryKey = ftok(".", 'm');
	if (memoryKey == -1)                                                          { printf("ftok() fail, %s", strerror(errno)); return -1; }

	memoryId = shmget(memoryKey, sizeof(struct memoryStruct), IPC_CREAT | S_IRUSR | S_IWUSR);
	if (memoryId == -1)                                                           { printf("shmget() fail, %s", strerror(errno)); return -1; }

	sharedMemory = shmat(memoryId, NULL, 0);
	if (sharedMemory == (void *) -1)                                              { printf("shmat() fail, %s", strerror(errno)); return -1; }

	memset(sharedMemory, 0, sizeof(struct memoryStruct));

	semaphoreKey = ftok(".", 's');
	if (semaphoreKey == -1)                                                       { printf("ftok() fail, %s", strerror(errno)); return -1; }

	semaphoreId = semget(semaphoreKey, 3, IPC_CREAT | S_IRUSR | S_IWUSR);
	if (semaphoreId == -1)                                                        { printf("semget() fail, %s", strerror(errno)); return -1; }
	
	union semun semaphoreArgs;
	semaphoreArgs.val = 1;
	if (semctl(semaphoreId, ACCESS_SEMAPHORE, SETVAL, semaphoreArgs) == -1)       { printf("semctl() fail, %s", strerror(errno)); return -1; }
	
	semaphoreArgs.val = MAX_TASKS;
	if (semctl(semaphoreId, FULL_SEMAPHORE, SETVAL, semaphoreArgs) == -1)         { printf("semctl() fail, %s", strerror(errno)); return -1; }
	
	semaphoreArgs.val = 0;
	if (semctl(semaphoreId, EMPTY_SEMAPHORE, SETVAL, semaphoreArgs) == -1)        { printf("semctl() fail, %s", strerror(errno)); return -1; }

	


	printf("Buffer ready to accept and distribute tasks.\n");
	while(!interrupted){
		pause();
	}



	if (shmdt(sharedMemory) == -1)                                                { printf("shmdt() fail, %s", strerror(errno)); return -1; }
	
	if (shmctl(memoryId, IPC_RMID, NULL) == -1)                                   { printf("shmctl() fail, %s", strerror(errno)); return -1; }

	if (semctl(semaphoreId, -8, IPC_RMID) == -1)                                  { printf("semctl() fail, %s", strerror(errno)); return -1; }

	return 0;
}
