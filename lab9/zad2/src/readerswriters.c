#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>
#include <time.h>

#define NUM_OF_READERS 10
#define NUM_OF_WRITERS 4
#define NUM_OF_THREADS (NUM_OF_READERS + NUM_OF_WRITERS)

#define gettid() syscall(SYS_gettid)

#define TAB_SIZE 1000
#define MIN_NUM 0
#define MAX_NUM 10000
#define MIN_QNT 10
#define RAND_NUM (rand() % (MAX_NUM - MIN_NUM) + MIN_NUM)
#define RAND_QNT (rand() % (TAB_SIZE - MIN_QNT) + MIN_QNT)
#define RAND_POS (rand() % TAB_SIZE)

int globalTab[TAB_SIZE];

struct WaitingThread {
	bool isReader;
	sem_t *semaphore;
};

struct Queue{
	struct WaitingThread *tab[NUM_OF_THREADS];
	int firstIndex;
	int lastIndex;
	int size;
	sem_t *queueSemaphore;
}*queue;

struct Queue* initQueue(int size){
	struct Queue *q = (struct Queue*) malloc(size * sizeof(struct WaitingThread));
	q->firstIndex = 0;
	q->lastIndex = 0;
	q->size = size;
	q->queueSemaphore = (sem_t*) malloc(sizeof(sem_t));

	sem_init(q->queueSemaphore, 0, 1);

	return q;
}

struct WaitingThread* popReader(struct Queue *q){
	sem_wait(q->queueSemaphore);
	int p = q->firstIndex;
	if(q->tab[p] == NULL){
		sem_post(q->queueSemaphore);
		return NULL;
	}
	if(q->tab[p]->isReader) {
		struct WaitingThread *temp = q->tab[p];
		q->tab[p] = NULL;
		q->firstIndex = (p + 1) % q->size;
		sem_post(q->queueSemaphore);
		return temp;
	}

	sem_post(q->queueSemaphore);
	return NULL;
}

struct WaitingThread* popNext(struct Queue *q){
	sem_wait(q->queueSemaphore);

	int p = q->firstIndex;
	if(q->tab[p] == NULL){
		sem_post(q->queueSemaphore);
		return NULL;
	}
	struct WaitingThread *temp = q->tab[p];
	q->tab[p] = NULL;
	q->firstIndex = (p + 1) % q->size;

	sem_post(q->queueSemaphore);
	return temp;	
}

void push(struct Queue *q, struct WaitingThread *elem){
	sem_wait(q->queueSemaphore);

	if (q->tab[q->lastIndex] != NULL)                                                { printf("Queue overflow\n"); return; }
	q->tab[q->lastIndex] = elem;
	q->lastIndex = (q->lastIndex + 1) % q->size;

	sem_post(q->queueSemaphore);
}

pthread_t threads[NUM_OF_THREADS];

int readersInLibraryCount = 0;
pthread_mutex_t readersInLibraryCountMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wakingReadersMutex = PTHREAD_MUTEX_INITIALIZER;

void readInLibrary(){
	printf("Reader %ld starting reading   <    \n", gettid());
	
	usleep(rand() % 250000 + 5000);
	int position = -1;
	int i = 0;
	int toFind = RAND_NUM;
	while(position == -1 && i < TAB_SIZE){
		if(globalTab[i] == toFind){
			position = i;
		}
		i++;
	}
	
	printf("Reader %ld finished reading   >    ", gettid());
	if(position == -1){
		printf("Number %d not found\n", toFind);
	}
	else{
		printf("Number %d found at position %d\n", toFind, position);
	}
}

void writeInLibrary(){
	printf("Writer %ld starting writing <      \n", gettid());
	
	int qnt = RAND_QNT;
	for(int i = 0; i < qnt; i++){
		globalTab[RAND_POS] = RAND_NUM;
	}
	usleep(rand() % 250000 + 5000);
	
	printf("Writer %ld finished writing >      Modified %d numbers\n", gettid(), qnt);
}

void* readerFunc(void *arg){
	struct WaitingThread *this = (struct WaitingThread*) arg;
	
	while(8){
		sem_wait(this->semaphore);
		pthread_mutex_lock(&readersInLibraryCountMutex);
		readersInLibraryCount++;
		pthread_mutex_unlock(&readersInLibraryCountMutex);

		//Awake all other waiting readers
		pthread_mutex_lock(&wakingReadersMutex);
		struct WaitingThread *next = popReader(queue);
		while(next != NULL){
			sem_post(next->semaphore);
			next = popReader(queue);
		}
		pthread_mutex_unlock(&wakingReadersMutex);

		readInLibrary();

		push(queue, this);

		pthread_mutex_lock(&readersInLibraryCountMutex);
		readersInLibraryCount--;
		if(readersInLibraryCount == 0) {
			struct WaitingThread *writer = popNext(queue);
			sem_post(writer->semaphore);

		}
		pthread_mutex_unlock(&readersInLibraryCountMutex);
	}
	return NULL;
}

void* writerFunc(void *arg){
	struct WaitingThread *this = (struct WaitingThread*) arg;
	
	while(8){
		sem_wait(this->semaphore);
		
		writeInLibrary();

		push(queue, this);
		sem_post(popNext(queue)->semaphore);
	}
}

void createReader(int index){
	struct WaitingThread *thread = (struct WaitingThread*) malloc(sizeof(struct WaitingThread));
	thread->isReader = true;
	thread->semaphore = (sem_t*) malloc(sizeof(sem_t));
	sem_init(thread->semaphore, 0, 0);
	push(queue, thread);
	pthread_create(&threads[index], NULL, readerFunc, thread);
}

void createWriter(int index){
	struct WaitingThread *thread = (struct WaitingThread*) malloc(sizeof(struct WaitingThread));
	thread->isReader = false;
	thread->semaphore = (sem_t*) malloc(sizeof(sem_t));
	sem_init(thread->semaphore, 0, 0);
	push(queue, thread);
	pthread_create(&threads[index], NULL, writerFunc, thread);
}

int main(){
	srand(time(NULL));

	for(int i = 0; i < TAB_SIZE; i++){
		globalTab[i] = RAND_NUM;
	}

	queue = initQueue(NUM_OF_THREADS);

	int readersMade = 0;
	int writersMade = 0;
	for(int i = 0; i < NUM_OF_THREADS; i++){
		if(rand() % 2 == 0 && readersMade < NUM_OF_READERS){
			readersMade++;
			createReader(i);
		}
		else{
			writersMade++;
			createWriter(i);
		}
	}

	sem_post(popNext(queue)->semaphore);

	for(int i = 0; i < NUM_OF_READERS; i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}

