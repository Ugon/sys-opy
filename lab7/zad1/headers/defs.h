#define MAX_TASKS 10

#define ACCESS_SEMAPHORE 0
#define FULL_SEMAPHORE 1
#define EMPTY_SEMAPHORE 2

struct memoryStruct {
	double tasks[MAX_TASKS];
	int producerPosition;
	int consumerPosition;
	int tasksAwaiting;
};

//copied from manual
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};