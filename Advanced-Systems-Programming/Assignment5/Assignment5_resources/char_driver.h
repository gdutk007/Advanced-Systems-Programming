#ifndef _CHAR_DRIVER_H_
#define _CHAR_DRIVER_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#define MYDEV_NAME "mycdrv"

#define ramdisk_size (size_t) (16*PAGE_SIZE)
#define DEVICE_BLOCK_SIZE 512

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)


#endif 