#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>

// sudo cat /proc/pid/smaps

static int pid = 0;
module_param(pid, int, 0644);

static void print_task_struct(struct task_struct *proc)
{
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    mm = proc->mm;
    vma = mm->mmap;
    pr_info("mm_struct addr = 0x%lx\n", (unsigned long)mm);

    /* protect from simultaneous modification */
    /* 遍歷VMA鏈表：
    需要使用down_read()函數來申請一個reader信號量。
    注意，
    只是讀取VMA鏈表，不會修改這個鏈表，	所以申請reader類型的信號量就夠了。
    若需要對VMA鏈表進行修改的話，就需要申請writer類型的信號量。	*/

    // down_read(&mm->mmap_sem); // Linux kernel 5.8 -
    mmap_read_lock(mm);       // Linux kernel 5.8 +

    int n = 0;
    unsigned long start, end, length;

    pr_info("  vmas:              vma            start              end     length\n");
    while (vma)
    {
        start = vma->vm_start;
        end = vma->vm_end;
        length = end - start;
        pr_info("%6d: %16lx %16lx %16lx   %8ld\n",
                n++, (unsigned long)vma, start, end, length);
        vma = vma->vm_next;
    }

    // up_read(&mm->mmap_sem); // Linux kernel 5.8 -
    mmap_read_unlock(mm);   // Linux kernel 5.8 +
}

static int __init my_init(void)
{
    struct task_struct *proc;

    if (pid == 0)
    {
        /* if don't pass the pid over insmod, then use the current process */
        pr_info("using current process\n");
        pid = current->pid; // where current is this tast_struct
    }
    proc = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!proc)
        return -1;
    pr_info("Examining vma's for pid=%d, command=%s\n", pid, proc->comm);
    print_task_struct(proc);
    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Module exit\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL v2");
