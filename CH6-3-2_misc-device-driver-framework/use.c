#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEV_NAME "/dev/my_demo_dev"

int main(void)
{
    char buffer[128];

    int fd;
    fd = open(DEV_NAME, O_RDONLY);

    if (fd < 0)
    {
        printf("open device %s failded\n", DEV_NAME);
        return -1;
    }

    read(fd, buffer, 128);
    close(fd);

    return 0;
}