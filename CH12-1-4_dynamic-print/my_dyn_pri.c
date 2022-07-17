#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/mutex.h>
#include <linux/delay.h>

static int __init my_init(void)
{
    pr_debug("test for pr_debug() %d\n", 888);
    /*
        /home/khaos/LinuxKernel-Study/CH12-1-4_dynamic-print/my_dyn_pri.c:11
        [my_dyn_pri]my_init =_ "test for pr_debug() %d\012"
    */
    return 0;
}

static void __exit my_exit(void)
{
    printk("goodbye\n");
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);