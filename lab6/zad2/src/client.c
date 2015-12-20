#include "msg.h"
#include <sys/types.h>
#include <sys/stat.h> 
#include <mqueue.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

mqd_t createClientQueue(char *nick);
mqd_t openServerQueue();

int main(int argc, char **argv){
	if(argc != 2)                                                         { printf("Usage: ./client nick\n"); return -1; }

	char *nick = argv[1];

	mqd_t clientQueue = createClientQueue(nick);
	mqd_t serverQueue = openServerQueue();

	if(mq_send(serverQueue, nick, strlen(nick) + 1, 0) == -1)             { printf("mq_send() fail"); return -1; } 
			                                                                  
	pid_t pid;
	if((pid = fork()) < 0)                                                { printf("fork() fail"); return -1; }
	else if (pid == 0){
		//Receiving process
		while(kill(getppid(), 0) == 0){ //while sending process exists
			unsigned int priority = -1;
			char receivedMessage[MAX_SIZE];
			if(mq_receive(clientQueue, receivedMessage, MAX_SIZE, &priority) == -1) {
				if(errno != EAGAIN && errno != ENOMSG)                    { printf("mq_receive() fail"); return -1; }
				else continue;
			}
			if(priority == 1) {
				if(mq_send(clientQueue, receivedMessage, strlen(receivedMessage) + 1, 1) == -1) 
				                                                          { printf("mq_send() fail"); return -1; }
			}
			else {
				printf("%s\n", receivedMessage);
			}
			usleep(500000);
		}

		if(mq_close(clientQueue) == -1)                                   { printf("mq_close() fail"); return -1; }

		return 0;
	} else {
		//Sending process
		bool exited = false;
		
		do {
			char messageToSend[1024];
			char txt[1024];
			scanf("%s", txt);
			
			if(strcmp(txt, "exit") == 0) {
			 	exited = true;
			} else {
				time_t t = time(NULL);
				char *tm = ctime(&t);
				tm[strlen(tm) - 1] = '\0';
				sprintf(messageToSend, "[%s] %s: %s", tm, nick, txt);
			}	
			if(mq_send(clientQueue, messageToSend, strlen(messageToSend) + 1, 1) == -1) 
				                                                          { printf("mq_send() fail"); return -1; }
		} while(!exited);
				
		return 0;
	}
}

mqd_t createClientQueue(char *nick){
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_SIZE;

	char *name = malloc(strlen(nick) + 2);
	sprintf(name, "/%s", nick);

	mqd_t clientQueue = mq_open(name, O_RDWR | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
	if(clientQueue == -1)                                                 { printf("mq_open() fail"); return -1; }

	return clientQueue;
}

mqd_t openServerQueue(){
	mqd_t serverQueue = mq_open(SERVER_QNAME, O_WRONLY | O_NONBLOCK);
	if(serverQueue == -1)                                                 { printf("mq_open() fail"); return -1; }

	return serverQueue;
}