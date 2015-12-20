#include "defs.h"
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

bool isPrime(int number) {
	for(int i = 2; i < number; i++){
		if(number % i == 0){
			return false;
		}
	}
	return true;
}

int main() {
	int sharedMemoryDescriptor;
	struct memoryStruct *sharedMemory;

	sem_t *accessSemaphore, *fullSemaphore, *emptySemaphore;

	sharedMemoryDescriptor = shm_open(MEMORY_NAME, O_RDWR, S_IRUSR | S_IWUSR);
	if (sharedMemoryDescriptor == -1)                                             { printf("sem_open() fail, %s", strerror(errno)); return -1; }
	
	sharedMemory = mmap(NULL, sizeof(struct memoryStruct), PROT_WRITE, MAP_SHARED, sharedMemoryDescriptor, 0);
	if (sharedMemory == MAP_FAILED)                                               { printf("sem_open() fail, %s", strerror(errno)); return -1; }

	accessSemaphore = sem_open(ACCESS_SEMAPHORE, 0);
	if (accessSemaphore == SEM_FAILED)                                            { printf("sem_open() fail, %s", strerror(errno)); return -1; }

	fullSemaphore = sem_open(FULL_SEMAPHORE, 0);
	if (fullSemaphore == SEM_FAILED)                                              { printf("sem_open() fail, %s", strerror(errno)); return -1; }

	emptySemaphore = sem_open(EMPTY_SEMAPHORE, 0);
	if (emptySemaphore == SEM_FAILED)                                             { printf("sem_open() fail, %s", strerror(errno)); return -1; }




	while (1) {
		if (sem_wait(emptySemaphore) == -1)                                       { printf("sem_wait() fail, %s", strerror(errno)); return -1; }
		if (sem_wait(accessSemaphore) == -1)                                      { printf("sem_wait() fail, %s", strerror(errno)); return -1; }
		
		int number = sharedMemory->tasks[sharedMemory->consumerPosition];
		sharedMemory->consumerPosition = (sharedMemory->consumerPosition + 1) % MAX_TASKS;
		sharedMemory->tasksAwaiting--;

		printf("(%d %d) Sprawdzilem liczbe %d - %s. Pozostalo zadan oczekujacych: %d\n",
			(int) getpid(), (int) time(NULL), number, isPrime(number) ? "pierwsza" : "zlozona", sharedMemory->tasksAwaiting);
		
		if (sem_post(accessSemaphore) == -1)                                      { printf("sem_wait() fail, %s", strerror(errno)); return -1; }
		if (sem_post(fullSemaphore) == -1)                                        { printf("sem_wait() fail, %s", strerror(errno)); return -1; }
		
		sleep(1);
	}


	

	if(sem_close(accessSemaphore) == -1)                                          { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if(sem_close(fullSemaphore) == -1)                                            { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if(sem_close(emptySemaphore) == -1)                                           { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if (munmap(sharedMemory, sizeof(struct memoryStruct)) == -1)                  { printf("munmap() fail, %s", strerror(errno)); return -1; }

	return 0;
}