#include <stdio.h>

#define maxint(a, b) ({int A = (a), B = (b); A>B?A:B; })
#define minint(x, y) (     \
    {                      \
        typeof(x) X = (x); \
        typeof(y) Y = (y); \
        (void)(&X == &Y);  \
        X < Y ? X : Y;     \
    })

struct S1 // 12
{
    char aa; // 1
    // int bb[2]; // 4
    int bb[2] __attribute__((packed)); // 4
};

struct S2
{
    short f[3];
} __attribute__((aligned(64)));

int main(void)
{
    printf("%d\n", maxint(-999, 555));
    printf("%f\n", minint(-99.9, 55.5));

    struct S1 s1;
    printf("%ld\n", sizeof(s1));

    struct S2 s2;
    printf("%ld\n", sizeof(s2));
}

