#include <stdio.h>

extern int max2(int a, int b);

int main()
{
    printf("%d\n%d\n",
           max2(123, 456),
           max2(98, 76));
}