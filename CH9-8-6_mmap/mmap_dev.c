#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/compat.h>

#define DEV_NAME "khaos_mmap_dev"

/*virtual FIFO device's buffer*/
static char *device_buffer;

#define MAX_BUFFER_SIZE (16 * PAGE_SIZE)

static int my_open(struct inode *inode, struct file *file)
{
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);

    printk("%s: major=%d, minor=%d\n", __func__, major, minor);

    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    /**
     * simple_read_from_buffer - copy data from the buffer to user space
     * @to: the user space buffer to read to
     * @count: the maximum number of bytes to read
     * @ppos: the current position in the buffer
     * @from: the buffer to read from
     * @available: the size of the buffer
     *
     * The simple_read_from_buffer() function reads up to @count bytes from the
     * buffer @from at offset @ppos into the user space address starting at @to.
     *
     * On success, the number of bytes read is returned and the offset @ppos is
     * advanced by this number, or negative value is returned on error.
     **/
    int nbytes = simple_read_from_buffer(buf, count, ppos, device_buffer, MAX_BUFFER_SIZE);
    printk("%s: read nbytes=%d done at pos=%d\n", __func__, nbytes, (int)*ppos);
    return nbytes;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    /**
     * simple_write_to_buffer - copy data from user space to the buffer
     * @to: the buffer to write to
     * @available: the size of the buffer
     * @ppos: the current position in the buffer
     * @from: the user space buffer to read from
     * @count: the maximum number of bytes to read
     *
     * The simple_write_to_buffer() function reads up to @count bytes from the user
     * space address starting at @from into the buffer @to at offset @ppos.
     *
     * On success, the number of bytes written is returned and the offset @ppos is
     * advanced by this number, or negative value is returned on error.
     **/
    int nbytes = simple_write_to_buffer(device_buffer, MAX_BUFFER_SIZE, ppos, buf, count);
    printk("%s: write nbytes=%d done at pos=%d\n", __func__, nbytes, (int)*ppos);
    return nbytes;
}

/* defines our IOCTL cmd */
#define MYDEV_CMD__GET_BUFSIZE 0x01

static long my_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    unsigned long tbs = MAX_BUFFER_SIZE;

    switch (cmd)
    {
    case MYDEV_CMD__GET_BUFSIZE:
        if (copy_to_user((void __user *)arg, &tbs, sizeof(tbs)))
            return -EFAULT;
        return 0;
    default:
        return -EINVAL;
    }
}

#ifdef CONFIG_COMPAT
static long my_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return my_unlocked_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#endif

static int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long len = vma->vm_end - vma->vm_start;

    printk("%s: mapping %ld bytes of device buffer at offset %ld\n", __func__, len, offset);

    if (offset >= MAX_BUFFER_SIZE)
        return -EINVAL; /* Invalid argument */
    if (len > (MAX_BUFFER_SIZE - offset))
        return -EINVAL; /* Invalid argument */

    // 查找到该物理内存的起始的页幁号。
    unsigned long pfn = virt_to_phys(device_buffer + offset) >> PAGE_SHIFT;

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot); // prot: 映射的属性

    /**
     * remap_pfn_range() - 把内核态的物理内存映射到用户空间
     * @vma: 描述用户空间的虚拟内存
     * @addr: 要映射的用户空间虚拟内存的起始地址，这个地址必须在vma区域里。
     * @pfn: 物理内存的起始页幁号
     * @size: 映射的大小
     * @prot: 映射的属性
     **/
    if (remap_pfn_range(vma, vma->vm_start, pfn, len, vma->vm_page_prot))
        return -EAGAIN; /* Try again */

    return 0;
}

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .unlocked_ioctl = my_unlocked_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = my_compat_ioctl,
#endif
    .mmap = my_mmap,
};

static struct miscdevice my_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &my_fops,
};

static int __init dev_init(void)
{
    device_buffer = kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer)
        return -ENOMEM;

    int ret = misc_register(&my_misc_device);
    if (ret)
    {
        printk("failed register misc device\n");
        kfree(device_buffer);
        return ret;
    }

    printk("succeeded register char device: %s\n", DEV_NAME);

    return 0;
}

static void __exit dev_exit(void)
{
    printk("removing device\n");

    kfree(device_buffer);
    misc_deregister(&my_misc_device);

    printk("removed device\n");
}

module_init(dev_init);
module_exit(dev_exit);

MODULE_AUTHOR("Benshushu");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("simpe character device");