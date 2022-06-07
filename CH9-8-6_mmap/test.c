#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>

#define DEV_NAME "/dev/khaos_mmap_dev"

/* defines our IOCTL cmd */
#define MYDEV_CMD__GET_BUFSIZE 0x01

int main()
{
    int fd = open(DEV_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("open device %s failded\n", DEV_NAME);
        return -1;
    }

    size_t len = 0UL;
    if (ioctl(fd, MYDEV_CMD__GET_BUFSIZE, &len) < 0)
    {
        printf("ioctl fail\n");
        goto device_err;
    }
    printf("driver max buffer size=%ld\n", len);

    char *mmap_buffer = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_buffer == (char *)MAP_FAILED)
    {
        printf("mmap driver buffer fail\n");
        goto device_err;
    }
    printf("mmap driver buffer succeeded: %p\n", mmap_buffer);

    srand(time(NULL)); // use current time as seed for random generator
    /* modify the mmaped buffer */
    for (int i = 0; i < len; ++i)
        *(mmap_buffer + i) = (char)('A' + (rand() % 26));

    //
    char *read_buffer = malloc(len);
    if (!read_buffer)
        goto read_error;

    /* read the buffer back and compare with the mmap buffer*/
    if (read(fd, read_buffer, len) != len)
    {
        printf("read fail\n");
        goto read_error;
    }

    if (memcmp(read_buffer, mmap_buffer, len))
    {
        printf("buffer compare fail\n");
        goto comp_error;
    }
    printf("data modify and compare succussful\n");

    free(read_buffer);
    munmap(mmap_buffer, len);
    close(fd);
    return 0;

comp_error:
    free(read_buffer);
read_error:
    munmap(mmap_buffer, len);
device_err:
    close(fd);
    return -1;
}
