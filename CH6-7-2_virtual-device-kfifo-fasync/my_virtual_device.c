// Note that this sample program does not make every effort to
// account for edge cases, so be careful when using it in a real project.

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>

#define DEMO_NAME "my_demo_dev"
#define MAX_DEVICES 4
static dev_t dev;
static struct cdev *my_cdev;

struct my_device
{
    char name[64];
    struct kfifo *kfifo_buffer;
    wait_queue_head_t read_queue;
    wait_queue_head_t write_queue;
    struct mutex r_lock;
    struct mutex w_lock;
    struct fasync_struct *fasync;
};
static struct my_device *my_devices[MAX_DEVICES];

/*virtual FIFO device's buffer*/
#define MAX_KFIFO_SIZE 16

static int drv_open(struct inode *inode, struct file *file)
{
    unsigned int major = MAJOR(inode->i_rdev);
    unsigned int minor = MINOR(inode->i_rdev);

    struct my_device *_device = my_devices[minor];

    printk("%s: major=%u, minor=%u, device=%s\n",
           __func__, major, minor, _device->name);

    file->private_data = _device;

    return 0;
}

static ssize_t drv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    // printk("%s enter\n", __func__);

    struct my_device *_device = file->private_data;

    int ret;
    unsigned int copied;

    if (kfifo_is_empty(_device->kfifo_buffer))
    {
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;

        printk("%s: pid=%d, going to sleep\n", __func__, current->pid);
        ret = wait_event_interruptible(_device->read_queue, !kfifo_is_empty(_device->kfifo_buffer));
        if (ret != 0)
            return ret;
    }

    if (mutex_lock_interruptible(&_device->r_lock))
        return -ERESTARTSYS;

    ret = kfifo_to_user(_device->kfifo_buffer, buf, count, &copied);

    mutex_unlock(&_device->r_lock);

    if (!kfifo_is_full(_device->kfifo_buffer))
        wake_up_interruptible(&_device->write_queue);

    printk("%s enter. copied=%u\n", __func__, copied);
    return ret ? ret : copied;
}

static ssize_t drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk("%s enter. count=%lu\n", __func__, count);

    struct my_device *_device = file->private_data;

    int ret;
    unsigned int copied;

    if (kfifo_is_full(_device->kfifo_buffer))
    {
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;

        printk("%s: pid=%d, going to sleep\n", __func__, current->pid);
        ret = wait_event_interruptible(_device->write_queue, !kfifo_is_full(_device->kfifo_buffer));
        if (ret != 0)
            return ret;
    }

    if (mutex_lock_interruptible(&_device->w_lock))
        return -ERESTARTSYS;

    ret = kfifo_from_user(_device->kfifo_buffer, buf, count, &copied);

    mutex_unlock(&_device->w_lock);

    if (!kfifo_is_empty(_device->kfifo_buffer))
    {
        wake_up_interruptible(&_device->read_queue);

        kill_fasync(&_device->fasync, SIGIO, POLL_IN);
        printk("%s kill fasync\n", __func__);
    }

    return ret ? ret : copied;
}

static unsigned int drv_poll(struct file *file, poll_table *wait)
{
    struct my_device *_device = file->private_data;

    poll_wait(file, &_device->read_queue, wait);
    poll_wait(file, &_device->write_queue, wait);

    unsigned int mask = 0;
    if (!kfifo_is_empty(_device->kfifo_buffer))
        mask |= POLLIN | POLLRDNORM;
    if (!kfifo_is_full(_device->kfifo_buffer))
        mask |= POLLOUT | POLLWRNORM;

    return mask;
}

static int drv_fasync(int fd, struct file *file, int on)
{
    struct my_device *_device = file->private_data;
    printk("%s send SIGIO\n", __func__);
    return fasync_helper(fd, file, on, &_device->fasync);
}

static int drv_release(struct inode *inode, struct file *file)
{
    printk("%s enter\n", __func__);

    return drv_fasync(-1, file, 0); // 刪除非同步通知
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = drv_open,
    .release = drv_release,
    .read = drv_read,
    .write = drv_write,
    .poll = drv_poll,
    .fasync = drv_fasync,
};

static int __init simple_char_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev, 0, MAX_DEVICES, DEMO_NAME);
    if (ret)
    {
        printk(KERN_ERR "failed to allocate char device region\n");
        return ret;
    }

    my_cdev = cdev_alloc();
    if (!my_cdev)
    {
        printk(KERN_ERR "cdev_alloc failed\n");
        ret = -ENOSPC;
        goto unregister_chrdev;
    }

    cdev_init(my_cdev, &fops);

    ret = cdev_add(my_cdev, dev, MAX_DEVICES);
    if (ret)
    {
        printk("cdev_add failed\n");
        goto cdev_fail;
    }

    printk("succeeded register char device: %s\n", DEMO_NAME);
    printk("Major number = %d, minor number = %d\n", MAJOR(dev), MINOR(dev));

    int i = 0;
    for (i = 0; i < MAX_DEVICES; i++)
    {
        struct my_device *_device = kzalloc(sizeof(struct my_device), GFP_KERNEL);
        // struct my_device *_device = kmalloc(sizeof(struct my_device), GFP_KERNEL);
        if (!_device)
        {
            printk(KERN_ERR "error to alloc _device\n");
            ret = -ENOMEM;
            goto free_device;
        }
        my_devices[i] = _device;

        sprintf(_device->name, "%s%d", DEMO_NAME, i);

        _device->kfifo_buffer = kmalloc(sizeof(struct kfifo), GFP_KERNEL);
        ret = kfifo_alloc(_device->kfifo_buffer, MAX_KFIFO_SIZE, GFP_KERNEL);
        if (ret)
        {
            printk(KERN_ERR "error kfifo_alloc\n");
            ret = -ENOMEM;
            goto free_device;
        }

        init_waitqueue_head(&_device->read_queue);
        init_waitqueue_head(&_device->write_queue);

        mutex_init(&_device->r_lock); // lock for procfs read access
        mutex_init(&_device->w_lock); // lock for procfs write access
    }

    // printk("### Khaos: debug! i=%d, line=%d ###", i, __LINE__);
    printk("succeeded register char devices: %s\n", DEMO_NAME);
    return 0;

free_device:
    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (my_devices[i])
        {
            if (my_devices[i]->kfifo_buffer)
                kfifo_free(my_devices[i]->kfifo_buffer);
            kfree(my_devices[i]->kfifo_buffer);
            kfree(my_devices[i]);
        }
    }
cdev_fail:
    cdev_del(my_cdev);
unregister_chrdev:
    unregister_chrdev_region(dev, MAX_DEVICES);

    return ret;
}

static void __exit simple_char_exit(void)
{
    printk("removing device\n");

    int i = 0;
    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (my_devices[i])
        {
            if (my_devices[i]->kfifo_buffer)
                kfifo_free(my_devices[i]->kfifo_buffer);
            kfree(my_devices[i]->kfifo_buffer);
            kfree(my_devices[i]);
        }
    }

    if (my_cdev)
        cdev_del(my_cdev);

    unregister_chrdev_region(dev, MAX_DEVICES);
}

module_init(simple_char_init);
module_exit(simple_char_exit);

MODULE_AUTHOR("Benshushu");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("simpe character device");
