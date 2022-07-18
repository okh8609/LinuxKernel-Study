#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>

static int param = 100;
static struct platform_device *my_device;

//

static ssize_t data_show(struct device *d, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", param);
}

static ssize_t data_store(struct device *d, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &param);

	dev_dbg(d, ": write %d into data\n", param);

	return strnlen(buf, count);
}

static DEVICE_ATTR_RW(data); // dev_attr_##_name  ->  "dev_attr_data"

static struct attribute *mydevice_attr_entries[] = {
	&dev_attr_data.attr,
	NULL};

static struct attribute_group mydevice_attr_group = {
	.name = "khaos",
	.attrs = mydevice_attr_entries,
};

//

static int __init my_init(void)
{
	int ret;

	my_device = platform_device_register_simple("khaos", -1, NULL, 0); // add a platform-level device and its resources
	if (IS_ERR(my_device))
	{
		printk("platfrom device register fail\n");
		ret = PTR_ERR(my_device);
		goto dev_reg_fail;
	}

	ret = sysfs_create_group(&my_device->dev.kobj, &mydevice_attr_group);
	if (ret)
	{
		printk("create sysfs group fail\n");
		goto register_fail;
	}

	pr_info("create sysfs node done\n");

	return 0;

register_fail:
	platform_device_unregister(my_device);
dev_reg_fail:
	return ret;
}

static void __exit my_exit(void)
{
	sysfs_remove_group(&my_device->dev.kobj, &mydevice_attr_group);
	platform_device_unregister(my_device);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");