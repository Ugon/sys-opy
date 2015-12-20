#include "defs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

bool interrupted = false;

void interruptHandler(int signo) {
	interrupted = true;
}

int main() {
	signal(SIGINT, interruptHandler);
	
	key_t memoryKey, semaphoreKey;
	int memoryId, semaphoreId;
	struct memoryStruct *sharedMemory;
	struct sembuf freeAccess, gainAccess, lowerFull, raiseEmpty, startOperation[2], finishOperation[2];
	
	gainAccess.sem_num = ACCESS_SEMAPHORE;
	gainAccess.sem_op = -1;
	gainAccess.sem_flg = 0;

	freeAccess.sem_num = ACCESS_SEMAPHORE;
	freeAccess.sem_op = 1;
	freeAccess.sem_flg = 0;

	lowerFull.sem_num = FULL_SEMAPHORE;
	lowerFull.sem_op = -1;
	lowerFull.sem_flg = 0;

	raiseEmpty.sem_num = EMPTY_SEMAPHORE;
	raiseEmpty.sem_op = 1;
	raiseEmpty.sem_flg = 0;

	startOperation[0] = gainAccess;
	startOperation[1] = lowerFull;
	finishOperation[0] = freeAccess;
	finishOperation[1] = raiseEmpty;

	memoryKey = ftok(".", 'm');
	if (memoryKey == -1)                                                          { printf("ftok() fail, %s", strerror(errno)); return -1; }

	memoryId = shmget(memoryKey, sizeof(struct memoryStruct), S_IRUSR | S_IWUSR);
	if (memoryId == -1)                                                           { printf("shmget() fail, %s", strerror(errno)); return -1; }

	sharedMemory = shmat(memoryId, NULL, 0);
	if (sharedMemory == (void *) -1)                                              { printf("shmat() fail, %s", strerror(errno)); return -1; }

	semaphoreKey = ftok(".", 's');
	if (semaphoreKey == -1)                                                       { printf("ftok() fail, %s", strerror(errno)); return -1; }

	semaphoreId = semget(semaphoreKey, 3, S_IRUSR | S_IWUSR);
	if (semaphoreId == -1)                                                        { printf("semget() fail, %s", strerror(errno)); return -1; }
	   



	srand(time(NULL));
	while (!interrupted) {
		if (semop(semaphoreId, startOperation, 2) == -1)                          { printf("semop() fail, %s", strerror(errno)); return -1; }
		
		int number = rand();
		int a = sharedMemory->producerPosition;
		sharedMemory->tasks[sharedMemory->producerPosition] = number;
		sharedMemory->producerPosition = (sharedMemory->producerPosition + 1) % MAX_TASKS;
		sharedMemory->tasksAwaiting++;

		printf("(%d %d) Dodalem liczbe: %d. Liczba zadan oczekujacych: %d. index %d\n", (int) getpid(), (int) time(NULL), number, sharedMemory->tasksAwaiting, a);
		
		if (semop(semaphoreId, finishOperation, 2) == -1)                         { printf("semop() fail, %s", strerror(errno)); return -1; }

		sleep(1);
	}




	if (shmdt(sharedMemory) == -1)                                                { printf("shmdt() fail, %s", strerror(errno)); return -1; }

	return 0;
}
