#define MAX_TASKS 10

#define MEMORY_NAME "/MEMORY_NAME"

#define ACCESS_SEMAPHORE "/ACCESS_SEMAPHORE"
#define FULL_SEMAPHORE "/FULL_SEMAPHORE"
#define EMPTY_SEMAPHORE "/EMPTY_SEMAPHORE"

struct memoryStruct {
	double tasks[MAX_TASKS];
	int producerPosition;
	int consumerPosition;
	int tasksAwaiting;
};