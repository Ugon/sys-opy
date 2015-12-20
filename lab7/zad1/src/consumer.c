#include "defs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

bool interrupted = false;

bool isPrime(int number) {
	for(int i = 2; i < number; i++){
		if(number % i == 0){
			return false;
		}
	}
	return true;
}

void interruptHandler(int signo) {
	interrupted = true;
}

int main() {
	signal(SIGINT, interruptHandler);
	
	key_t memoryKey, semaphoreKey;
	int memoryId, semaphoreId;
	struct memoryStruct *sharedMemory;
	struct sembuf freeAccess, gainAccess, raiseFull, lowerEmpty, startOperation[2], finishOperation[2];
	
	gainAccess.sem_num = ACCESS_SEMAPHORE;
	gainAccess.sem_op = -1;
	gainAccess.sem_flg = 0;

	freeAccess.sem_num = ACCESS_SEMAPHORE;
	freeAccess.sem_op = 1;
	freeAccess.sem_flg = 0;

	lowerEmpty.sem_num = EMPTY_SEMAPHORE;
	lowerEmpty.sem_op = -1;
	lowerEmpty.sem_flg = 0;

	raiseFull.sem_num = FULL_SEMAPHORE;
	raiseFull.sem_op = 1;
	raiseFull.sem_flg = 0;

	startOperation[0] = gainAccess;
	startOperation[1] = lowerEmpty;
	finishOperation[0] = freeAccess;
	finishOperation[1] = raiseFull;

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
	   



	while (!interrupted) {
		if (semop(semaphoreId, startOperation, 2) == -1)                          { printf("semop() fail, %s", strerror(errno)); return -1; }
		
		int number = sharedMemory->tasks[sharedMemory->consumerPosition];
		sharedMemory->consumerPosition = (sharedMemory->consumerPosition + 1) % MAX_TASKS;
		sharedMemory->tasksAwaiting--;

		printf("(%d %d) Sprawdzilem liczbe %d - %s. Pozostalo zadan oczekujacych: %d\n",
			(int) getpid(), (int) time(NULL), number, isPrime(number) ? "pierwsza" : "zlozona", sharedMemory->tasksAwaiting);
		
		if (semop(semaphoreId, finishOperation, 2) == -1)                         { printf("semop() fail, %s", strerror(errno)); return -1; }

		sleep(1);
	}




	if (shmdt(sharedMemory) == -1)                                                { printf("shmdt() fail, %s", strerror(errno)); return -1; }

	return 0;
}
