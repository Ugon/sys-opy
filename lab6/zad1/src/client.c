#include "msg.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

int createClientQueue();
int openServerQueue();

int main(int argc, char **argv){
	if(argc != 2)                                                         { printf("Usage: ./client nick\n"); return -1; }

	char *nick = argv[1];

	int clientQueue = createClientQueue();
	int serverQueue = openServerQueue();

	struct msg joinMessage;
	joinMessage.mtype = getpid();
	strcpy(joinMessage.mtext, nick);

	if(msgsnd(serverQueue, &joinMessage, strlen(joinMessage.mtext) + 1, 0) == -1)      
		                                                                  { printf("msgsnd() fail"); return -1; }

	pid_t pid;
	if((pid = fork()) < 0)                                                { printf("fork() fail"); return -1; }
	else if (pid == 0){
		//Receiving process
		struct msg receivedMessage;

		while(kill(getppid(), 0) == 0){ //while sending process exists
			if(msgrcv(clientQueue, &receivedMessage, MAX_SIZE, SERVER_MTYPE, IPC_NOWAIT) == -1){
				if(errno != EAGAIN && errno != ENOMSG)                    { printf("msgrcv() fail\n"); return -1; }
				else continue;
			}

			printf("%s\n", receivedMessage.mtext);
			sleep(1);
		}

		if( msgctl(clientQueue, IPC_RMID, (struct msqid_ds *) NULL) == -1) { printf("msgctl() fail"); return -1; }

		return 0;
	} else {
		//Sending process
		bool exited = false;
		
		do {
			char txt[1024];
			scanf("%s", txt);
			
			struct msg messageToSend;
			messageToSend.mtype = CLIENT_MTYPE;
			
			if(strcmp(txt, "exit") == 0){
			 	exited = true;
				strcpy(messageToSend.mtext, txt);
			} else {
				time_t t = time(NULL);
				char *tm = ctime(&t);
				tm[strlen(tm) - 1] = '\0';
				sprintf(messageToSend.mtext, "[%s] %s: %s", tm, nick, txt);
			}	

			if(msgsnd(clientQueue, &messageToSend, strlen(messageToSend.mtext) + 1, 0) == -1) 
				                                                          { printf("msgsnd() fail"); return -1; }
		} while(!exited);
				
		return 0;
	}
}

int createClientQueue(){
	key_t clientKey = ftok(".", getpid());
	if(clientKey == -1)                                                   { printf("ftok() fail"); return -1; }

	int clientQueue = msgget(clientKey, 0666	 | IPC_CREAT);
	if(clientQueue == -1)                                                   { printf("msgget() fail"); return -1; }

	return clientQueue;
}

int openServerQueue(){
	key_t serverKey = ftok(SERVER_QNAME, 's');
	if(serverKey == -1)                                                   { printf("ftok() fail"); return -1; }

	int serverQueue = msgget(serverKey, 0200);
	if(serverQueue == -1)                                                   { printf("msgget() fail"); return -1; }

	return serverQueue;
}