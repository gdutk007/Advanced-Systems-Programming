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

//static char *ramdisk;
#define ramdisk_size (size_t) (16*PAGE_SIZE)
#define DEVICE_BLOCK_SIZE 512

int NUMBER_OF_DEVICES = 3;
int my_major = 0;

struct char_devices {
	unsigned char *data;
	unsigned long buffer_size; 
	unsigned long block_size;  
	struct mutex mutex; 
	struct cdev cdev;
};

struct char_devices * char_devices = NULL;
struct class * char_device_class = NULL;

static int build_device(struct char_devices * device_ptr,
						 int index,struct class * char_class);

static void cleanup_char_device(int dev_to_destroy);


static int mycdrv_open(struct inode *inode, struct file *filp)
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
	// int nbytes;
	// if ((lbuf + *ppos) > ramdisk_size) {
	// 	pr_info("trying to read past end of device,"
	// 		"aborting because this is just a stub!\n");
	// 	return 0;
	// }
	// nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	// *ppos += nbytes;
	// pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	// return nbytes;

	return 0;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	// int nbytes;
	// if ((lbuf + *ppos) > ramdisk_size) {
	// 	pr_info("trying to read past end of device,"
	// 		"aborting because this is just a stub!\n");
	// 	return 0;
	// }
	// nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	// *ppos += nbytes;
	// pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	//return nbytes;
	return 0;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
};

static int __init my_init(void)
{
	int err = 0;
	int i = 0;
	int devices_to_destroy = 0;
	dev_t dev = 0;

	/* Get a range of minor numbers (starting with 0) to work with */
	err = alloc_chrdev_region(&dev,0,NUMBER_OF_DEVICES,MYDEV_NAME);
	if (err < 0) {
		printk(KERN_WARNING "[target] alloc_chrdev_region() failed\n");
		return err;
	}
	my_major = MAJOR(dev);

	char_device_class = class_create(THIS_MODULE, MYDEV_NAME);
	if(IS_ERR(char_device_class)) {
		err = PTR_ERR(char_device_class);
		goto fail;
	}

	/* Allocate the array of devices */
	char_devices = (struct char_devices *)kzalloc(
		NUMBER_OF_DEVICES*sizeof(struct char_devices), 
		GFP_KERNEL);
	if (char_devices == NULL) {
		err = -ENOMEM;
		goto fail;
	}
    /* Construct devices */
	for (i = 0; i < NUMBER_OF_DEVICES; ++i) {
		err = build_device(&char_devices[i],i,char_device_class);
		if (err) {
			devices_to_destroy = i;
			goto fail;
		}
	}	
	pr_info("\nSucceeded in registering character device %s\n",MYDEV_NAME);
	return 0;
fail: 
	// we do cleanup here
	cleanup_char_device(devices_to_destroy);
	return err;
}

int build_device(struct char_devices * device_ptr, int index,struct class * char_class){
    int err = 0;
	dev_t devno = MKDEV(my_major,index);
	struct device *device = NULL;
	
	device_ptr->data = NULL;
	device_ptr->buffer_size = ramdisk_size;
	device_ptr->block_size = DEVICE_BLOCK_SIZE;
	mutex_init(&device_ptr->mutex);
	
	// init char device
	cdev_init(&device_ptr->cdev, &mycdrv_fops);
	device_ptr->cdev.owner = THIS_MODULE; 
	cdev_add(&device_ptr->cdev,devno,1);
	if (err)
	{
		printk(KERN_WARNING "[target] Error %d while trying to add %s%d",
			err, MYDEV_NAME, index);
		return err;
	}
	device = device_create(char_class,NULL,devno,NULL,MYDEV_NAME"%d",index);
	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			err, MYDEV_NAME, index);
		cdev_del(&device_ptr->cdev);
		return err;
	}
	return 0;
}

static void cleanup_char_device(int dev_to_destroy){
	int i;
	if(char_devices){
		for(i = 0; i < dev_to_destroy; ++i){
			device_destroy(char_device_class, MKDEV(my_major, i));
			cdev_del(&char_devices[i].cdev);
			//kfree(char_devices[i].data);
			mutex_destroy(&char_devices[i].mutex);
		}
		kfree(char_devices);
	}
	if(char_device_class){
		class_destroy(char_device_class);
	}
	unregister_chrdev_region(MKDEV(my_major,0),NUMBER_OF_DEVICES);
	return;
}

