#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

static int fd;

void my_func(int signum)
{
    int ret;
    char buf[64];

    if (signum == SIGIO)
    {
        if ((ret = read(fd, buf, sizeof(buf))) != -1)
        {
            buf[ret] = '\0';
            printf("%s", buf);
        }
    }
}

int main(void)
{
    // 先把buffer清空
    fd = open("/dev/mydev1", O_RDWR | O_NONBLOCK);
    if (fd < 0)
        goto fail;
    char buf[64];
    int ret;
    do
    {
        sleep(1);
        ret = read(fd, buf, sizeof(buf));
        printf("%d ", ret);
    } while (ret != -1);
    printf("\nbuffer is emptied.\n");
    close(fd);

    /*--------------------------------------------------*/

    signal(SIGIO, my_func);

    fd = open("/dev/mydev1", O_RDWR);
    if (fd < 0)
        goto fail;

    /*設定非同步IO的所有權；告知PID*/
    if (fcntl(fd, F_SETOWN, getpid()) == -1)
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