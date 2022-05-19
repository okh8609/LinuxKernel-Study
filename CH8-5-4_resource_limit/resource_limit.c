// getrlimit()
// setrlimit()
// On success, these system calls return 0.
// On error, -1 is returned,
// and errno is set appropriately.

/*
RLIMIT_CPU：CPU時間的最大值（秒單位），超過此限制後會發送SIGXCPU信號給進程。
RLIMIT_FSIZE：創建檔的最大位元組長度。默認為ulimited
RLIMIT_DATA：資料段的最大長度。默認為unlimited
RLIMIT_STACK：stack的長度，一般默認是8K
RLIMIT_CORE：程式crash後生成的core dump檔的大小，如果為0將不生成對應的core檔。
RLIMIT_RSS：最大可駐記憶體位元組長度
RLIMIT_NPROC：每個使用者ID能夠擁有的最大子進程數目，此限制會影響到sysconf的_SC_CHILD_MAX的返回值。
RLIMIT_NOFILE：進程能夠打開的最多檔數目，此限制會影響到sysconf的_SC_OPEN_MAX的返回值。
RLIMIT_MEMLOCK：一個進程使用mlock能夠鎖定存儲空間中的最大位元組長度
RLIMIT_AS/RLIMIT_VMEM: address space限制，記憶體使用者位址空間可用最大長度(虛擬記憶體空間)，會影響到sbrk和mmap函數。
RLIMIT_LOCKS：process可建立的鎖(lock)的最大值。

RLIMIT_NICE:對應進程的優先順序nice值。
RLIMIT_SWAP：進程能夠消耗的最大swap空間。
RLIMIT_MSGQUEUE：為posix訊息佇列可分配的最大存儲位元組數
RLIMIT_SIGPENDING：可排隊的信號最大數量
RLIMIT_NPTS：可同時打開的偽終端數目
RLIMIT_SBSIZE：單個用戶所有通訊端緩衝區的最大長度
RLIMIT_RTPRIO：進程可通過sched_setscheduler和sched_setparam設置的最大即時優先順序。
RLIM_INFINITY：無限制，unlimited。
*/

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

#define DEATH(mess)   \
	{                 \
		perror(mess); \
		exit(errno);  \
	}

#define NC 3 // number of children

void print_all_rlimits(void);
void print_rlimit(int limit, const char *limit_string, struct rlimit *rlim);
void print_rusage(int who, const char *name);

int main(int argc, char *argv[])
{
	/* Print out all limits */
	printf("Printing out all resource limits for pid=%d:\n", getpid());
	print_all_rlimits();

	/* change and printout the limit for core file size */
	struct rlimit rlim;
	printf(" **************************************************** \n\n");
	printf("Before Modification, this is RLIMIT_CORE:\n");
	print_rlimit(RLIMIT_CORE, "RLIMIT_CORE", &rlim);

	rlim.rlim_cur = 8 * 1024 * 1024;
	if (setrlimit(RLIMIT_CORE, &rlim) != 0)
		printf("ERROR: Setting resource limits was unsuccessful.\n");

	printf("After  Modification, this is RLIMIT_CORE:\n");
	print_rlimit(RLIMIT_CORE, "RLIMIT_CORE", &rlim);

	/* fork off the nchildren */
	printf("\n **************************************************** \n\n");
	fflush(stdout);
	pid_t pid = 0;
	int status = 0;
	for (int i = 0; i != NC; ++i)
	{
		pid = fork();
		if (pid < 0)
			DEATH("Failed in fork");
		if (pid == 0)
		{ /* any child */
			printf("In child pid= %d this is RLIMIT_CORE:\n", (int)getpid());
			print_rlimit(RLIMIT_CORE, "RLIMIT_CORE", &rlim);
			fflush(stdout);
			sleep(2);
			exit(EXIT_SUCCESS);
		}
	}
	while (pid > 0)
	{						 /* parent */
		pid = wait(&status); // wait: 運行成功返回子進程識別碼(PID)；發生錯誤返回-1，失敗原因存於errno中。
		printf("Parent got return on pid=%d, status=%d\n", (int)pid, status);
	}

	printf("\n **************************************************** \n");
	print_rusage(RUSAGE_SELF, "RUSAGE_SELF");
	print_rusage(RUSAGE_CHILDREN, "RUSAGE_CHILDREN");

	exit(EXIT_SUCCESS);
}

