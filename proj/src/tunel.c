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
#include <stdlib.h>

bool registration = true;
bool allDone = false;

int memoryId, semaphoreId;
struct memoryStruct *sharedMemory;

void registrationFinishHandler(int signo) {
	registration = false;
}

int step = 1;
void allowTrain() {
	int laneToAlow = -1;
	int maxPriority = -1;

	//wybranie pociagu z najwyzszym priorytetem
	for(int lane = 0; lane < MAX_LANES; lane++){
		//Reservoir sampling to pick random train
		int samePriorities = 1;
		if(sharedMemory->enterIndexes[lane] < sharedMemory->registerIndexes[lane]) {
			if (sharedMemory->lanes[lane][sharedMemory->enterIndexes[lane]].priority > maxPriority) {
				maxPriority = sharedMemory->lanes[lane][sharedMemory->enterIndexes[lane]].priority;
				laneToAlow = lane;
				samePriorities = 1;
			} 
			else if (sharedMemory->lanes[lane][sharedMemory->enterIndexes[lane]].priority == maxPriority &&
				    (rand() % samePriorities) == (samePriorities - 1)) {
				maxPriority = sharedMemory->lanes[lane][sharedMemory->enterIndexes[lane]].priority;
				laneToAlow = lane;
				samePriorities++;
			}
		}
	}
	if (maxPriority == -1) {
		allDone = true;
	}
	else {
		//wypisanie informacji o sytuacji na torach
		printf("\n\n\nSTEP: %d\n", step++);
		for (int i = 0; i < MAX_LANES; i++) {
			printf("%02d %c ", i, i == laneToAlow ? '=' : '|');
			for (int a = sharedMemory->enterIndexes[i]; a < sharedMemory->registerIndexes[i]; a++){
				printf("[ p:%d, t:%d ] ", sharedMemory->lanes[i][a].priority, sharedMemory->lanes[i][a].trainTime);
			}
			printf("\n");
		}

		struct sembuf freeAccess;
		freeAccess.sem_num = FIRST_TUNEL_SEMAPHORE + laneToAlow;
		freeAccess.sem_op = 1;
		freeAccess.sem_flg = 0;
		//wpuszczenie pociagu do tunelu
		if (semop(semaphoreId, &freeAccess, 1) == -1)                             { printf("semop() fail, %s", strerror(errno)); return; }
		sharedMemory->enterIndexes[laneToAlow]++;
	}
}

int main() {
	signal(SIGUSR1, registrationFinishHandler);
	signal(SIGUSR2, allowTrain);
	
	
	//stworzenie pamieci wspoldzielonej
	key_t memoryKey;
	
	memoryKey = ftok(".", 'm');
	if (memoryKey == -1)                                                          { printf("ftok() fail, %s", strerror(errno)); return -1; }

	memoryId = shmget(memoryKey, sizeof(struct memoryStruct), IPC_CREAT | S_IRUSR | S_IWUSR);
	if (memoryId == -1)                                                           { printf("shmget() fail, %s", strerror(errno)); return -1; }

	sharedMemory = shmat(memoryId, NULL, 0);
	if (sharedMemory == (void *) -1)                                              { printf("shmat() fail, %s", strerror(errno)); return -1; }

	memset(sharedMemory, 0, sizeof(struct memoryStruct));

	
	//stworzenie semaforow
	key_t semaphoreKey;
	semaphoreKey = ftok(".", 's');
	if (semaphoreKey == -1)                                                       { printf("ftok() fail, %s", strerror(errno)); return -1; }

	semaphoreId = semget(semaphoreKey, MAX_LANES + 2, IPC_CREAT | S_IRUSR | S_IWUSR);
	if (semaphoreId == -1)                                                        { printf("semget() fail, %s", strerror(errno)); return -1; }
	
	union semun semaphoreArgs;
	semaphoreArgs.val = 1;
	if (semctl(semaphoreId, REGISTER_SEMAPHORE, SETVAL, semaphoreArgs) == -1)     { printf("semctl() fail, %s", strerror(errno)); return -1; }
	
	semaphoreArgs.val = 0;
	for(int i = 0; i < MAX_LANES; i++) {
		if (semctl(semaphoreId, FIRST_TUNEL_SEMAPHORE + i, SETVAL, semaphoreArgs) == -1)
                                                                                  { printf("semctl() fail, %s", strerror(errno)); return -1; }
	}

	sharedMemory->tunelPID = getpid();
	sharedMemory->registered = 0;

	printf("Starting registration\n");
	while(registration)	{
		pause();
	}
	printf("Finished registration\n");

	printf("Starting traffic\n");
	allowTrain();
	while(!allDone) {
		pause();
	}
	printf("Traffic stopped\n");


	//usuniecie pamieci wspoldzielonej i semaforow
	if (shmdt(sharedMemory) == -1)                                                { printf("shmdt() fail, %s", strerror(errno)); return -1; }
	
	if (shmctl(memoryId, IPC_RMID, NULL) == -1)                                   { printf("shmctl() fail, %s", strerror(errno)); return -1; }

	if (semctl(semaphoreId, -8, IPC_RMID) == -1)                                  { printf("semctl() fail, %s", strerror(errno)); return -1; }

	return 0;
}
