#ifndef MSG_H
#define MSG_H

#define MAX_SIZE 1024
#define CLIENT_MTYPE 1
#define SERVER_MTYPE 2

const char* SERVER_QNAME = "/server_queue";

struct msg {
	long mtype;
	char mtext[1024];
};

#endif