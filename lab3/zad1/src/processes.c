#define _GNU_SOURCE //error: ‘CLONE_VFORK’ undeclared

#include <stdlib.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

#define CHILD_STACK_SIZE 1024

#define lambda(l_ret_type, l_arguments, l_body)         \
  ({                                                    \
    l_ret_type l_anonymous_functions_name l_arguments   \
      l_body                                            \
    &l_anonymous_functions_name;                        \
  })


long counter = 0;

//sciagniete ze Stevensa
void printTimes(clock_t startReal, clock_t stopReal, struct tms *startTms, struct tms *stopTms){
	static long clktck = 0;
	if(clktck == 0 && (clktck = sysconf(_SC_CLK_TCK)) < 0) printf("sysconf error");

	printf("real:       %7.2f\n", (stopReal            - startReal           ) / (double) clktck);
	printf("user:       %7.2f\n", (stopTms->tms_utime  - startTms->tms_utime ) / (double) clktck);
	printf("sys:        %7.2f\n", (stopTms->tms_stime  - startTms->tms_stime ) / (double) clktck);
	printf("child user: %7.2f\n", (stopTms->tms_cutime - startTms->tms_cutime) / (double) clktck);
	printf("child sys:  %7.2f\n", (stopTms->tms_cstime - startTms->tms_cstime) / (double) clktck);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: ./processes N\n");
		return -1;
	}

	int N = atoi(argv[1]);
	struct tms startTms, stopTms;
	clock_t startReal, stopReal;

	startReal = times(&startTms);

	for(int i = 0; i < N; i ++) {
		pid_t pid;

#ifdef FORK		
		if ((pid = fork()) < 0) {
			printf("fork error\n");
		}
		else if (pid == 0) {
			counter++;
			_exit(0);
		}
		else {
			wait(NULL);
		}
#endif

#ifdef VFORK
		if ((pid = vfork()) < 0) {
			printf("fork error\n");
		}
		else if (pid == 0) {
			counter++;
			_exit(0);
		}
		else {
			wait(NULL);
		}	
#endif

#ifdef CLONE
		char *stack = malloc(CHILD_STACK_SIZE);

		pid = clone(
		lambda (int, (), {
				counter++;
				_exit(0);
			}),
			stack + CHILD_STACK_SIZE,
			SIGCHLD,
			NULL);
		
		if (pid == -1) {
			printf("clone error\n");
		}

		wait(NULL);

		free(stack);


#endif

#ifdef VCLONE
		char *stack = malloc(CHILD_STACK_SIZE);

		pid = clone(
			lambda (int, (), {
				counter++;
				_exit(0);
			}),
			stack + CHILD_STACK_SIZE,
			CLONE_VFORK | CLONE_VM | SIGCHLD,
			NULL);

		if (pid == -1) {
			printf("clone error\n");
		}

		wait(NULL);

		free(stack);
#endif
	}

	stopReal = times(&stopTms);

	printTimes(startReal, stopReal, &startTms, &stopTms);
	printf("\nCounter result: %ld\n", counter);
	return 0;
}


