#define _GNU_SOURCE
#include <stdbool.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>

#include "defs.h"

int clientFDs[MAX_CLIENTS];

fd_set FDset;
int unixFD, inetFD, maxFD = 0;

bool loop = true;

char socketPath[256];
int portNumber;

void broadcast(int senderFD, struct msg m) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clientFDs[i] != -1 && clientFDs[i] != senderFD) {
			if (write(clientFDs[i], &m, sizeof(struct msg)) == -1)                          { printf("write() fail, %s", strerror(errno)); return; }
		}
	}
}

void addClient(int fd) {
	int i = 0;
	while(clientFDs[i] != -1 && i < MAX_CLIENTS) i++;
	if (i == MAX_CLIENTS)                                                                     { printf("Client overflow\n"); return; }

	struct msg joinMsg;
	strcpy(joinMsg.name, "server");
	sprintf(joinMsg.txt, "Client joined.");
	printf("%s\n", joinMsg.txt);
	broadcast(-1, joinMsg);

	clientFDs[i] = fd;

	FD_SET(fd, &FDset);

	if (fd > maxFD) maxFD = fd;
}

void removeClient(int fd){
	int i = 0;

	while (clientFDs[i] != fd && i < MAX_CLIENTS) i++;
	if (i == MAX_CLIENTS)                                                                     { printf("Client not existing\n"); return; }

	struct msg removeMsg;
	strcpy(removeMsg.name, "server");
	sprintf(removeMsg.txt, "Client left.");
	printf("%s\n", removeMsg.txt);
	broadcast(fd, removeMsg);

	clientFDs[i] = -1;
	FD_CLR(fd, &FDset);
	if (shutdown(fd, SHUT_RDWR) == -1 )                                                      { printf("shutdown() fail, %s", strerror(errno)); return; }
	if (close(fd) == -1)                                                                     { printf("close() fail, %s", strerror(errno)); return ; }
}

void sigintHandler(int signo) {
	printf("SIGINT received, exiting\n");
	loop = false;
}

int main(int argc, char** argv) {
	for (int i = 0; i < MAX_CLIENTS; i++) clientFDs[i] = -1;
	FD_ZERO(&FDset);

	if (argc != 3)                                                                            { printf("Usage: ./server <unix socket path> <port number>\n"); return -1; }

	strcpy(socketPath, argv[1]);
	portNumber = atoi(argv[2]);

	if (signal(SIGINT, sigintHandler) == SIG_ERR)                                             { printf("signal() fail, %s", strerror(errno)); return -1; }


	//AF_UNIX socket
	struct sockaddr_un unix_addr;
	unix_addr.sun_family = AF_UNIX;
	strcpy(unix_addr.sun_path, socketPath);

	if ((unixFD = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)                                     { printf("socket() fail, %s", strerror(errno)); return -1; }

	if (bind(unixFD, &unix_addr, sizeof(unix_addr)) == -1)                                    { printf("bind() fail, %s", strerror(errno)); return -1; }

	if (listen(unixFD, BACKLOG_SIZE) == -1)                                                   { printf("listen() fail, %s", strerror(errno)); return -1; }

	FD_SET(unixFD, &FDset);
	maxFD = unixFD;


	//AF_INET socket
	struct sockaddr_in inet_addr;
	inet_addr.sin_family = AF_INET;
	inet_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_addr.sin_port = htons(portNumber);

	if ((inetFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)                                     { printf("socket() fail, %s", strerror(errno)); return -1; }

	int optval = 1;
	if (setsockopt(inetFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	                                                                                          { printf("setsockopt() fail, %s", strerror(errno)); return -1; }

	if (bind(inetFD, &inet_addr, sizeof(inet_addr)) == -1)                                    { printf("bind() fail, %s", strerror(errno)); return -1; }

	if (listen(inetFD, BACKLOG_SIZE) == -1)                                                   { printf("listen() fail, %s", strerror(errno)); return -1; }

	FD_SET(inetFD, &FDset);
	if (inetFD > maxFD) maxFD = inetFD;


	while (loop) {
		fd_set read_set = FDset;
		if (select(MAX_CLIENTS + NUM_OF_SERVER_SOCKETS + 1, &read_set, NULL, NULL, NULL) == -1) 
			                                                                                  { printf("select() fail, no events\n"); continue; }
		for (int fd = 0; fd <= maxFD; fd++) {
			if (!FD_ISSET(fd, &read_set)) continue;

			if (fd == unixFD || fd == inetFD) {
				int clientFD;
				if ((clientFD = accept(fd, NULL, 0)) == -1)                                   { printf("accept() fail, %s", strerror(errno)); return -1; }
		
				addClient(clientFD);
			}
			else {
				struct msg receivedMsg;
				int receivedSize = 0;
				if ((receivedSize = read(fd, &receivedMsg, sizeof(receivedMsg))) < 0)         { printf("read() fail, %s", strerror(errno)); return -1; }
				else if (receivedSize == 0) {
					removeClient(fd);
				}
				else {
					printf("Broadcasting message from client.\n");
					broadcast(fd, receivedMsg);	
				}
			}
		}	
	}

	if (unlink(socketPath) == -1)                                                             { printf("unlink() fail, %s", strerror(errno)); return -1; }

	return 0;
}