#include <linux/init.h>
#include <linux/module.h>

static int debug = 1;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "enable debugging info.\n");

#define dprintk(args...)         \
    if (debug)                   \
    {                            \
        printk(KERN_DEBUG args); \
    }


static int my_param = 999;
module_param(my_param, int, 0644);
MODULE_PARM_DESC(my_param,"my module param.\n");

static int __init my_init(void)
{
    dprintk("hello !!");
    dprintk("my module param = %d", my_param);
    return 0;
}

static void __exit my_exit(void)
{
    dprintk("my module param = %d", my_param);
    printk("bye bye ~");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("khaos");
MODULE_DESCRIPTION("example for kernel module param.");
MODULE_ALIAS("kkk");