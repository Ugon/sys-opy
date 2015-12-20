#define _GNU_SOURCE
#include <stdbool.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "defs.h"

struct client {
	char *name;
	int fd;
	time_t time;
	struct sockaddr *addr;
	socklen_t addrlen;
} *clients[MAX_CLIENTS];

bool loop = true;
int numOfClients = 0;

char socketPath[256];
int portNumber;

pthread_t kickingThread;

int findClientByName(char *name) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0) {
			return i;
		}
	}
	return -1;
}

void broadcast(char *senderName, struct msg m) {
	int i;
	if (strcmp("server", senderName) == 0) {
		i = -2;
	}
	else {
		i = findClientByName(senderName);
	}
	for (int j = 0; j < MAX_CLIENTS; j++) {
		if (clients[j] != NULL && i != j) {
			sendto(clients[j]->fd, &m, sizeof(struct msg), 0, clients[j]->addr, clients[j]->addrlen);
		}
	}
}

struct client* registerClient(char *name, int fd, struct sockaddr *addr, socklen_t addrlen) {
	int i = 0;
	while(i < MAX_CLIENTS && clients[i] != NULL) i++;

	if (i == MAX_CLIENTS)                                                                     { printf("Client overflow\n"); return NULL; }

	struct msg joinMsg;
	joinMsg.type = MSG_TYPE_MSG;
	strcpy(joinMsg.name, "server");
	sprintf(joinMsg.txt, "Client joined: %s", name);
	printf("%s\n", joinMsg.txt);
	broadcast("server", joinMsg);

	if ((clients[i] = malloc(sizeof(struct client))) == NULL)                                 { printf("malloc() fail, %s", strerror(errno)); return NULL; }
	if ((clients[i]->name = malloc(strlen(name) + 1)) == NULL)                                { printf("malloc() fail, %s", strerror(errno)); return NULL; }
	strcpy(clients[i]->name, name);
	clients[i]->fd = fd;
	clients[i]->addr = addr;
	clients[i]->addrlen = addrlen;
	clients[i]->time = time(NULL);
	numOfClients++;

	return clients[i];
}

void prolongClient(char *name){
	int i = findClientByName(name);
	if (i == -1)                                                                              { printf("No such client"); return; }
	clients[i]->time = time(NULL);
}

void* kickingFunc(void *arg){
	while(loop) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (clients[i] != NULL && (time(NULL) - clients[i]->time) > ACCEPTABLE_INACTIVITY_PERIOD) {
				struct client *client = clients[i];
				clients[i] = NULL;
				
				struct msg kickMsg;
				kickMsg.type = MSG_TYPE_MSG;
				strcpy(kickMsg.name, "server");
				sprintf(kickMsg.txt, "Client kicked out (timeout): %s", client->name);
				printf("%s\n", kickMsg.txt);
				broadcast("server", kickMsg);

				free(client->name);
				free(client->addr);
				free(client);
				numOfClients--;
			}
		}
		sleep(10);
	}
	return NULL;
}

void sigintHandler(int signo) {
	printf("SIGINT received, exiting\n");
	loop = false;
}

void cleanup() {
	if (socketPath != NULL) {
		if (unlink(socketPath) == -1)                                                         { printf("unlink() fail, %s", strerror(errno)); return ; }
	}
}

int main(int argc, char** argv) {
	struct pollfd poolFDs[NUM_OF_SERVER_SOCKETS];

	if (argc != 3)                                                                            { printf("Usage: ./server <unix socket path> <port number>\n"); return -1; }

	strcpy(socketPath, argv[1]);
	portNumber = atoi(argv[2]);

	if (atexit(cleanup) != 0)                                                                 { printf("atexit() fail, %s", strerror(errno)); return -1; }
	if (signal(SIGINT, sigintHandler) == SIG_ERR)                                             { printf("signal() fail, %s", strerror(errno)); return -1; }

	if (pthread_create(&kickingThread, NULL, kickingFunc, NULL) != 0 )                        { printf("pthread_create() fail, %s", strerror(errno)); return -1; }


	//AF_UNIX socket
	struct sockaddr_un unix_addr;
	unix_addr.sun_family = AF_UNIX;
	strcpy(unix_addr.sun_path, socketPath);

	if ((poolFDs[UNIX_POLLFD].fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)                     { printf("socket() fail, %s", strerror(errno)); return -1; }

	if ((bind(poolFDs[UNIX_POLLFD].fd, &unix_addr, sizeof(unix_addr))) == -1)                 { printf("bind() fail, %s", strerror(errno)); return -1; }

	poolFDs[UNIX_POLLFD].events = POLLIN;


	//AF_INET socket
	struct sockaddr_in inet_addr;
	inet_addr.sin_family = AF_INET;
	inet_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_addr.sin_port = htons(portNumber);

	if ((poolFDs[INET_POLLFD].fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)                     { printf("socket() fail, %s", strerror(errno)); return -1; }

	if ((bind(poolFDs[INET_POLLFD].fd, &inet_addr, sizeof(inet_addr))) == -1)                 { printf("bind() fail, %s", strerror(errno)); return -1; }

	poolFDs[INET_POLLFD].events = POLLIN;


	while (loop) {
		int events = 0;
		if ((events = poll(poolFDs, NUM_OF_SERVER_SOCKETS, -1)) == 0)                         { printf("pool() fail, no events\n"); continue; }
		else if (events == -1) {
			if (errno == EINTR) continue;
			else                                                                              { printf("pool() fail, %s", strerror(errno)); return -1; }
		}
		else {
			for(int sockIndex = 0; sockIndex < NUM_OF_SERVER_SOCKETS; sockIndex++) {
				if (poolFDs[sockIndex].revents & POLLIN) {
					struct msg msg;
					struct sockaddr *addr;
					socklen_t addrlen;

					if (sockIndex == INET_POLLFD) {
						addrlen = sizeof(struct sockaddr_in);
					}
					else if (sockIndex == UNIX_POLLFD) {
						addrlen = sizeof(struct sockaddr_un);
					}
					else                                                                      { printf("Received a message from unknown socket.\n"); continue; }

					if ((addr = malloc(addrlen)) == NULL)                                     { printf("malloc() fail, %s", strerror(errno)); continue; }
	
					if ((recvfrom(poolFDs[UNIX_POLLFD].fd, &msg, sizeof(struct msg), 0, addr, &addrlen)) == -1) 
						                                                                      { printf("recvfrom() fail, %s", strerror(errno)); continue; }
	
					if (msg.type == MSG_TYPE_REGISTER) {
						registerClient(msg.name, poolFDs[UNIX_POLLFD].fd, addr, addrlen);
						continue;
					}
					else {
						free(addr);
					}

					if (msg.type == MSG_TYPE_MSG) {
						printf("Broadcasting message from client: %s\n", msg.name);
						broadcast(msg.name, msg);	
					}
	
					else if (msg.type == MSG_TYPE_PROLONG) {
						printf("Prolonging registration period for client: %s\n", msg.name);
						prolongClient(msg.name);
					}
				}
			}
		}
	}

	return 0;
}

