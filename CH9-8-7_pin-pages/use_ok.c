#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <malloc.h>

#define DEV_NAME "/dev/my_pin_dev"

#define DEV_CMD_GET_BUFSIZE 0x02 /* defines our IOCTL cmd */

int main(void)
{
    int fd = open(DEV_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("open device %s failded\n", DEV_NAME);
        return -1;
    }

    size_t len = 0;
    if (ioctl(fd, DEV_CMD_GET_BUFSIZE, &len) < 0)
    {
        printf("ioctl fail\n");
        goto open_fail;
    }
    printf("driver max buffer size=%ld\n", len);

    char *read_buffer, *write_buffer;
    read_buffer = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    if (read_buffer == MAP_FAILED)
        goto open_fail;
    write_buffer = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    if (write_buffer == MAP_FAILED)
        goto buffer_fail;

    /* modify the write buffer */
    int i;
    for (i = 0; i < len; i++)
        *(write_buffer + i) = 0x55;

    if (write(fd, write_buffer, len) != len)
    {
        printf("write fail\n");
        goto rw_fail;
    }

    /* read the buffer back and compare with the mmap buffer*/
    if (read(fd, read_buffer, len) != len)
    {
        printf("read fail\n");
        goto rw_fail;
    }

    if (memcmp(read_buffer, write_buffer, len))
    {
        printf("buffer compare fail\n");
        goto rw_fail;
    }

    printf("data modify and compare succussful\n");

    munmap(write_buffer, len);
    munmap(read_buffer, len);
    close(fd);

    return 0;

rw_fail:
    munmap(write_buffer, len);
buffer_fail:
    munmap(read_buffer, len);
open_fail:
    close(fd);
    return -1;
}