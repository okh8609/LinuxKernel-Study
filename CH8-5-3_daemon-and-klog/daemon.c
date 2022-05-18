#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/klog.h>

#define FALLBACK_KLOG_BUF_SHIFT 17 /* CONFIG_LOG_BUF_SHIFT in kernel */ // 128KB
#define FALLBACK_KLOG_BUF_LEN (1 << FALLBACK_KLOG_BUF_SHIFT)

#define KLOG_CLOSE 0         // Close the log. Currently a NOP.
#define KLOG_OPEN 1          // Open the log. Currently a NOP.
#define KLOG_READ 2          // Read from the log.
#define KLOG_READ_ALL 3      // Read all messages remaining in the ring buffer.
#define KLOG_READ_CLEAR 4    // Read and clear all messages remaining in the ring buffer.
#define KLOG_CLEAR 5         // The call executes just the "clear ring buffer" command.
#define KLOG_CONSOLE_OFF 6   // Disable printk to console.
#define KLOG_CONSOLE_ON 7    // The call sets the console log level to the default, so that messages are printed to the console.
#define KLOG_CONSOLE_LEVEL 8 // The call sets the console log level to the value given in `len`, which must be an integer between 1 and 8.
#define KLOG_SIZE_UNREAD 9   // The call returns the number of bytes currently available to be read.
#define KLOG_SIZE_BUFFER 10  // This command returns the total size of the kernel log buffer.

/* we use 'Linux version' string instead of Oops in this lab */
// #define OOPS_LOG "Linux version"
#define OOPS_LOG "Oops"

int save_kernel_log(char *buffer)
{
	time_t t = time(0);
	struct tm *tm = localtime(&t);

	// 處理檔名
	char path[128];
	snprintf(path, 128, "/tmp/%04d-%02d-%02dT%02d-%02d-%02d.log",
			 tm->tm_year + 1900,
			 tm->tm_mon + 1,
			 tm->tm_mday,
			 tm->tm_hour,
			 tm->tm_min,
			 tm->tm_sec);
	printf("%s\n", path);

	// 存檔
	int fd;
	fd = open(path, O_WRONLY | O_CREAT, 0644);
	if (fd == -1)
	{
		printf("open error\n");
		return -1;
	}
	write(fd, buffer, strlen(buffer));
	close(fd);

	return 0;
}

int check_kernel_log()
{
	printf("start kernel log\n");

	int ret = -1;
	char *buffer;
	ssize_t klog_size;
	int size;

	// 取得buffer大小
	klog_size = klogctl(KLOG_SIZE_BUFFER, 0, 0);
	if (klog_size <= 0)
		klog_size = FALLBACK_KLOG_BUF_LEN;
	printf("kernel log size: %ld\n", klog_size);

	// 建立buffer
	buffer = malloc(klog_size + 1);
	if (!buffer)
		return -1;

	// 讀取資料 (從開機到現在的所有log訊息)
	size = klogctl(KLOG_READ_ALL, buffer, klog_size);
	if (size < 0)
	{
		printf("klogctl read error\n");
		goto done;
	}
	buffer[size] = '\0';
	printf("kernel log read size: %d\n", size);

	// 篩選並儲存資料
	if (strstr(buffer, OOPS_LOG)) // check if oops in klog
	{
		printf("we found '%s' on kernel log\n", OOPS_LOG);
		save_kernel_log(buffer);
		ret = 0;
	}

	// 清理
done:
	free(buffer);
	return ret;
}

int main(void)
{
    if (daemon(0, 0) == -1)
    {
        printf("daemon error");
        return -1;
    }

    while (1)
    {
        check_kernel_log();

        sleep(5);
    }

    return 0;
}

// gcc daemon.c --static
