#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/mm.h>

#define NULL ((void *)0)

#define MB (1024 * 1024)
#define GB (1024 * 1024 * 1024)

static int __init my_init(void)
{
    unsigned long MEM = get_num_physpages() * PAGE_SIZE;

    unsigned int order;
    unsigned long size;

    char *buff;

    // __get_free_pages__
    for (size = PAGE_SIZE, order = 0; order < MAX_ORDER; ++order, size *= 2)
    {
        pr_info(" order=%2u, pages=%5lu, size=%8lu ",
                order, size / PAGE_SIZE, size);
        buff = (char *)__get_free_pages(GFP_ATOMIC, order);
        if (!buff)
        {
            pr_err("... __get_free_pages failed\n");
            break;
        }
        pr_info("... __get_free_pages OK\n");
        free_pages((unsigned long)buff, order);
    }

    // kmalloc
    for (size = PAGE_SIZE, order = 0; order < MAX_ORDER; ++order, size *= 2)
    {
        pr_info(" order=%2u, pages=%5lu, size=%8lu ",
                order, size / PAGE_SIZE, size);
        buff = kmalloc((size_t)size, GFP_ATOMIC);
        if (!buff)
        {
            pr_err("... kmalloc failed\n");
            break;
        }
        pr_info("... kmalloc OK\n");
        kfree(buff);
    }

    // vmalloc
    // for (size = 4 * MB; size <= MEM; size += 4 * MB)
    for (size = 4096UL * MB; size <= MEM; size += 4096UL * MB) // 要了60GB也沒問題!!
    {
        pr_info(" pages=%6lu, size=%8luMB (%8luGB ) ", size / PAGE_SIZE, size / MB, size / GB);
        buff = vmalloc(size);
        if (!buff)
        {
            pr_err("... vmalloc failed\n");
            break;
        }
        pr_info("... vmalloc OK\n");
        vfree(buff);
    }

    pr_info("Module init OK!\n");
    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Module exit\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Khaos");
MODULE_LICENSE("GPL v2");
