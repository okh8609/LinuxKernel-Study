#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>

static int __init my_init(void)
{
	unsigned long order = 10;
	size_t count = 0;

	/* try __get_free_pages__ */
	for (;;)
	{
		if (!alloc_pages(GFP_KERNEL, order))
		{
			pr_err("have alloc %ld pages, but continue alloc failed\n", count);
			break;
		}
		count += (1 << order);
		// msleep_interruptible(10);
	}

	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Module exit\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Ben ShuShu");
MODULE_LICENSE("GPL v2");
