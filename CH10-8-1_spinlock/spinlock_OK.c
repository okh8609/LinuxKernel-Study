#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/delay.h>

static DEFINE_SPINLOCK(spl);
static struct page *page;
int order = 5;

static int __init my_init(void)
{
    page = alloc_pages(GFP_KERNEL, order);
    /**
     * kmalloc() 在系統空閒記憶體不足時會等待睡眠
     * 若放在 Critical Section 內，易導致效能低下
     * 可顯式使用 GFP_ATOMIC 分配隱藏來解決。
     */

    spin_lock(&spl);

    if (!page)
    {
        printk("cannot alloc pages\n");
        return -ENOMEM;
    }
    
    /* we sleep here to simulate that allocate memory under pressure */
    // msleep(10000);

    spin_unlock(&spl);

    return 0;
}

static void __exit my_exit(void)
{
    __free_pages(page, order);
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
