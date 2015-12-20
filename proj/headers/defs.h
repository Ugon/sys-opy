#ifndef DEFS_H
#define DEFS_H

#include <unistd.h>

#define MAX_LANES 10
#define MAX_TRAINS 100

#define REGISTER_SEMAPHORE 0
#define FIRST_TUNEL_SEMAPHORE 2

#define MIN_PRIORITY 0
#define MED_PRIORITY 1
#define MAX_PRIORITY 2

//informacje o pociągu
struct trainInfo {
	int id;
	pid_t pid;
	int priority;
	int trackNumber;
	int trainTime;
};

//struktura pamięci dzielonej
struct memoryStruct {
	struct trainInfo lanes[MAX_LANES][MAX_TRAINS];
	int registerIndexes[MAX_LANES];
	int enterIndexes[MAX_LANES];
	pid_t tunelPID;
	int registered;
};

//copied from manual
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

#endif
//ipcs -s | grep ugon | awk ' { print $2 } ' | xargs ipcrm sem