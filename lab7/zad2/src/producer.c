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




	srand(time(NULL));
	while (1) {
		if (sem_wait(fullSemaphore) == -1)                                        { printf("sem_wait() fail, %s", strerror(errno)); return -1; }
		if (sem_wait(accessSemaphore) == -1)                                      { printf("sem_wait() fail, %s", strerror(errno)); return -1; }
		
		int number = rand();
		sharedMemory->tasks[sharedMemory->producerPosition] = number;
		sharedMemory->producerPosition = (sharedMemory->producerPosition + 1) % MAX_TASKS;
		sharedMemory->tasksAwaiting++;

		printf("(%d %d) Dodalem liczbe: %d. Liczba zadan oczekujacych: %d\n", (int) getpid(), (int) time(NULL), number, sharedMemory->tasksAwaiting);
		
		if (sem_post(accessSemaphore) == -1)                                      { printf("sem_wait() fail, %s", strerror(errno)); return -1; }
		if (sem_post(emptySemaphore) == -1)                                       { printf("sem_wait() fail, %s", strerror(errno)); return -1; }

		sleep(1);
	}


	

	if(sem_close(accessSemaphore) == -1)                                          { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if(sem_close(fullSemaphore) == -1)                                            { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if(sem_close(emptySemaphore) == -1)                                           { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if (munmap(sharedMemory, sizeof(struct memoryStruct)) == -1)                  { printf("munmap() fail, %s", strerror(errno)); return -1; }

	return 0;
}
