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
#include <sys/wait.h>

#define MIN_TRAIN_TIME 2
#define MAX_TRAIN_TIME 4

int main(int argc, char** argv) {
	if (argc != 3)                                                                { printf("Usage: ./traffic numOfLanes numOfTrains\n"); return -1; }

	int numOfLanes = atoi(argv[1]);
	int numOfTrains = atoi(argv[2]);


	//inicjalizacja pamieci wspoldzielonej
	key_t memoryKey;
	int memoryId;
	struct memoryStruct *sharedMemory;
	
	memoryKey = ftok(".", 'm');
	if (memoryKey == -1)                                                          { printf("ftok() fail, %s", strerror(errno)); return -1; }

	memoryId = shmget(memoryKey, sizeof(struct memoryStruct), S_IRUSR | S_IWUSR);
	if (memoryId == -1)                                                           { printf("shmget() fail, %s", strerror(errno)); return -1; }

	sharedMemory = shmat(memoryId, NULL, 0);
	if (sharedMemory == (void *) -1)                                              { printf("shmat() fail, %s", strerror(errno)); return -1; }


	//inicjalizacja semaforow
	key_t semaphoreKey;
	int semaphoreId;
	struct sembuf gainAccess, freeRegister, gainRegister;

	gainAccess.sem_op = -1;
	gainAccess.sem_flg = 0;

	gainRegister.sem_num = REGISTER_SEMAPHORE;
	gainRegister.sem_op = -1;
	gainRegister.sem_flg = 0;

	freeRegister.sem_num = REGISTER_SEMAPHORE;
	freeRegister.sem_op = 1;
	freeRegister.sem_flg = 0;

	semaphoreKey = ftok(".", 's');
	if (semaphoreKey == -1)                                                       { printf("ftok() fail, %s", strerror(errno)); return -1; }

	semaphoreId = semget(semaphoreKey, MAX_LANES + 2, S_IRUSR | S_IWUSR);
	if (semaphoreId == -1)                                                        { printf("semget() fail, %s", strerror(errno)); return -1; }


	//rejestracja pociagow
	int tunelPID = sharedMemory->tunelPID;
	pid_t *childPIDS = malloc(numOfTrains * sizeof(pid_t));

	for (int i = 0; i < numOfTrains; i++) {
		pid_t pid = fork();
		if (pid == -1)                                                            { printf("fork() fail, %s", strerror(errno)); return -1; }
		else if (pid == 0) {
			//child process - proces nowego pociagu
			srand(getpid());
			
			//rozpoczenie zglaszania checi wjazdu do tunelu
			if (semop(semaphoreId, &gainRegister, 1) == -1)                       { printf("semop() fail, %s", strerror(errno)); return -1; }
						
			int lane = rand() % numOfLanes;

			//utworzenie nowego pociagu			
			struct trainInfo info;
			info.pid = getpid();
			info.trackNumber = lane;
			info.priority = rand() % (MAX_PRIORITY + 1);
			info.trainTime = MIN_TRAIN_TIME + rand() % (MAX_TRAIN_TIME - MIN_TRAIN_TIME + 1);
			
			//zapisanie informacji o oczekiwaniu w pamieci wspoldzielonej
			sharedMemory->lanes[lane][sharedMemory->registerIndexes[lane]] = info;
			sharedMemory->registerIndexes[lane]++;
			sharedMemory->registered++;

			printf("Registering train %d\t for lane %d\t with priority %d.\n", info.pid, info.trackNumber, info.priority);

			//zakonczenie zglaszania checi wjazdu do tunelu
			if (semop(semaphoreId, &freeRegister, 1) == -1)                       { printf("semop() fail, %s", strerror(errno)); return -1; }

			//oczekiwanie na otwarcie semaforu na torze przez nadzorce
			gainAccess.sem_num = FIRST_TUNEL_SEMAPHORE + lane;
			if (semop(semaphoreId, &gainAccess, 1) == -1)                         { printf("semop() fail, %s", strerror(errno)); return -1; }
			
			//przejazd pociagu
			printf("Tain %d\t on lane %d\t with priority %d\t STARTING.\n", info.pid, info.trackNumber, info.priority);
			sleep(info.trainTime);
			printf("Tain %d\t on lane %d\t with priority %d\t FINISHED.\n", info.pid, info.trackNumber, info.priority);

			//poinformowanie nadzorcy o zwolnieniu tunelu
			kill(tunelPID, SIGUSR2);
			
			exit(0);
		}
		else {
			//parent process - dopisanie nowego pociagu do tablicy.
			childPIDS[i] = pid;
		}
	}
	while(sharedMemory->registered < numOfTrains) sleep(1);
	printf("Registration finished.\n");
	kill(tunelPID, SIGUSR1);

	//oczekiwanie na przejazd wszystkich pociagow
	printf("Traffic starting\n");
	for (int i = 0; i < numOfTrains; i++) {
		waitpid(childPIDS[i], NULL, 0);
	}
	
	printf("Traffic finished\n");
	if (shmdt(sharedMemory) == -1)                                                { printf("shmdt() fail, %s", strerror(errno)); return -1; }

	return 0;
}
