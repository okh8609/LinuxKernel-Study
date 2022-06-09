#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/compat.h>
#include <linux/highmem.h>

#define false 0
#define true 1

#define DEV_NAME "my_pin_dev"

#define DEV_READ 0x00
#define DEV_WRITE 0x01

#define DEV_CMD_GET_BUFSIZE 0x02 /* defines our IOCTL cmd */

/*virtual FIFO device's buffer*/
static char *device_buffer;

#define MAX_BUFFER_SIZE (16 * PAGE_SIZE)

static int dev_open(struct inode *inode, struct file *file)
{
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);

    printk("%s: major=%d, minor=%d\n", __func__, major, minor);

    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    return 0;
}

static size_t dev_rw(void *buf, size_t len, int rw)
{
    // 有幾個pages
    int n_pages = DIV_ROUND_UP(len, PAGE_SIZE); // 向上取整
    printk("%s: len=%ld, npage=%d\n", __func__, len, n_pages);

    // 分配一個雙重指標
    struct page **pages;
    pages = kmalloc(n_pages * sizeof(pages), GFP_KERNEL);
    if (!pages)
    {
        printk("alloc pages fail\n");
        return -ENOMEM;
    }

    struct mm_struct *mm = current->mm;
    // down_read(&mm->mmap_sem); // Linux kernel 5.8 -
    mmap_read_lock(mm); // Linux kernel 5.8 +
    int ret = get_user_pages_fast((unsigned long)buf, n_pages, (int)true, pages);
    if (ret < n_pages)
    {
        printk("pin page fail\n");
        goto fail_pin_pages;
    }
    // up_read(&mm->mmap_sem); // Linux kernel 5.8 -
    mmap_read_unlock(mm); // Linux kernel 5.8 +
    printk("pin %d pages from user done\n", ret);

    char *kmap_addr;
    size_t size = 0;
    size_t count = 0;
    int i;
    for (i = 0; i < n_pages; ++i)
    {
        kmap_addr = kmap(pages[i]); // 把物理页面pages[i]进行一个临时的映射，从而得到一个内核态的虚拟地址kmap_addr。（使用高端記憶體?）
        // print_hex_dump_bytes("kmap:", DUMP_PREFIX_OFFSET, kmap_addr, PAGE_SIZE);
        size = min_t(size_t, PAGE_SIZE, len);
        switch (rw)
        {
        case DEV_READ:
            memcpy(kmap_addr, device_buffer + PAGE_SIZE * i, size);
            // print_hex_dump_bytes("read:", DUMP_PREFIX_OFFSET, kmap_addr, size);
            printk("%s: %s user buffer %ld bytes done\n", __func__, rw ? "write" : "read", count);
            break;
        case DEV_WRITE:
            memcpy(device_buffer + PAGE_SIZE * i, kmap_addr, size);
            // print_hex_dump_bytes("write:", DUMP_PREFIX_OFFSET, device_buffer + PAGE_SIZE*i, size);
            printk("%s: %s user buffer %ld bytes done\n", __func__, rw ? "write" : "read", count);
            break;
        default:
            break;
        }
        len -= size;
        put_page(pages[i]);
        count += size;
        kunmap(pages[i]); // 取消刚才建立的临时映射。（釋放高端記憶體?）
    }

    kfree(pages);
    return count;

fail_pin_pages:
    // up_read(&mm->mmap_sem); // Linux kernel 5.8 -
    mmap_read_unlock(mm); // Linux kernel 5.8 +
    for (i = 0; i < ret; ++i)
        put_page(pages[i]);
    kfree(pages);

    return -EFAULT;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    size_t nbytes = dev_rw(buf, count, DEV_READ);
    printk("%s: read nbytes=%ld done at pos=%d\n", __func__, nbytes, (int)*ppos);
    return nbytes;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    size_t nbytes = dev_rw((void *)buf, count, DEV_WRITE);
    printk("%s: write nbytes=%ld done at pos=%d\n", __func__, nbytes, (int)*ppos);
    return nbytes;
}

static int dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long pfn;
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long len = vma->vm_end - vma->vm_start;

    if (offset >= MAX_BUFFER_SIZE)
        return -EINVAL;
    if (len > (MAX_BUFFER_SIZE - offset))
        return -EINVAL;

    printk("%s: mapping %ld bytes of device buffer at offset %ld\n",
           __func__, len, offset);

    /*    pfn = page_to_pfn (virt_to_page (ramdisk + offset)); */
    pfn = virt_to_phys(device_buffer + offset) >> PAGE_SHIFT;

    if (remap_pfn_range(vma, vma->vm_start, pfn, len, vma->vm_page_prot))
        return -EAGAIN;

    return 0;
}

static long dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    size_t tbs = MAX_BUFFER_SIZE;
    void __user *ioargp = (void __user *)arg;

    switch (cmd)
    {
    default:
        return -EINVAL;
    case DEV_CMD_GET_BUFSIZE:
        if (copy_to_user(ioargp, &tbs, sizeof(tbs)))
            return -EFAULT;
        return 0;
    }
}

#ifdef CONFIG_COMPAT
static long dev_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return dev_unlocked_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#endif

static const struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .mmap = dev_mmap,
    .unlocked_ioctl = dev_unlocked_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = dev_compat_ioctl,
#endif
};

static struct miscdevice misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &dev_fops,
};

static int __init dev_init(void)
{
    device_buffer = kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer)
        return -ENOMEM;

    int ret = misc_register(&misc_device);
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
    misc_deregister(&misc_device);

    printk("removed device\n");
}

module_init(dev_init);
module_exit(dev_exit);

MODULE_AUTHOR("Benshushu");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("simpe character device");