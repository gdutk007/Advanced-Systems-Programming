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
	struct cdev device;
	char *ramdisk;
	struct semaphore sem;
	int devNo;
	int size;
};

static struct ASP_mycdrv * my_ASP_mycdrv;

//NUM_DEVICES defaults to 3 unless specified during insmod
int NUM_DEVICES = DEFAULT_DRIVERS;
static int major = 0, minor = 0;
// params
module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);
module_param(NUM_DEVICES, int, S_IRUGO);


#define CDRV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)

/*
 * Empty out the scull device; must be called with the device
 * semaphore held.
 */
// what does this really do? 
// int scull_trim(struct scull_dev *dev)
// {
// 	// struct scull_qset *next, *dptr;
// 	// int qset = dev->qset;   /* "dev" is not-null */
// 	// int i;

// 	// for (dptr = dev->data; dptr; dptr = next) { /* all the list items */
// 	// 	if (dptr->data) {
// 	// 		for (i = 0; i < qset; i++)
// 	// 			kfree(dptr->data[i]);
// 	// 		kfree(dptr->data);
// 	// 		dptr->data = NULL;
// 	// 	}
// 	// 	next = dptr->next;
// 	// 	kfree(dptr);
// 	// }
// 	// dev->size = 0;
// 	// dev->quantum = scull_quantum;
// 	// dev->qset = scull_qset;
// 	// dev->data = NULL;
// 	return 0;
// }

static int mycdrv_open(struct inode *inode, struct file *filp)
{
	struct ASP_mycdrv * dev;

	dev = container_of(inode->i_cdev, struct ASP_mycdrv,device);

	// /* now trim to 0 the length of the device if open was write-only */
	// if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
	// 	if (down_interruptible(&dev->sem))
	// 		return -ERESTARTSYS;
	// 	scull_trim(dev); /* ignore errors */
	// 	up(&dev->sem);
	// }
	pr_info(" OPENING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static ssize_t
mycdrv_read(struct file *filp, char __user * buf, size_t count,
												 loff_t * f_pos)
{
	struct ASP_mycdrv *dev = filp->private_data; 
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (*f_pos >= ramdisk_size)
		goto out;
	if (*f_pos + count > ramdisk_size)
		count = dev->size - *f_pos;

	ssize_t num;
	num = count - copy_to_user(buf, dev->ramdisk + *f_pos, count);
	retval = num;
	*f_pos += retval;
    pr_info("\n READING function, nbytes=%lu, pos=%d\n", num, (int)*f_pos);
	
out: 
	up(&dev->sem);
	return retval;
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
    
	cdev_init(&dev->device, &character_driver_fops);
	dev->device.owner = THIS_MODULE;
	err = cdev_add (&dev->device, devno, 1);
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
		my_ASP_mycdrv[i].size = ramdisk_size;
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
