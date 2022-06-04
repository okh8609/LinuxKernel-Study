#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>

#ifndef ARCH_PFN_OFFSET
#define ARCH_PFN_OFFSET 0 // x86 ????????????
#endif

#define NULL ((void *)0)

#define PRINT_INFO(key, value)         \
    pr_info("%-15s=%10d %10ld %8ld\n", \
            key, value, (PAGE_SIZE * value) / 1024, (PAGE_SIZE * value) / 1024 / 1024)

static int __init my_init(void)
{
    unsigned long num_physpages;
    //獲取系統所有記憶體的大小，返回實體記憶體的頁面數量。
    num_physpages = get_num_physpages();
    pr_info("Examining %ld pages (num_phys_pages) = %ld MB. (page size = %ld)\n",
            num_physpages, num_physpages * PAGE_SIZE / 1024 / 1024, PAGE_SIZE);
    num_physpages += 819200; // x86 ??????????????

    struct page *pg = NULL;

    unsigned long i = 0, pfn = 0, valid = 0;
    int free = 0, locked = 0, reserved = 0, swapcache = 0, referenced = 0, slab = 0, priv = 0, uptodate = 0, dirty = 0, active = 0, writeback = 0, mappedtodisk = 0;
    pr_info("ARCH_PFN_OFFSET = %d", ARCH_PFN_OFFSET);
    for (i = 0; i < num_physpages; i++)
    {
        /* Most of ARM systems have ARCH_PFN_OFFSET */
        pfn = i + ARCH_PFN_OFFSET;
        /* may be holes due to remapping */
        if (!pfn_valid(pfn)) // 檢查頁幁號pfn是否有效。
            continue;
        ++valid;

        pg = pfn_to_page(pfn); // 表示從頁幁號pfn轉換到struct page資料結構p。
        if (pg == NULL)
            continue;

        /* page_count(page) == 0 is a free page. */
        if (page_count(pg) == 0) // 引用計數，表示內核中引用該page的次數。等於0的話，說明這個頁面是空閒頁面。
        {
            ++free;
            continue;
        }

        if (PageLocked(pg))
            locked++;
        if (PageReserved(pg))
            reserved++;
        if (PageSwapCache(pg))
            swapcache++;
        if (PageReferenced(pg))
            referenced++;
        if (PageSlab(pg))
            slab++;
        if (PagePrivate(pg))
            priv++;
        if (PageUptodate(pg))
            uptodate++;
        if (PageDirty(pg))
            dirty++;
        if (PageActive(pg))
            active++;
        if (PageWriteback(pg))
            writeback++;
        if (PageMappedToDisk(pg))
            mappedtodisk++;
    }

    pr_info("Pages with valid PFN's=%ld, = %ld MB\n",
            valid, valid * PAGE_SIZE / 1024 / 1024);

    pr_info("\n                     Pages         KB       MB\n");
    PRINT_INFO("free", free);
    PRINT_INFO("locked", locked);
    PRINT_INFO("reserved", reserved);
    PRINT_INFO("swapcache", swapcache);
    PRINT_INFO("referenced", referenced);
    PRINT_INFO("slab", slab);
    PRINT_INFO("private", priv);
    PRINT_INFO("uptodate", uptodate);
    PRINT_INFO("dirty", dirty);
    PRINT_INFO("active", active);
    PRINT_INFO("writeback", writeback);
    PRINT_INFO("mappedtodisk", mappedtodisk);

    return 0;
}

static void __exit my_exit(void)
{
    pr_info("mem_info Module exit\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Khaos");
MODULE_LICENSE("GPL v2");