#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SIZE (256 * 1024 * 1024)

static int alloc_mem(long int size)
{
    char *s;
    long i, pagesz = getpagesize();

    printf("thread(%lx), allocating %ld MB.\n", (unsigned long)pthread_self(), size / 1024 / 1024);

    s = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (s == MAP_FAILED)
        return errno;

    /* touch the memory */
    for (i = 0; i < size; i += pagesz)
        s[i] = '\a';

    return 0;
}

static void *child_thread(void *args)
{
    int ret = 0;

    /* keep allocating until there is an error */
    while (!ret)
        ret = alloc_mem(SIZE);

    exit(ret);
}

static void child_alloc(int n_cpus)
{
    pthread_t *th;
    th = malloc(sizeof(pthread_t) * n_cpus);
    if (!th)
    {
        printf("malloc failed\n");
        exit(1);
    }

    /* create threads */
    int i, ret;
    for (i = 0; i != n_cpus; ++i)
    {
        ret = pthread_create(th + i, NULL, child_thread, NULL);
        if (ret)
        {
            printf("pthread_create error: %s\n", strerror(errno));
            if (ret != EAGAIN)
                exit(1);
        }
    }

    /* wait for one of threads to exit whole process */
    while (1)
        sleep(1);
}

int main(void)
{
    long n_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("number of cpus is %ld.\n", n_cpus);

    pid_t pid;
    switch (pid = fork())
    {
    case 0:
        child_alloc(n_cpus > 1 ? n_cpus : 1);
    default:
        break;
    }

    printf("expected victim is %d.\n", pid);

    int status = 0;
    waitpid(-1, &status, 0);

    if (WIFSIGNALED(status)) // 为非0 (true) 表明进程`异常`终止。
    {
        printf("victim signalled: %d\n", WTERMSIG(status)); // 9: SIGKILL
    }
    else if (WIFEXITED(status)) // 为非0 (true) 表明进程`正常`结束。
    {
        int retcode = WEXITSTATUS(status);
        printf("victim retcode: (%d) %s\n", retcode, strerror(retcode));
    }
    else
    {
        printf("victim unexpected ended\n");
    }

    return 0;
}