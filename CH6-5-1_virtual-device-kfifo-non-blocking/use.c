#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEMO_DEV_NAME "/dev/my_demo_dev"

int main()
{
	int ret;

	char message[80] = "Testing the virtual FIFO device";
	size_t message_len = sizeof(message);

	char *read_buffer;
    size_t read_buffer_size = 128;
	read_buffer = malloc(read_buffer_size);
	memset(read_buffer, 0, read_buffer_size);

	int fd;
	fd = open(DEMO_DEV_NAME, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		printf("open device %s failded\n", DEMO_DEV_NAME);
		return -1;
	}

	/*1: read it first*/
    printf("1: read it first\n");
	ret = read(fd, read_buffer, read_buffer_size);
	printf("read %d bytes\n", ret);
	printf("read buffer=%s\n", read_buffer);

	/*2. write the message to device*/
    printf("2. write the message to device\n");
	ret = write(fd, message, message_len);
	if (ret != message_len)
		printf("have write %d bytes\n", ret);

	/*3. write again*/
    printf("3. write again\n");
	ret = write(fd, message, message_len);
	if (ret != message_len)
		printf("have write %d bytes\n", ret);

	/*4. read again*/
    printf("4. read again\n");
	ret = read(fd, read_buffer, read_buffer_size);
	printf("read %d bytes\n", ret);
	printf("read buffer=%s\n", read_buffer);

    /*5. read again*/
    printf("5. read again\n");
	ret = read(fd, read_buffer, read_buffer_size);
	printf("read %d bytes\n", ret);
	printf("read buffer=%s\n", read_buffer);

	close(fd);

	return 0;
}
