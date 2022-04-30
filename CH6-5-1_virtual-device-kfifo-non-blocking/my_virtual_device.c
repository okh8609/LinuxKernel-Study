// Note that this sample program does not make every effort to
// account for edge cases, so be careful when using it in a real project.

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>

#define DEMO_NAME "my_demo_dev"
static struct device *my_device;

/*virtual FIFO device's buffer*/
#define MAX_KFIFO_SIZE 16
static struct kfifo test;

/* lock for procfs read access */
static DEFINE_MUTEX(read_lock);

/* lock for procfs write access */
static DEFINE_MUTEX(write_lock);

static int drv_open(struct inode *inode, struct file *file)
{
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);

    printk("%s: major=%d, minor=%d\n", __func__, major, minor);
    return 0;
}

static int drv_release(struct inode *inode, struct file *file)
{
    printk("%s enter\n", __func__);
    return 0;
}

static ssize_t drv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk("%s enter\n", __func__);

    int ret;
    unsigned int copied;

    if (kfifo_is_empty(&test))
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;

    if (mutex_lock_interruptible(&read_lock))
        return -ERESTARTSYS;

    ret = kfifo_to_user(&test, buf, count, &copied);

    mutex_unlock(&read_lock);

    return ret ? ret : copied;
}

static ssize_t drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk("%s enter. count=%lu\n", __func__, count);

    int ret;
    unsigned int copied;

    // int kfifo_free_space = kfifo_size(&test) - kfifo_len(&test);
    // if (kfifo_free_space < count)
    // {
    //     if (file->f_flags & O_NONBLOCK)
    //     {
    //         return -EAGAIN;
    //     }
    //     else
    //     {
    //         return -ENOMEM; /* Out of memory */
    //     }
    // }
    if (kfifo_is_full(&test))
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;

    if (mutex_lock_interruptible(&write_lock))
        return -ERESTARTSYS;

    ret = kfifo_from_user(&test, buf, count, &copied);

    mutex_unlock(&write_lock);

    return ret ? ret : copied;
}

static const struct file_operations demodrv_fops = {
    .owner = THIS_MODULE,
    .open = drv_open,
    .release = drv_release,
    .read = drv_read,
    .write = drv_write};

static struct miscdevice my_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEMO_NAME,
    .fops = &demodrv_fops,
};

static int __init simple_char_init(void)
{
    int ret;

    ret = kfifo_alloc(&test, MAX_KFIFO_SIZE, GFP_KERNEL);
    if (ret)
    {
        printk(KERN_ERR "error kfifo_alloc\n");
        return ret;
    }

    ret = misc_register(&my_misc_device);
    if (ret)
    {
        printk(KERN_ERR "failed register misc device\n");
        kfifo_free(&test);
        return ret;
    }

    my_device = my_misc_device.this_device;

    printk("succeeded register char device: %s\n", DEMO_NAME);
    printk("Major number = %d, minor number = %d\n", MAJOR(my_device->devt), MINOR(my_device->devt));
    return 0;
}

static void __exit simple_char_exit(void)
{
    printk("removing device\n");
    misc_deregister(&my_misc_device);
    kfifo_free(&test);
}

module_init(simple_char_init);
module_exit(simple_char_exit);

MODULE_AUTHOR("Benshushu");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("simpe character device");