void print_all_rlimits(void)
{
	struct rlimit rlim;
	print_rlimit(RLIMIT_CPU, "RLIMIT_CPU", &rlim);
	print_rlimit(RLIMIT_FSIZE, "RLMIT_FSIZE", &rlim);
	print_rlimit(RLIMIT_DATA, "RLMIT_DATA", &rlim);
	print_rlimit(RLIMIT_STACK, "RLIMIT_STACK", &rlim);
	print_rlimit(RLIMIT_CORE, "RLIMIT_CORE", &rlim);
	print_rlimit(RLIMIT_RSS, "RLIMIT_RSS", &rlim);
	print_rlimit(RLIMIT_NPROC, "RLIMIT_NPROC", &rlim);
	print_rlimit(RLIMIT_NOFILE, "RLIMIT_NOFILE", &rlim);
	print_rlimit(RLIMIT_MEMLOCK, "RLIMIT_MEMLOCK", &rlim);
	print_rlimit(RLIMIT_AS, "RLIMIT_AS", &rlim);
	print_rlimit(RLIMIT_LOCKS, "RLIMIT_LOCKS", &rlim);
	printf("\n");
}

void print_rlimit(int limit, const char *limit_string, struct rlimit *rlim)
{
	if (getrlimit(limit, rlim) != 0)
		fprintf(stderr, "Failed to get resource limit.\n");
	else
		printf("%16s=%2d: cur=%20lu, max=%20lu\n",
			   limit_string,
			   limit,
			   rlim->rlim_cur,
			   rlim->rlim_max);
}

void print_rusage(int who, const char *name)
{
	printf("\n[For %s:]\n", name);

	struct rusage usage;
	if (getrusage(who, &usage))
		DEATH("getrusage failed");

	printf("ru_utime.tv_sec = %5d, ru_utime.tv_usec = %5d (user time used)\n", (int)usage.ru_utime.tv_sec, (int)usage.ru_utime.tv_usec);
	printf("ru_stime.tv_sec = %5d, ru_stime.tv_usec = %5d (system time used)\n", (int)usage.ru_stime.tv_sec, (int)usage.ru_stime.tv_usec);
	printf("ru_maxrss =   %5ld (max resident set size)\n", usage.ru_maxrss);
	printf("ru_ixrss =    %5ld (integral shared memory size)\n", usage.ru_ixrss);
	printf("ru_idrss =    %5ld (integral unshared data size)\n", usage.ru_idrss);
	printf("ru_isrss =    %5ld (integral unshared stack size)\n", usage.ru_isrss);
	printf("ru_minflt =   %5ld (page reclaims)\n", usage.ru_minflt);
	printf("ru_majflt =   %5ld (page faults)\n", usage.ru_majflt);
	printf("ru_nswap =    %5ld (swaps)\n", usage.ru_nswap);
	printf("ru_inblock =  %5ld (block input operations)\n", usage.ru_inblock);
	printf("ru_oublock =  %5ld (block output operations)\n", usage.ru_oublock);
	printf("ru_msgsnd =   %5ld (messages sent)\n", usage.ru_msgsnd);
	printf("ru_msgrcv =   %5ld (messages received)\n", usage.ru_msgrcv);
	printf("ru_nsignals = %5ld (signals received)\n", usage.ru_nsignals);
	printf("ru_nvcsw =    %5ld (voluntary context switches)\n", usage.ru_nvcsw);
	printf("ru_nivcsw =   %5ld (involuntary context switches)\n", usage.ru_nivcsw);
}