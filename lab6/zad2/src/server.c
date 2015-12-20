#include "msg.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

mqd_t createServerQueue();
void handleUserAddition();
void handleMessage();
void addUser(char *message);
void removeUser(int i);
void broadcast(char *message);

struct client {
	char name[1024];
	mqd_t queue;
} clients[1024];

int currentUsers = 0;
mqd_t serverQueue;

int main(){
	serverQueue = createServerQueue();

	printf("Server started\n");

	while(1)	{
		handleUserAddition();
		handleMessage();
		sleep(1);
	}

	return 0;
}

mqd_t createServerQueue(){
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_SIZE;

	mqd_t serverQueue = mq_open(SERVER_QNAME, O_RDWR | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
	if(serverQueue == -1)                                           { printf("mq_open() fail"); return -1; }

	return serverQueue;
}

void handleUserAddition(){
	char message[MAX_SIZE];
	if(mq_receive(serverQueue, message, MAX_SIZE, 0) == -1) {
		if(errno != EAGAIN && errno != ENOMSG)                      { printf("mq_receive() fail"); return; }
	}
	else {
		addUser(message);
	}
}

void addUser(char *nick){
	char *name = malloc(strlen(nick) + 2);
	sprintf(name, "/%s", nick);
	
	mqd_t clientQueue = mq_open(name, O_RDWR | O_NONBLOCK);
	if(clientQueue == -1)                                           { printf("mq_open() fail"); return; }

	strcpy(clients[currentUsers].name, nick);
	clients[currentUsers].queue = clientQueue;
	currentUsers++;

	char addMessage[1024];

	time_t t = time(NULL);
	char *tm = ctime(&t);
	tm[strlen(tm) - 1] = '\0';
	sprintf(addMessage, "[%s] %s JOINED", tm, nick);

	printf("%s\n", addMessage);
	broadcast(addMessage);
}

void removeUser(int i){
	clients[i].queue = -1;

	char removeMessage[MAX_SIZE];

	time_t t = time(NULL);
	char *tm = ctime(&t);
	tm[strlen(tm) - 1] = '\0';
	sprintf(removeMessage, "[%s] %s LEFT", tm, clients[i].name);

	printf("%s\n", removeMessage);
	broadcast(removeMessage);
}

void handleMessage(){
	for(int i = 0; i < currentUsers; i++){
		if(clients[i].queue != -1) {
			char message[MAX_SIZE];
			if(mq_receive(clients[i].queue, message, MAX_SIZE, NULL) == -1) {
				if(errno != EAGAIN && errno != ENOMSG)                    { printf("mq_receive() fail"); return; }
				else continue;
			}
			else {	
				if(strcmp(message, "exit") == 0) {
					removeUser(i);
				} else {
					broadcast(message);
				}
			}
		}
	}
}

void broadcast(char *message){
	for(int i = 0; i < currentUsers; i++)	{
		if(clients[i].queue != -1){
			if(mq_send(clients[i].queue, message, MAX_SIZE, 0) == -1)     { printf("mq_send() fail"); return; }
		}
	}
} 
