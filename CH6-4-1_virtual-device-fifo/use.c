#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEV_NAME "/dev/my_demo_dev"

int main(void)
{
	int ret;

	/*1. write the message to device*/
	// char message[] = "Testing the virtual FIFO device";
	char message[] = "1234";
    int fd1;
    fd1 = open(DEV_NAME, O_RDWR);
    if (fd1 < 0)
    {
        printf("open device %s failded\n", DEV_NAME);
        return -1;
    }
	ret = write(fd1, message, sizeof(message));
	if (ret != sizeof(message)) {
		printf("canot write on device %d, ret=%d", fd1, ret);
		return -1;
	}
	close(fd1);

	/*2. read the message from device*/
    char buffer[128];
    int fd2;
    fd2 = open(DEV_NAME, O_RDWR);
    if (fd2 < 0)
    {
        printf("open device %s failded\n", DEV_NAME);
        return -1;
    }
	ret = read(fd2, buffer, 64);
	printf("read %d bytes\n", ret);
	printf("read buffer=%s\n", buffer);
    close(fd2);

    return 0;
}