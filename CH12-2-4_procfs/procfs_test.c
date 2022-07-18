#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>

static struct proc_dir_entry *my_root;
static struct proc_dir_entry *my_proc;

static int my_param = 100;
static char my_str[32]; // should be less sloppy about overflows :)

static ssize_t my_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
    int nbytes = sprintf(my_str, "%d\n", my_param);
    printk("%s: my_param = %d\n", __func__, my_param);
    return simple_read_from_buffer(buf, lbuf, ppos, my_str, nbytes);
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos)
{
    ssize_t rc = simple_write_to_buffer(my_str, lbuf, ppos, buf, lbuf);
    sscanf(my_str, "%d", &my_param);
    printk("%s: my_param = %d\n", __func__, my_param);
    return rc;
}

// static const struct file_operations my_proc_fops = {
//     .owner = THIS_MODULE,
//     .read = my_read,
//     .write = my_write,
// };

static const struct proc_ops my_proc_fops = {
    .proc_read = my_read,
    .proc_write = my_write,
};

static int __init my_init(void)
{
    my_root = proc_mkdir("khaos", NULL);
    if (IS_ERR(my_root))
    {
        pr_err("proc_mkdir failed.\n");
        return -1;
    }

    my_proc = proc_create("khaos/my_proc", 0, NULL, &my_proc_fops);
    if (IS_ERR(my_proc))
    {
        pr_err("proc_create failed.\n");
        return -1;
    }

    pr_info("proc node created.\n");
    return 0;
}

static void __exit my_exit(void)
{
    if (my_proc != NULL)
    {
        proc_remove(my_proc);
        proc_remove(my_root);
        pr_info("proc node removed.\n");
    }
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");