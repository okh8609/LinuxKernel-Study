#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/kthread.h>
#include <linux/delay.h>

struct cc
{
    int val;
    struct rcu_head rcu_lock;
};

static struct cc *cc_ptr;

static int frth1(void *data) //读者线程1
{
    struct cc *_ccptr = NULL;
    while (1)
    {
        if (kthread_should_stop())
            break;

        msleep(20);

        rcu_read_lock();

        mdelay(200);
        _ccptr = rcu_dereference(cc_ptr);
        if (_ccptr)
            printk("%s: read a=%d\n", __func__, _ccptr->val);

        rcu_read_unlock();
    }
    return 0;
}

static int frth2(void *data) //读者线程2
{
    struct cc *_ccptr = NULL;
    while (1)
    {
        if (kthread_should_stop())
            break;

        msleep(30);

        rcu_read_lock();

        mdelay(100);
        _ccptr = rcu_dereference(cc_ptr);
        if (_ccptr)
            printk("%s: read a=%d\n", __func__, _ccptr->val);

        rcu_read_unlock();
    }
    return 0;
}

static void cc_rcu_del(struct rcu_head *rhh)
{
    struct cc *obj = container_of(rhh, struct cc, rcu_lock);
    printk("%s: val=%d\n", __func__, obj->val);
    kfree(obj);
}
static int fwthd(void *data) //写者线程
{
    struct cc *old_ptr;
    struct cc *new_ptr;
    int value = *((int *)data);

    while (1)
    {
        if (kthread_should_stop())
            break;

        msleep(250);

        new_ptr = kmalloc(sizeof(struct cc), GFP_KERNEL);

        *new_ptr = *cc_ptr;
        new_ptr->val = value;

        old_ptr = cc_ptr;
        rcu_assign_pointer(cc_ptr, new_ptr);

        call_rcu(&old_ptr->rcu_lock, cc_rcu_del);

        printk("%s: write to new %d\n", __func__, value++);
    }

    return 0;
}

static struct task_struct *rth1;
static struct task_struct *rth2;
static struct task_struct *wthd;

static int __init my_test_init(void)
{
    printk("### my module init\n");
    cc_ptr = kzalloc(sizeof(struct cc), GFP_KERNEL);

    // 傳遞的參數必須在 Heap 區
    int *value = kzalloc(sizeof(int), GFP_KERNEL);
    *value = 168;

    rth1 = kthread_run(frth1, NULL, "rcu_reader_thread_1");
    rth2 = kthread_run(frth2, NULL, "rcu_reader_thread_2");
    wthd = kthread_run(fwthd, (void *)value, "rcu_writer_thread");

    return 0;
}

static void __exit my_test_exit(void)
{
    kthread_stop(rth1);
    kthread_stop(rth2);
    kthread_stop(wthd);
    if (cc_ptr)
        kfree(cc_ptr);
    printk("### goodbye\n");
}

MODULE_LICENSE("GPL");
module_init(my_test_init);
module_exit(my_test_exit);
