#include <stdio.h>

int max3(int a, int b, int c)
{
    printf("in clang: %d, %d, %d\n\n", a, b, c);
    if (a >= b && a >= c)
        return a;
    if (b >= a && b >= c)
        return b;
    if (c >= b && c >= a)
        return c;
}