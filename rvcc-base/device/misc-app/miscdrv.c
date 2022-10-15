#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/string.h>

#define DEV_NAME  "miscdevtest"

static ssize_t misc_read (struct file *pfile, char __user *buff, size_t size, loff_t *off)
{
	printk(KERN_EMERG "line:%d,%s is call\n", __LINE__, __FUNCTION__);
	return 0;
}
 
static ssize_t misc_write(struct file *pfile, const char __user *buff, size_t size, loff_t *off)
{
	printk(KERN_EMERG "line:%d,%s is call\n", __LINE__, __FUNCTION__);
	return 0;
}
 
static int  misc_open(struct inode *pinode, struct file *pfile)
{
	printk(KERN_EMERG "line:%d,%s is call\n", __LINE__, __FUNCTION__);
	return 0;
}
 
static int misc_release(struct inode *pinode, struct file *pfile)
{
	printk(KERN_EMERG "line:%d,%s is call\n", __LINE__, __FUNCTION__);
	return 0;
}

static const  struct file_operations misc_fops = {
	.read     = misc_read,
	.write    = misc_write,
	.release  = misc_release,
    .open     = misc_open,
};

static struct miscdevice  misc_dev =  {
    .fops  =  &misc_fops,         /* device operation method */
    .minor =  255,                /* second device number */
    .name  =  DEV_NAME,           /* device-name/dev/device-name   */
};

static int __init miscdrv_init(void)
{
    misc_register(&misc_dev);
	printk(KERN_EMERG "INIT misc dev\n");
    return 0;
}

static void  __exit miscdrv_exit(void)
{
    misc_deregister(&misc_dev);
    printk(KERN_EMERG "EXIT,misc\n");
}
 
module_init(miscdrv_init);
module_exit(miscdrv_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LMOS");
