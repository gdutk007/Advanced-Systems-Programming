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

#include "char_driver.h"

int NUMBER_OF_DEVICES = 3;
int my_major = 0;
int NUM_DEVICES = 0;

struct char_devices {
	unsigned char *data;
	unsigned long buffer_size; 
	unsigned long block_size;  
	struct semaphore sem; 
	struct cdev cdev;
};

module_param(NUM_DEVICES, int, S_IRUGO);

struct char_devices * char_devices = NULL;
struct class * char_device_class = NULL;

static int build_device(struct char_devices * device_ptr,
						 int index,struct class * char_class);

static void cleanup_char_device(int dev_to_destroy);

static loff_t mycdrv_lseek(struct file *filp, loff_t off, int whence);

static int mycdrv_open(struct inode *inode, struct file *filp)
{
	struct char_devices * dev;
	dev = container_of(inode->i_cdev, struct char_devices, cdev);
	filp->private_data = dev;
	pr_info(" OPENING device: %s:\n\n", MYDEV_NAME);
	
	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n", MYDEV_NAME);
	return 0;
}

static loff_t
 mycdrv_lseek(struct file *filp, loff_t off, int whence){
	struct char_devices *dev = filp->private_data;
	loff_t newpos;
	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = dev->buffer_size + off;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	
	if(newpos >= dev->buffer_size){
		unsigned char * temp = dev->data;
		dev->data = (unsigned char *)kzalloc(dev->buffer_size+1024,GFP_KERNEL);
		copy_to_user(dev->data,temp,dev->buffer_size);
		dev->buffer_size += 1024;
		kfree(temp);
		pr_info("Allocated a new buffer of size %li \n",dev->buffer_size);
		pr_info("New file position %lli \n",filp->f_pos);
	}
	return newpos;
}

static ssize_t
mycdrv_read(struct file *filp, char __user * buf, size_t count, loff_t * f_pos)
{
	struct char_devices * dev = filp->private_data;
	ssize_t retval = 0;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if(*f_pos >= dev->buffer_size)
		goto out;
	if(*f_pos + count > dev->buffer_size)
		count = dev->buffer_size - *f_pos;
	if( !dev || dev->data == NULL){
		goto out;
	}
	pr_info("The file position is: %lli. The count is %lu. file size %lu.\n",
							*f_pos, count, dev->buffer_size);
	if( copy_to_user(buf,dev->data+*f_pos,count) ){
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;
out: 
	up(&dev->sem);
	return retval;
}

static ssize_t
mycdrv_write(struct file *filp, const char __user * buf, size_t count,
	     loff_t * f_pos)
{
	struct char_devices * dev = filp->private_data;
	ssize_t retval = -ENOMEM;
	
	pr_info("The file position is: %lli \n",*f_pos);
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if(!dev || !dev->data)
		goto out;
	if(count+*f_pos > dev->buffer_size){
		pr_info("The count + position is greater than the buffer size \n");
		count = dev->buffer_size-*f_pos;
	}
	if(copy_from_user(dev->data+*f_pos, buf, count)){
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;
	if(dev->buffer_size < *f_pos){
		pr_info("Buffer size is less than position...\n");
		dev->buffer_size = *f_pos;
	}
out:
	up(&dev->sem);
	return retval;
}

static long mycdrv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	int err = 0;
	int retval = 0;
	struct char_devices *dev;
	if (_IOC_TYPE(cmd) != CDRV_IOC_MAGIC) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	dev = filp->private_data;

	switch (cmd)
	{
	case ASP_CLEAR_BUF:
		filp->f_pos = 0;
		kfree(dev->data);
		dev->buffer_size = ramdisk_size;
		dev->data = (unsigned char *)kzalloc(dev->buffer_size,GFP_KERNEL);
		break;
	
	default:
		pr_info("IOCTL cmd: %d Not recognized. \n",cmd);
		return -ENOTTY;
		break;
	}
	return retval;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.llseek = mycdrv_lseek,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.unlocked_ioctl = mycdrv_ioctl,
	.open = mycdrv_open,
	.release = mycdrv_release,
};

static int __init my_init(void)
{
	int err = 0;
	int i = 0;
	int devices_to_destroy = 0;
	dev_t dev = 0;
	if(NUM_DEVICES)
		NUMBER_OF_DEVICES = NUM_DEVICES;
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
	
	device_ptr->data = (unsigned char *)kzalloc(ramdisk_size,GFP_KERNEL);
	device_ptr->buffer_size = ramdisk_size;
	device_ptr->block_size = DEVICE_BLOCK_SIZE;
	sema_init(&device_ptr->sem,1);
	pr_info("Created a buffer of size: %li \n",device_ptr->buffer_size);
	
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
			kfree(char_devices[i].data);
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
	cleanup_char_device(NUMBER_OF_DEVICES);
	pr_info("\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_LICENSE("GPL v2");