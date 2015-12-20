#include "defs.h"
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

bool interrupted = false;

void interruptHandler(int signo) {
	interrupted = true;
}

int main() {
	signal(SIGINT, interruptHandler);
	
	int sharedMemoryDescriptor;
	struct memoryStruct *sharedMemory;

	sem_t *accessSemaphore, *fullSemaphore, *emptySemaphore;

	sharedMemoryDescriptor = shm_open(MEMORY_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (sharedMemoryDescriptor == -1)                                             { printf("shm_open() fail, %s", strerror(errno)); return -1; }

	if (ftruncate(sharedMemoryDescriptor, sizeof(struct memoryStruct)) == -1)     { printf("ftruncate() fail, %s", strerror(errno)); return -1; }

	sharedMemory = mmap(NULL, sizeof(struct memoryStruct), PROT_WRITE | PROT_READ, MAP_SHARED, sharedMemoryDescriptor, 0);
	if (sharedMemory == MAP_FAILED)                                               { printf("mmap() fail, %s", strerror(errno)); return -1; }

	memset(sharedMemory, 0, sizeof(struct memoryStruct));

	accessSemaphore = sem_open(ACCESS_SEMAPHORE, O_CREAT, S_IRUSR | S_IWUSR, 1);
	if (accessSemaphore == SEM_FAILED)                                            { printf("sem_open() fail, %s", strerror(errno)); return -1; }

	fullSemaphore = sem_open(FULL_SEMAPHORE, O_CREAT, S_IRUSR | S_IWUSR, MAX_TASKS);
	if (fullSemaphore == SEM_FAILED)                                              { printf("sem_open() fail, %s", strerror(errno)); return -1; }

	emptySemaphore = sem_open(EMPTY_SEMAPHORE, O_CREAT, S_IRUSR | S_IWUSR, 0);
	if (emptySemaphore == SEM_FAILED)                                             { printf("sem_open() fail, %s", strerror(errno)); return -1; }
	



	printf("Buffer ready to accept and distribute tasks.\n");
	while(!interrupted){
		pause();
	}




	if(sem_close(accessSemaphore) == -1)                                          { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if(sem_close(fullSemaphore) == -1)                                            { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if(sem_close(emptySemaphore) == -1)                                           { printf("sem_close() fail, %s", strerror(errno)); return -1; }
	
	if (sem_unlink(ACCESS_SEMAPHORE) == -1)                                       { printf("sem_unlink() fail, %s", strerror(errno)); return -1; }

	if (sem_unlink(FULL_SEMAPHORE) == -1)                                         { printf("sem_unlink() fail, %s", strerror(errno)); return -1; }

	if (sem_unlink(EMPTY_SEMAPHORE) == -1)                                        { printf("sem_unlink() fail, %s", strerror(errno)); return -1; }

	if (munmap(sharedMemory, sizeof(struct memoryStruct)) == -1)                  { printf("munmap() fail, %s", strerror(errno)); return -1; }
	
	if (shm_unlink(MEMORY_NAME) == -1)                                            { printf("shm_unlink() fail, %s", strerror(errno)); return -1; }

	return 0;
}
