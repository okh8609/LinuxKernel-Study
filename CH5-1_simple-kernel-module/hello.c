#include <linux/init.h>
#include <linux/module.h>

static int __init my_init(void)
{
    printk("hello ~");
    return 0;
}

static void __exit my_exit(void)
{
    printk("bye bye ~");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("khaos");
MODULE_DESCRIPTION("my first kernel module");
MODULE_ALIAS("kkkkk");