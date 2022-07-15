#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/interrupt.h>

#define KFIFO_SIZE 128
#define MAX_DEVICES 4
#define DEV_NAME "my_dev"

static dev_t dev;
static struct cdev *cdev;
static struct class *dev_class;

struct my_device
{
    char name[64];
    struct device *dev;
    struct kfifo *my_fifo;
    wait_queue_head_t read_queue;
    wait_queue_head_t write_queue;
    struct mutex lock;
    struct fasync_struct *fasync;
};

struct my_private_data
{
    char name[64];
    struct my_device *device;
    struct tasklet_struct tasklet;
    struct work_struct workqueue;
};

static struct my_device *my_devices[MAX_DEVICES];

static void tasklet_foo(unsigned long data)
{
    struct my_device *device = (struct my_device *)data;

    dev_info(device->dev, "%s: trigger a tasklet\n", __func__);
}

static void work_foo(struct work_struct *work)
{
    struct my_private_data *pdata;
    struct my_device *device;
    pdata = container_of(work, struct my_private_data, workqueue);
    device = pdata->device;
    dev_info(device->dev, "%s: trigger a work\n", __func__);
}

static int mydev_open(struct inode *inode, struct file *file)
{
    unsigned int minor = iminor(inode);
    struct my_device *device = my_devices[minor];

    dev_info(device->dev, "%s: major=%d, minor=%d, device=%s\n",
             __func__, MAJOR(inode->i_rdev), MINOR(inode->i_rdev), device->name);

    // 準備 private data
    struct my_private_data *pdata;
    pdata = kmalloc(sizeof(struct my_private_data), GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;
    sprintf(pdata->name, "private_data_%d", minor);
    tasklet_init(&pdata->tasklet, tasklet_foo, (unsigned long)device); // 初始化tasklet，併指定處理常式。
    INIT_WORK(&pdata->workqueue, work_foo);                            // 初始化workqueue，併指定處理常式。
    pdata->device = device;                                            // 把對應的 device物件 放進去

    file->private_data = pdata; // 記住本次開啟的 private data
    return 0;
}

static ssize_t
mydev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct my_private_data *pdata = file->private_data;
    struct my_device *device = pdata->device;
    int actual_readed;
    int ret;

    if (kfifo_is_empty(device->my_fifo))
    {
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;

        dev_info(device->dev, "%s:%s pid=%d, going to sleep, %s\n", __func__, device->name, current->pid, pdata->name);
        ret = wait_event_interruptible(device->read_queue, !kfifo_is_empty(device->my_fifo));
        if (ret)
            return ret;
    }

    mutex_lock(&device->lock);
    ret = kfifo_to_user(device->my_fifo, buf, count, &actual_readed);
    if (ret)
        return -EIO;
    tasklet_schedule(&pdata->tasklet);
    schedule_work(&pdata->workqueue);
    mutex_unlock(&device->lock);

    if (!kfifo_is_full(device->my_fifo))
    {
        wake_up_interruptible(&device->write_queue);
        kill_fasync(&device->fasync, SIGIO, POLL_OUT);
    }

    dev_info(device->dev, "%s:%s, pid=%d, actual_readed=%d, pos=%lld\n",
             __func__, device->name, current->pid, actual_readed, *ppos);
    return actual_readed;
}

static ssize_t
mydev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct my_private_data *pdata = file->private_data;
    struct my_device *device = pdata->device;
    unsigned int actual_write;
    int ret;

    if (kfifo_is_full(device->my_fifo))
    {
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;

        dev_info(device->dev, "%s:%s pid=%d, going to sleep\n", __func__, device->name, current->pid);
        ret = wait_event_interruptible(device->write_queue, !kfifo_is_full(device->my_fifo));
        if (ret)
            return ret;
    }

    mutex_lock(&device->lock);
    ret = kfifo_from_user(device->my_fifo, buf, count, &actual_write);
    if (ret)
        return -EIO;
    //---tasklet***
    //---workqueue***
    mutex_unlock(&device->lock);

    if (!kfifo_is_empty(device->my_fifo))
    {
        wake_up_interruptible(&device->read_queue);
        kill_fasync(&device->fasync, SIGIO, POLL_IN);
    }

    dev_info(device->dev, "%s:%s pid=%d, actual_write =%d, ppos=%lld, ret=%d\n",
             __func__, device->name, current->pid, actual_write, *ppos, ret);
    return actual_write;
}