static void __exit my_exit(void)
{
	// cdev_del(my_cdev);
	// unregister_chrdev_region(first, count);
	// kfree(ramdisk);
	cleanup_char_device(NUMBER_OF_DEVICES);
	pr_info("\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_LICENSE("GPL v2");





// #define MYDEV_NAME "mycdrv"

// #define ramdisk_size (size_t) (16 * PAGE_SIZE) // ramdisk size 

// #ifndef DEFAULT_DRIVERS
// #define DEFAULT_DRIVERS 3  // default number of drivers 
// #endif

// struct ASP_mycdrv {
// 	struct cdev device;
// 	char *ramdisk;
// 	struct semaphore sem;
// 	int devNo;
// 	int size;
// };

// static struct ASP_mycdrv * my_ASP_mycdrv;

// //NUM_DEVICES defaults to 3 unless specified during insmod
// int NUM_DEVICES = DEFAULT_DRIVERS;
// static int major = 0, minor = 0;
// static struct class *driver_class = NULL;
// // params
// // module_param(major, int, S_IRUGO);
// // module_param(minor, int, S_IRUGO);
// // module_param(NUM_DEVICES, int, S_IRUGO);


// #define CDRV_IOC_MAGIC 'Z'
// #define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)



// void cleanup_momdule(void);

// void cleanup_momdule(void){
// 	int i;
// 	dev_t devno = MKDEV(major,minor);
// 	if(my_ASP_mycdrv){
// 		for(i = 0; i < NUM_DEVICES; ++i){
// 			kfree(my_ASP_mycdrv[i].ramdisk);
// 			device_destroy(driver_class, MKDEV(major, i));
// 			cdev_del(&my_ASP_mycdrv[i].device);	
// 		}
// 		kfree(my_ASP_mycdrv);
// 	}

// 	if (driver_class)
// 		class_destroy(driver_class);

// 	unregister_chrdev_region(devno,NUM_DEVICES);
// }

// /*
//  * Empty out the scull device; must be called with the device
//  * semaphore held.
//  */
// // what does this really do? 
// // int scull_trim(struct scull_dev *dev)
// // {
// // 	// struct scull_qset *next, *dptr;
// // 	// int qset = dev->qset;   /* "dev" is not-null */
// // 	// int i;

// // 	// for (dptr = dev->data; dptr; dptr = next) { /* all the list items */
// // 	// 	if (dptr->data) {
// // 	// 		for (i = 0; i < qset; i++)
// // 	// 			kfree(dptr->data[i]);
// // 	// 		kfree(dptr->data);
// // 	// 		dptr->data = NULL;
// // 	// 	}
// // 	// 	next = dptr->next;
// // 	// 	kfree(dptr);
// // 	// }
// // 	// dev->size = 0;
// // 	// dev->quantum = scull_quantum;
// // 	// dev->qset = scull_qset;
// // 	// dev->data = NULL;
// // 	return 0;
// // }

// static int mycdrv_open(struct inode *inode, struct file *filp)
// {

// 	pr_info("We are opening one of the devices...\n");
// 	// unsigned int mj = imajor(inode);
// 	// unsigned int mn = iminor(inode);

// 	// struct ASP_mycdrv * dev = &my_ASP_mycdrv[mn]; 
// 	// filp->private_data = dev;
// 	// /* now trim to 0 the length of the device if open was write-only */
// 	// if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
// 	// 	if (down_interruptible(&dev->sem))
// 	// 		return -ERESTARTSYS;
// 	// 	scull_trim(dev); /* ignore errors */
// 	// 	up(&dev->sem);
// 	// }
// 	pr_info(" OPENING device: %s:\n\n", MYDEV_NAME);
// 	return 0;
// }

// static int mycdrv_release(struct inode *inode, struct file *file)
// {
// 	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
// 	return 0;
// }

// static ssize_t
// mycdrv_read(struct file *filp, char __user * buf, size_t count,
// 												 loff_t * f_pos)
// {
// // 	struct ASP_mycdrv *dev = filp->private_data; 
// // 	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

// // 	if (down_interruptible(&dev->sem))
// // 		return -ERESTARTSYS;
// // 	if (*f_pos >= ramdisk_size)
// // 		goto out;
// // 	if (*f_pos + count > ramdisk_size)
// // 		count = dev->size - *f_pos;

// // 	ssize_t num;
// // 	num = count - copy_to_user(buf, dev->ramdisk + *f_pos, count);
// // 	retval = num;
// // 	*f_pos += retval;
// //     pr_info("\n READING function, nbytes=%lu, pos=%d\n", num, (int)*f_pos);
	
// // out: 
// // 	up(&dev->sem);
// //	return retval;
// return 0;
// }

// static ssize_t
// mycdrv_write(struct file *filp, 
// 		const char __user * buf, size_t count,
// 	     loff_t * f_pos)
// {
// // 	struct ASP_mycdrv *dev = filp->private_data; 
// // 	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
	
// // 	if (down_interruptible(&dev->sem))
// // 		return -ERESTARTSYS;

// // 	if(count+*f_pos >= ramdisk_size)
// // 		pr_info("Trying to write past buffer\n");
// // 		goto out;

// // 	if (copy_from_user(dev->ramdisk+*f_pos,buf,count)) {
// // 		retval = -EFAULT;
// // 		goto out;
// // 	}
// // 	*f_pos += count;
// // 	retval = count;
// // 	pr_info("\n WRITING function, nbytes=%lu, pos=%d\n", 
// // 									count, (int)*f_pos);
// // out: 
// // 	up(&dev->sem);
// //	return retval;
// return 0;
// }

// static const struct file_operations character_driver_fops = {
// 	.owner = THIS_MODULE,
// 	.read = mycdrv_read,
// 	.write = mycdrv_write,
// 	.open = mycdrv_open,
// 	.release = mycdrv_release,
// };

// /*
//  * Set up the char_dev structure for this device.
//  */
// static void setup_cdev(struct ASP_mycdrv *dev, int index,
// 						struct class * class)
// {
// 	struct device *device = NULL;
// 	int err, devno = MKDEV(major, minor + index);
//     pr_info("Sett");
// 	cdev_init(&dev->device, &character_driver_fops);
// 	dev->device.owner = THIS_MODULE;
// 	//dev->device.ops   = &character_driver_fops;
// 	err = cdev_add (&dev->device, devno, 1);
	
// 	device = device_create(class, NULL, /* no parent device */ 
// 		devno, NULL, /* no additional data */
// 		MYDEV_NAME "%d", minor + index);


// 	/* Fail gracefully if need be */
// 	if (err)
// 		printk(KERN_NOTICE "Error %d adding character device %d", err, index);
// }

// static int init_driver(void){
// 	int result, i;
// 	int err = 0;
// 	dev_t dev = 0;
// 	result = alloc_chrdev_region(&dev, minor, NUM_DEVICES,"scull");
// 	major = MAJOR(dev);
//     printk(KERN_WARNING "DRIVER: Initializing Driver... \n");

// 	driver_class = class_create(THIS_MODULE,MYDEV_NAME);
// 	if (IS_ERR(driver_class)) {
// 		err = PTR_ERR(driver_class);
// 		goto fail;
// 	}
// 	printk(KERN_WARNING "DRIVER: Initializing Driver... \n");
// 	if (result < 0) {
// 		printk(KERN_WARNING 
// 			"DRIVER: can't allocate a major driver, result: %d\n", result);
// 		return result;
// 	}
// 	// allocating a dynamic number of devices here
// 	my_ASP_mycdrv = kmalloc(NUM_DEVICES * sizeof(struct ASP_mycdrv), GFP_KERNEL);
// 	if (!my_ASP_mycdrv) {
// 		result = -ENOMEM;
// 		goto fail;  /* Make this more graceful */
// 	}
// 	memset(my_ASP_mycdrv, 0x00, NUM_DEVICES * sizeof(struct ASP_mycdrv));
// 	printk(KERN_WARNING "DRIVER: Initializing Driver... \n");
// 	// now we need to initialize them
// 	for(i = 0; i < NUM_DEVICES; ++i){
// 		my_ASP_mycdrv[i].devNo = i;
// 		my_ASP_mycdrv[i].ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
// 		my_ASP_mycdrv[i].size = ramdisk_size;
// 		sema_init(&my_ASP_mycdrv[i].sem,1);
// 		setup_cdev(&my_ASP_mycdrv[i],i,driver_class);
// 	}
//     pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
//     printk(KERN_WARNING "DRIVER: Initializing Driver... \n");
// 	return 0;
//   fail:
// 	cleanup_module();
// 	return result;
// }

// static void exit_driver(void){
//     pr_info("\ndevice unregistered\n");
// }

// module_init(init_driver);
// module_exit(exit_driver);

// MODULE_AUTHOR("user");
// MODULE_LICENSE("GPL v2");
