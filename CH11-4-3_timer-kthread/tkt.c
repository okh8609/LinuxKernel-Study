// 本實驗請在 arm 的機器上跑！

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/mutex.h>
#include <linux/delay.h>

static struct task_struct *thread;
wait_queue_head_t wait_head;
static atomic_t flags;

static int thread_foo(void *nothing)
{
    set_freezable(); // 清除當前執行緒標誌flags中的PF_NOFREEZE位元，表示當前執行緒能進入掛起或休眠狀態。
    set_user_nice(current, 0);

    while (!kthread_should_stop()) // forever loop
    {
        // try_to_sleeep
        DEFINE_WAIT(wait);

        if (freezing(current) || kthread_should_stop()) // `freezing(current)` -> 檢查系統是否處於freezing狀態
            goto NN;

        prepare_to_wait(&wait_head, &wait, TASK_INTERRUPTIBLE);

        if (!atomic_read(&flags)) // 當 flags==0
            schedule();           // 明確放棄當前作業並讓調度程序接管以選擇活動任務列表中的下一個任務。提高性能。

        finish_wait(&wait_head, &wait);

        // ********** ********** ********** ********** **********
        // ********** ********** ********** ********** **********
        // ********** ********** ********** ********** **********
    NN:
        atomic_set(&flags, 0);
        // ********** ********** ********** ********** **********
        // ********** ********** ********** ********** **********
        // ********** ********** ********** ********** **********

        // show register
        unsigned int spsr, sp;

        asm("mrs %0, spsr_el1"
            : "=r"(spsr)
            :
            : "cc");
        asm("mov %0, sp"
            : "=r"(sp)
            :
            : "cc");

        printk("%s: %s, pid:%d\n", __func__, current->comm, current->pid);
        printk("cpsr:0x%x, sp:0x%x\n", spsr, sp);
    }
    return 0;
}

static void timer_foo(struct timer_list *unused);
// 創建 struct timer_list 型變數，無需再外部定義 timer_list 變數
static DEFINE_TIMER(my_timer, timer_foo);
static void timer_foo(struct timer_list *unused)
{
    atomic_set(&flags, 1);

    printk("%s: set flags %d\n", __func__, atomic_read(&flags));

    wake_up_interruptible(&wait_head);

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000)); // 重設延時時間，啟動計時器。重新註冊定時器到核心，而不管定時器函式是否被執行過。
}

static int __init my_init(void)
{
    init_waitqueue_head(&wait_head);

    /*創建一個執行緒來處理某些事情*/
    thread = kthread_run(thread_foo, NULL, "kthread_foo");

    /*創建一個計時器來類比某些非同步事件，比如中斷等*/
    my_timer.expires = jiffies + msecs_to_jiffies(500);
    // 增加計時器， 計時器運行後，自動釋放。註冊內核計時器，將計時器加入到內核動態計時器鏈表，即啟動計時器。
    add_timer(&my_timer);

    return 0;
}

static void __exit my_exit(void)
{
    kthread_stop(thread);
    del_timer(&my_timer); //註銷計時器

    printk("goodbye\n");
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);