static unsigned int mydev_poll(struct file *file, poll_table *wait)
{
    int mask = 0;
    struct my_private_data *pdata = file->private_data;
    struct my_device *device = pdata->device;

    mutex_lock(&device->lock);

    poll_wait(file, &device->read_queue, wait);
    poll_wait(file, &device->write_queue, wait);

    if (!kfifo_is_empty(device->my_fifo))
        mask |= POLLIN | POLLRDNORM;
    if (!kfifo_is_full(device->my_fifo))
        mask |= POLLOUT | POLLWRNORM;

    mutex_unlock(&device->lock);

    return mask;
}

static int mydev_fasync(int fd, struct file *file, int on)
{
    struct my_private_data *pdata = file->private_data;
    struct my_device *device = pdata->device;
    dev_info(device->dev, "%s send SIGIO\n", __func__);

    int ret;

    mutex_lock(&device->lock);
    ret = fasync_helper(fd, file, on, &device->fasync);
    mutex_unlock(&device->lock);

    return ret;
}

static int mydev_release(struct inode *inode, struct file *file)
{
    struct my_private_data *pdata = file->private_data;

    tasklet_kill(&pdata->tasklet);
    kfree(pdata);

    return mydev_fasync(-1, file, 0); // 刪除非同步通知
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mydev_open,
    .release = mydev_release,
    .read = mydev_read,
    .write = mydev_write,
    .poll = mydev_poll,
    .fasync = mydev_fasync,
};

static int __init simple_char_init(void)
{
    int ret;
    int i;

    ret = alloc_chrdev_region(&dev, 0, MAX_DEVICES, DEV_NAME);
    if (ret)
    {
        printk("failed to allocate char device region");
        return ret;
    }

    cdev = cdev_alloc();
    if (!cdev)
    {
        printk("cdev_alloc failed\n");
        ret = -ENOSPC;
        goto unregister_cdev;
    }

    cdev_init(cdev, &fops);
    ret = cdev_add(cdev, dev, MAX_DEVICES);
    if (ret)
    {
        printk("cdev_add failed\n");
        goto cdev_fail;
    }

    dev_class = class_create(THIS_MODULE, "my_class");

    for (i = 0; i < MAX_DEVICES; ++i)
    {
        struct my_device *device;
        device = kzalloc(sizeof(struct my_device), GFP_KERNEL);
        if (!device)
        {
            printk(KERN_ERR "error to alloc device\n");
            ret = -ENOMEM;
            goto free_device;
        }

        sprintf(device->name, "%s%d", DEV_NAME, i);  // set dev name
        mutex_init(&device->lock);                   // init device lock
        device->dev = device_create(dev_class, NULL, // create device
                                    MKDEV(dev, i), NULL, "mydemo:%d:%d", MAJOR(dev), i);
        // print information
        dev_info(device->dev, "create device: %d:%d\n", MAJOR(dev), MINOR(i));

        my_devices[i] = device;

        init_waitqueue_head(&device->read_queue);
        init_waitqueue_head(&device->write_queue);

        device->my_fifo = kmalloc(sizeof(struct kfifo), GFP_KERNEL);
        ret = kfifo_alloc(device->my_fifo, KFIFO_SIZE, GFP_KERNEL);
        if (ret)
        {
            ret = -ENOMEM;
            goto free_device;
        }
    }

    printk("succeeded register char device: %s\n", DEV_NAME);
    return 0;

free_device:
    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (my_devices[i])
        {
            if (my_devices[i]->my_fifo)
                kfifo_free(my_devices[i]->my_fifo);
            kfree(my_devices[i]->my_fifo);
            kfree(my_devices[i]);
        }
    }
cdev_fail:
    cdev_del(cdev);
unregister_cdev:
    unregister_chrdev_region(dev, MAX_DEVICES);
    return ret;
}

static void __exit simple_char_exit(void)
{
    int i;
    printk("removing device\n");

    if (cdev)
        cdev_del(cdev);

    unregister_chrdev_region(dev, MAX_DEVICES);

    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (my_devices[i])
        {
            if (my_devices[i]->my_fifo)
                kfifo_free(my_devices[i]->my_fifo);
            kfree(my_devices[i]->my_fifo);
            kfree(my_devices[i]);
            device_destroy(dev_class, MKDEV(dev, i));
        }
    }
    class_destroy(dev_class);
}

module_init(simple_char_init);
module_exit(simple_char_exit);

MODULE_AUTHOR("Khaos");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("simpe character device");
