// Note that this sample program does not make every effort to
// account for edge cases, so be careful when using it in a real project.

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/slab.h>

#define DEMO_NAME "my_demo_dev"
static struct device *my_device;

/*virtual FIFO device's buffer*/
static char *device_buffer;
#define MAX_DEVICE_BUFFER_SIZE 8

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

static ssize_t drv_read(struct file *file, char __user *buf, size_t count, loff_t *offp)
{
    printk("%s enter\n", __func__);

    int for_read = MAX_DEVICE_BUFFER_SIZE - *offp;
    if (for_read == 0)
        dev_warn(my_device, "end of file.\n");
    if (for_read < count)
        count = for_read;

    int ret = copy_to_user(buf, device_buffer + *offp, count);
    if (ret == count)
        return -EFAULT;

    int actual_readed = count - ret;
    *offp += actual_readed;
    printk("%s: actual_readed =%d, pos=%lld\n", __func__, actual_readed, *offp);
    return actual_readed;
}

static ssize_t drv_write(struct file *file, const char __user *buf, size_t count, loff_t *offp)
{
    printk("%s enter\n", __func__);

    int free_space = MAX_DEVICE_BUFFER_SIZE - *offp;
    if (free_space == 0)
        dev_warn(my_device, "not enough space to write");
    if (free_space < count)
        count = free_space;

    int ret = copy_from_user(device_buffer + *offp, buf, count);
    if (ret == count)
        return -EFAULT;

    int actual_write = count - ret;
    *offp += actual_write;
    printk("%s: actual_write =%d, pos=%lld\n", __func__, actual_write, *offp);
    return count;
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
    device_buffer = kmalloc(MAX_DEVICE_BUFFER_SIZE, GFP_KERNEL);
    if (device_buffer == NULL)
        return -ENOMEM; /* Out of memory */

    int ret;

    ret = misc_register(&my_misc_device);
    if (ret)
    {
        printk("failed register misc device\n");
        kfree(device_buffer);
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
    kfree(device_buffer);
    misc_deregister(&my_misc_device);
}

module_init(simple_char_init);
module_exit(simple_char_exit);

MODULE_AUTHOR("Benshushu");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("simpe character device");
