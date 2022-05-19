#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    pid_t mypid = getpid();
    printf("\nExamining priorities forPID = %d \n", mypid);

    printf("%10s%10s%10s\n", "Previous", "Requested", "Assigned");
    int i, old_prio, new_prio;
    for (i = -20; i < 20; i += 2)
    {
        old_prio = getpriority(PRIO_PROCESS, (int)mypid);
        
        if (setpriority(PRIO_PROCESS, (int)mypid, i))
            fprintf(stderr, "setpriority() failed ");

        /* must clear errno before call to getpriority
           because -1 is a valid return value */
        errno = 0;
        new_prio = getpriority(PRIO_PROCESS, (int)mypid);

        printf("%10d%10d%10d\n", old_prio, i, new_prio);
    }
    exit(EXIT_SUCCESS);
}