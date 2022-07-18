#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/debugfs.h>

#define NODE "khaos"

static char my_str[32]; /* should be less sloppy about overflows :) */
static int my_param = 100;

//

static ssize_t my_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
	int nbytes = sprintf(my_str, "%d\n", my_param);
	return simple_read_from_buffer(buf, lbuf, ppos, my_str, nbytes);
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
	ssize_t rc;
	rc = simple_write_to_buffer(my_str, lbuf, ppos, buf, lbuf);
	sscanf(my_str, "%d", &my_param);
	return rc;
}

static const struct file_operations my_debugfs_ops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
};

//

struct dentry *debugfs_dir;

static int __init my_init(void)
{
	debugfs_dir = debugfs_create_dir(NODE, NULL);
	if (IS_ERR(debugfs_dir))
	{
		printk("create debugfs dir fail\n");
		return -EFAULT;
	}

	struct dentry *debugfs_file;
	debugfs_file = debugfs_create_file("my_debugfs_file", 0444, debugfs_dir, NULL, &my_debugfs_ops);
	if (IS_ERR(debugfs_file))
	{
		printk("create debugfs file fail\n");
		debugfs_remove_recursive(debugfs_dir);
		return -EFAULT;
	}

	pr_info("debugfs: %s: Created %s\n", __func__, NODE);

	return 0;
}

static void __exit my_exit(void)
{
	if (debugfs_dir)
	{
		debugfs_remove_recursive(debugfs_dir);
		pr_info("debugfs: %s: Removed %s\n", __func__, NODE);
	}
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
