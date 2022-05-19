#include <linux/module.h>
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/cpumask.h>

static DEFINE_PER_CPU(long, cpu_var) = 10;
static long __percpu *cpu_alloc = NULL;

static int __init my_init(void)
{
    int cpu;
    long num;
    long *num_p;
    pr_info("module loaded at 0x%p\n", my_init);

    // Static:
    for_each_possible_cpu(cpu) // 印所有$$
        pr_info("cpu_var cpu%d = %ld\n", cpu, per_cpu(cpu_var, cpu));

    for_each_possible_cpu(cpu)
        per_cpu(cpu_var, cpu) = 100 + cpu; // 訪問特定CPU上的per-CPU變數

    for_each_possible_cpu(cpu) // 印所有$$
        pr_info("cpu_var cpu%d = %ld\n", cpu, per_cpu(cpu_var, cpu));

    num = get_cpu_var(cpu_var); // 停止搶占
    pr_info("cpu_var = %ld\n", num);
    put_cpu_var(cpu_var); // 允許搶占

    __this_cpu_write(cpu_var, 200); // 只會改變其中一顆某當前CPU的值 -> BUG: using __this_cpu_write() in preemptible

    num = get_cpu_var(cpu_var); // 停止搶占
    pr_info("cpu_var = %ld\n", num);
    put_cpu_var(cpu_var); // 允許搶占

    for_each_possible_cpu(cpu) // 印所有$$
        pr_info("cpu_var cpu%d = %ld\n", cpu, per_cpu(cpu_var, cpu));

    // Dynamic:
    cpu_alloc = alloc_percpu(long);

    for_each_possible_cpu(cpu) // 印所有$$
        pr_info("cpu_alloc cpu%d = %ld\n", cpu, *per_cpu_ptr(cpu_alloc, cpu));

    for_each_possible_cpu(cpu)
        (*per_cpu_ptr(cpu_alloc, cpu)) = 300 + cpu; // 訪問特定CPU上的per-CPU變數

    for_each_possible_cpu(cpu) // 印所有$$
        pr_info("cpu_alloc cpu%d = %ld\n", cpu, *per_cpu_ptr(cpu_alloc, cpu));

    num_p = get_cpu_ptr(cpu_alloc); // 停止搶占
    pr_info("cpu_alloc = %ld\n", *num_p);
    put_cpu_ptr(cpu_alloc); // 允許搶占

    for_each_possible_cpu(cpu) // 印所有$$
        pr_info("cpu_alloc cpu%d = %ld\n", cpu, *per_cpu_ptr(cpu_alloc, cpu));

    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Bye: module unloaded from 0x%p\n", my_exit);

    free_percpu(cpu_alloc);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL v2");