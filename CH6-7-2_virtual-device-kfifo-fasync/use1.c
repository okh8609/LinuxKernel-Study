#define _GNU_SOURCE

#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

static int fd;

void my_func(int signum, siginfo_t *siginfo, void *act)
{
    int ret;
    char buf[64];

    if (signum == SIGIO)
    {
        if (siginfo->si_band & POLLIN)
        {
            // printf("FIFO is not empty\n");
            if ((ret = read(fd, buf, sizeof(buf))) != -1)
            {
                buf[ret] = '\0';
                printf("%s", buf);
            }
        }
        if (siginfo->si_band & POLLOUT)
        {
            // printf("FIFO is not full\n");
        }
    }
}

int main(void)
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGIO);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = my_func;
    if (sigaction(SIGIO, &act, NULL) == -1)
        goto fail;

    fd = open("/dev/mydev0", O_RDWR);
    if (fd < 0)
        goto fail;

    /*設定非同步IO的所有權；告知PID*/
    if (fcntl(fd, F_SETOWN, getpid()) == -1)
        goto fail;

    /*将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID*/
    if (fcntl(fd, F_SETSIG, SIGIO) == -1)
        goto fail;

    /*获取文件flags*/
    int flag = fcntl(fd, F_GETFL);
    if (flag == -1)
        goto fail;
    /*设置文件flags, 设置FASYNC,支持异步通知*/
    if (fcntl(fd, F_SETFL, flag | FASYNC) == -1)
        goto fail;

    while (1)
        sleep(1);

fail:
    perror("fasync error");
    exit(EXIT_FAILURE);
}