#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define NUM_OF_PHILOS 5
#define NUM_OF_EATS 10

pthread_t threads[NUM_OF_PHILOS];

pthread_mutex_t accessMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t philoCondVar[NUM_OF_PHILOS];
bool forkInUse[NUM_OF_PHILOS];

void* philoFunc(void *arg){
	int philoNum = *((int*) arg);

	int leftFork = philoNum;
	int rightFork = (philoNum + 1) % NUM_OF_PHILOS;

	int eatsLeft = NUM_OF_EATS;
	while(eatsLeft > 0){
		if (pthread_mutex_lock(&accessMutex) != 0)                              { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return NULL; }

		while(forkInUse[leftFork] || forkInUse[rightFork]){
			if (pthread_cond_wait(&philoCondVar[philoNum], &accessMutex) != 0)  { printf("pthread_cond_wait() fail, %s", strerror(errno)); return NULL; };
		}

		forkInUse[leftFork] = true;
		forkInUse[rightFork] = true;

		printf("Philosopher %d is eating\n", philoNum);
		if (pthread_mutex_unlock(&accessMutex) != 0)                            { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }
		
		eatsLeft--;
		usleep(rand() % 10000 + 1500);
		
		if (pthread_mutex_lock(&accessMutex) != 0)                              { printf("pthread_mutex_lock() fail, %s", strerror(errno)); return NULL; }
		printf("Philosopher %d is not eating anymore\n", philoNum);

		forkInUse[leftFork] = false;
		forkInUse[rightFork] = false;

		if (pthread_cond_broadcast(&philoCondVar[(NUM_OF_PHILOS + philoNum - 1) % NUM_OF_PHILOS]) != 0)
			                                                                    { printf("pthread_cond_broadcast() fail, %s", strerror(errno)); return NULL; }
		if (pthread_cond_broadcast(&philoCondVar[(philoNum + 1) % NUM_OF_PHILOS]) != 0 )
		                                                                        { printf("pthread_cond_broadcast() fail, %s", strerror(errno)); return NULL; }

		if (pthread_mutex_unlock(&accessMutex) != 0)                            { printf("pthread_mutex_unlock() fail, %s", strerror(errno)); return NULL; }
	}

	return NULL;
}


int main(){
	for(int i = 0; i < NUM_OF_PHILOS; i++){
		if (pthread_cond_init(&philoCondVar[i], NULL) != 0)                      { printf("pthread_cond_init() fail, %s", strerror(errno)); return -1; }
		forkInUse[i] = false;
	}

	for(int i = 0; i < NUM_OF_PHILOS; i++){
		int *philoNum = (int*) malloc(sizeof(int));
		*philoNum = i;
		if (pthread_create(&threads[i], NULL, philoFunc, philoNum) != 0 )        { printf("pthread_create() fail, %s", strerror(errno)); return -1; }
	}

	for(int i = 0; i < NUM_OF_PHILOS; i++){
		if (pthread_join(threads[i], NULL) != 0 )                                { printf("pthread_join() fail, %s", strerror(errno)); return -1; }
	}

	return 0;
}

