#include <linux/module.h>

static int __init crash_module_init(void)
{
    int *p = 0;
    printk("%d\n", *p);
    return 0;
}

static void __exit crash_module_exit(void)
{
}

MODULE_LICENSE("GPL v2");
module_init(crash_module_init);
module_exit(crash_module_exit);