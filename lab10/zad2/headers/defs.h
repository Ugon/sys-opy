#ifndef DEFS_H
#define DEFS_H

#define BACKLOG_SIZE 10

#define MAX_NAME_LEN 256
#define MAX_MSG_LEN 256

#define MAX_CLIENTS 64

#define NUM_OF_SERVER_SOCKETS 2

struct msg {
	char name[MAX_MSG_LEN];
	char txt[MAX_MSG_LEN];
};

#endif