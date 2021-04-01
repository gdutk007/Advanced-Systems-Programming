#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <linux/uaccess.h>	/* copy_*_user */

#include <linux/semaphore.h>

#define MYDEV_NAME "mycdrv"

#define ramdisk_size (size_t) (16 * PAGE_SIZE) // ramdisk size 

#ifndef DEFAULT_DRIVERS
#define DEFAULT_DRIVERS 3  // default number of drivers 
#endif

struct ASP_mycdrv {
	struct cdev dev;
	char *ramdisk;
	struct semaphore sem;
	int devNo;
};

static struct ASP_mycdrv * my_ASP_mycdrv;

//NUM_DEVICES defaults to 3 unless specified during insmod
int NUM_DEVICES = 3;

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)

static int major = 0, minor = 0;

static int mycdrv_open(struct inode *inode, struct file *file)
{
	pr_info(" OPENING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes = 0;
	// if ((lbuf + *ppos) > ramdisk_size) {
	// 	pr_info("trying to read past end of device,"
	// 		"aborting because this is just a stub!\n");
	// 	return 0;
	// }
	// nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	// *ppos += nbytes;
	// pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	int nbytes = 0;
	// if ((lbuf + *ppos) > ramdisk_size) {
	// 	pr_info("trying to read past end of device,"
	// 		"aborting because this is just a stub!\n");
	// 	return 0;
	// }
	// nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	// *ppos += nbytes;
	// pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}


static const struct file_operations character_driver_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
};

/*
 * Set up the char_dev structure for this device.
 */
static void setup_cdev(struct ASP_mycdrv *dev, int index)
{
	int err, devno = MKDEV(major, minor + index);
    
	cdev_init(&dev->dev, &character_driver_fops);
	dev->dev.owner = THIS_MODULE;
	err = cdev_add (&dev->dev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding character device %d", err, index);
}
static int init_driver(void){

	int result, i;
	dev_t dev = 0;
	result = alloc_chrdev_region(&dev, minor, NUM_DEVICES,"scull");
	major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING 
			"DRIVER: can't allocate a major driver, result: %d\n", result);
		return result;
	}

	// allocating a dynamic number of devices here
	my_ASP_mycdrv = kmalloc(NUM_DEVICES * sizeof(struct ASP_mycdrv), GFP_KERNEL);
	if (!my_ASP_mycdrv) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(my_ASP_mycdrv, 0x00, NUM_DEVICES * sizeof(struct ASP_mycdrv));


	// now we need to initialize them
	for(i = 0; i < NUM_DEVICES; ++i){
		my_ASP_mycdrv[i].devNo = i;
		my_ASP_mycdrv[i].ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
		sema_init(&my_ASP_mycdrv[i].sem,1);
		setup_cdev(&my_ASP_mycdrv[i],i);
	}
    pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
    return 0;
  fail:
	//scull_cleanup_module();
	return result;
}

static void exit_driver(void){

    pr_info("\ndevice unregistered\n");

}


module_init(init_driver);
module_exit(exit_driver);

MODULE_AUTHOR("user");
MODULE_LICENSE("GPL v2");
