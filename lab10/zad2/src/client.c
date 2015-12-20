#define _GNU_SOURCE
#include <stdbool.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "defs.h"

char name[MAX_NAME_LEN];

bool loop = true;
struct pollfd pollFD;
pthread_t ioThread;

void* ioFunc(void *arg) {
	struct msg toSendMsg;
	strcpy(toSendMsg.name, name);
	while(8) {
		scanf("%s", toSendMsg.txt);
		if (write(pollFD.fd, &toSendMsg, sizeof(struct msg)) == -1)                           { printf("write() fail, %s", strerror(errno)); return NULL; }
	}
}

void sigintHandler(int signo) {
	printf("SIGINT received, exiting\n");
	loop = false;
}

int main(int argc, char** argv) {
	struct sockaddr *addr;
	socklen_t addrlen;

	signal(SIGINT, sigintHandler);

	if (argc == 4 && (strcmp("unix", argv[2]) == 0)) {
		struct sockaddr_un unix_addr;
		unix_addr.sun_family = AF_UNIX;
		strcpy(unix_addr.sun_path, argv[3]);
		
		addr = (struct sockaddr*) &unix_addr;
		addrlen = sizeof(struct sockaddr_un);
	}
	else if (argc == 5 && (strcmp("inet", argv[2]) == 0)) {
		struct hostent *host;
		if ((host = gethostbyname(argv[3])) == NULL)                                          { printf("atexit() fail, %s", strerror(errno)); return -1; }
	
		struct sockaddr_in inet_addr;
		inet_addr.sin_family = AF_INET;
		inet_aton(argv[3], &(inet_addr.sin_addr));
		inet_addr.sin_addr.s_addr = ((struct in_addr *) (host->h_addr))->s_addr;
		inet_addr.sin_port = htons(atoi(argv[4]));
	
		addr = (struct sockaddr*) &inet_addr;
		addrlen = sizeof(struct sockaddr_in);
	}
	else {
		printf("   Usage: ./server <nickname> unix <unix socket path>\n");
		printf("or Usage: ./server <nickname> inet <server address> <port>\n");
		return -1; 
	}

	strcpy(name, argv[1]);

	if ((pollFD.fd = socket(addr->sa_family, SOCK_STREAM, 0)) == -1)                           { printf("socket() fail, %s", strerror(errno)); return -1; }

	int optval = 1;
	if (setsockopt(pollFD.fd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval)) == -1)         { printf("setsockopt() fail, %s", strerror(errno)); return -1; }

	if (connect(pollFD.fd, addr, addrlen) == -1)                                               { printf("connect() fail, %s", strerror(errno)); return -1; }

	pollFD.events = POLLIN;

	if (pthread_create(&ioThread, NULL, ioFunc, NULL) != 0 )                                   { printf("pthread_create() fail, %s", strerror(errno)); return -1; }

	while (loop) {
		int events = 0;
		if ((events = poll(&pollFD, 1, -1)) == 0)                                              { printf("poll() fail, no events\n"); continue; }
		else if (events == -1) {
			if (errno == EINTR) continue;
			else                                                                               { printf("pool() fail, %s", strerror(errno)); return -1; }
		}
		else if (pollFD.revents & POLLIN) {
			struct msg receivedMsg;
			if (read(pollFD.fd, &receivedMsg, sizeof(struct msg)) == -1)                       { printf("read() fail, %s", strerror(errno)); return -1; }
			printf("%s: %s\n", receivedMsg.name, receivedMsg.txt);

		}
	}
	
	if (shutdown(pollFD.fd, SHUT_RDWR) == -1 )                                                 { printf("shutdown() fail, %s", strerror(errno)); return -1; }

	if (close(pollFD.fd) == -1 )                                                               { printf("close() fail, %s", strerror(errno)); return -1; }
	
	return 0;
}