#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>

static int obj_size = 19;
static struct kmem_cache *m_slab;
static char *m_buf;

static int __init my_init(void)
{
    /* create a memory cache */
    if (obj_size > KMALLOC_MAX_SIZE)
    {
        pr_err(" size=%d is too large; you can't have more than %lu!\n", obj_size, KMALLOC_MAX_SIZE);
        return -1;
    }

    /* create a slab identifier */
    m_slab = kmem_cache_create("my_cache", obj_size, 0, SLAB_HWCACHE_ALIGN, NULL);
    if (!m_slab)
    {
        pr_err("kmem_cache_create failed\n");
        return -ENOMEM;
    }
    pr_info("create mycache correctly\n");

    /* allocate a memory cache object */
    m_buf = kmem_cache_alloc(m_slab, GFP_ATOMIC);
    if (!m_buf)
    {
        pr_err(" failed to create a cache object\n");
        (void)kmem_cache_destroy(m_slab);
        return -1;
    }
    pr_info("successfully created a object, kbuf_addr=0x%lx\n", (unsigned long)m_buf);

    return 0;
}

static void __exit my_exit(void)
{
    /* destroy the memory cache object */
    kmem_cache_free(m_slab, m_buf);
    pr_info("destroyed a cache object\n");

    /* destroy the slab identifier */
    kmem_cache_destroy(m_slab);
    pr_info("destroyed mycache\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Khaos OU");