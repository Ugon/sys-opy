#include "msg.h"
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

int createServerQueue();
void handleUserAddition();
void handleMessage();
void addUser(struct msg *message);
void removeUser(int i);
void broadcast(struct msg *message);

struct client {
	char name[1024];
	int queue;
} clients[1024];

int currentUsers = 0;
int serverQueue;

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

int createServerQueue(){
	key_t serverKey = ftok(SERVER_QNAME, 's');
	if(serverKey == -1)                                             { printf("ftok() fail"); return -1; }

	int serverQueue = msgget(serverKey, 0600 | IPC_CREAT);
	if(serverQueue == -1)                                           { printf("msgget() fail"); return -1; }

	return serverQueue;
}

void handleUserAddition(){
	struct msg message;
	if(msgrcv(serverQueue, &message, MAX_SIZE, 0, IPC_NOWAIT) == -1){
		if(errno != EAGAIN && errno != ENOMSG)                      { printf("msgrcv() fail"); return; }
	} else {
		addUser(&message);
	}
}

void addUser(struct msg *message){
	key_t clientKey = ftok(".", message->mtype);
	if(clientKey == -1)                                             { printf("ftok() fail"); return; }

	int clientQueue = msgget(clientKey, 0600);
	if(clientQueue == -1)                                           { perror("msgget() fail"); return; }

	strcpy(clients[currentUsers].name, message->mtext);
	clients[currentUsers].queue = clientQueue;
	currentUsers++;

	struct msg addMessage;
	addMessage.mtype = SERVER_MTYPE;

	time_t t = time(NULL);
	char *tm = ctime(&t);
	tm[strlen(tm) - 1] = '\0';
	sprintf(addMessage.mtext, "[%s] %s JOINED", tm, message->mtext);

	printf("%s\n", addMessage.mtext);
	broadcast(&addMessage);
}

void removeUser(int i){
	clients[i].queue = -1;

	struct msg removeMessage;
	removeMessage.mtype = SERVER_MTYPE;

	time_t t = time(NULL);
	char *tm = ctime(&t);
	tm[strlen(tm) - 1] = '\0';
	sprintf(removeMessage.mtext, "[%s] %s LEFT", tm, clients[i].name);

	printf("%s\n", removeMessage.mtext);
	broadcast(&removeMessage);
}

void handleMessage(){
	for(int i = 0; i < currentUsers; i++){
		if(clients[i].queue != -1){
			struct msg message;

			if(msgrcv(clients[i].queue, &message, MAX_SIZE, CLIENT_MTYPE, IPC_NOWAIT) == -1)	{
				if(errno != EAGAIN && errno != ENOMSG)              { printf("msgrcv() fail"); return; }
				else continue;
			} else {	
				if(strcmp(message.mtext, "exit") == 0)	{
					removeUser(i);
				} else {
					printf("%s\n", message.mtext);
					message.mtype = SERVER_MTYPE;
					broadcast(&message);
				}
			}
		}
	}
}

void broadcast(struct msg *message){
	for(int i = 0; i < currentUsers; i++)	{
		if(clients[i].queue != -1 && msgsnd(clients[i].queue, message, MAX_SIZE, 0) == -1) 
		                                                            { printf("msgsnd() fail"); return; }
	}
} 
