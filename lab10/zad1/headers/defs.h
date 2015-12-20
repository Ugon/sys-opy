#ifndef DEFS_H
#define DEFS_H

#define MAX_NAME_LEN 256
#define MAX_MSG_LEN 256

#define MAX_CLIENTS 64

#define NUM_OF_SERVER_SOCKETS 2
#define UNIX_POLLFD 0
#define INET_POLLFD 1

#define MSG_TYPE_REGISTER 0
#define MSG_TYPE_MSG 1
#define MSG_TYPE_PROLONG 2

#define ACCEPTABLE_INACTIVITY_PERIOD 15
#define PROLONG_FREQUENCY 10

struct msg {
	int type;
	char name[MAX_MSG_LEN];
	char txt[MAX_MSG_LEN];
};

#endif