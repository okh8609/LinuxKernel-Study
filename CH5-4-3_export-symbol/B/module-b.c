#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

// 導出的符號可以被其他module使用，
// 不過使用之前要先宣告prototype
int add2(int x, int y);

static int __init my_init(void)
{
    printk(KERN_ALERT "my_init\n");
    printk(KERN_ALERT "%d\n", add2(12, 13));
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_ALERT "my_exit\n");
}

module_init(my_init);
module_exit(my_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("khaos");
MODULE_DESCRIPTION("use exported symbol.");
MODULE_ALIAS("kkk");