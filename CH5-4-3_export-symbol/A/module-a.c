#include <linux/module.h>

int add2(int x, int y)
{
    return x+y;
}

EXPORT_SYMBOL(add2);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("khaos");
MODULE_DESCRIPTION("example for export symbol.");
MODULE_ALIAS("kkk");