#define _GNU_SOURCE
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>

int param = 0;

int thread_fn(void *data)
{
	int j;
	printf("starting child thread_fn, pid=%d\n", getpid());
	for (j = 0; j < 10; j++) {
		param = j + 1000;
		sleep(1);
		printf("child thread running: j=%d, param=%d secs\n", j, param);
	}
	printf("child thread_fn exit\n");
	return 0;
}

int main(int argc, char **argv)
{
	printf("starting parent process, pid=%d\n", getpid());

	int j, tid;

    int pagesize, stacksize;
	pagesize = getpagesize(); // 系統的分頁大小，不一定會與硬體分頁大小相同。 Intel x86 上其返回值為 4096bytes
	stacksize = 4 * pagesize;
    printf("stacksize = %d\n", stacksize); 
    // ↑ 16KB，處理程序核心堆疊大小：Linux 4.0 為 8KB，Linux 5.4 為 16KB

	/* could probably just use malloc(), but this is safer */
	/* stack = (char *)memalign (pagesize, stacksize); */
	void *stack;
	posix_memalign(&stack, pagesize, stacksize);

	printf("Setting a clone child thread with stacksize = %d....", stacksize);
	tid = clone(thread_fn, (char *)stack + stacksize, CLONE_VM | SIGCHLD, 0);
	printf(" with tid=%d\n", tid);
	if (tid < 0)
		exit(EXIT_FAILURE);

	/* could do a  wait (&status) here if required */
	for (j = 0; j < 6; j++) {
		param = j;
		sleep(1);
		printf("parent running: j=%d, param=%d secs\n", j, param);
	}
	printf("parent killitself\n");
	/* We shouldn't free(stack) here since the child using it is still running */
	exit(EXIT_SUCCESS);
}

