#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h>    //GPIO



dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
char now_word = 0;

static char seg_for_c[27][16] = {
    "1111001100010001", // A
    "0000011100000101", // b
    "1100111100000000", // C
    "0000011001000101", // d
    "1000011100000001", // E
    "1000001100000001", // F
    "1001111100010000", // G
    "0011001100010001", // H
    "1100110001000100", // I
    "1100010001000100", // J
    "0000000001101100", // K
    "0000111100000000", // L
    "0011001110100000", // M
    "0011001110001000", // N
    "1111111100000000", // O
    "1000001101000001", // P
    "0111000001010000", // q
    "1110001100011001", // R
    "1101110100010001", // S
    "1100000001000100", // T
    "0011111100000000", // U
    "0000001100100010", // V
    "0011001100001010", // W
    "0000000010101010", // X
    "0000000010100100", // Y
    "1100110000100010", // Z
    "0000000000000000"
};

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp,char __user *buf, size_t len, loff_t *off);
static ssize_t etx_write(struct file *filp,const char *buf, size_t len, loff_t *off);
/******************************************************/

// File operation structure
static struct file_operations fops =
{
        .owner = THIS_MODULE,
        .read = etx_read,
        .write = etx_write,
        .open = etx_open,
        .release = etx_release,
};

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file){

    pr_info("Device File Opened...!!!\n");
    return 0;
}


/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file){

    pr_info("Device File Closed...!!!\n");
    return 0;
}


/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp,char __user *buf, size_t len, loff_t *off){

    // write to user
    len = 16;
    if (copy_to_user(buf, seg_for_c[now_word - 'A'], len) > 0){
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }

    return 0;
}


/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp,const char __user *buf, size_t len, loff_t *off){

    uint8_t rec_buf[10] = {0};

    if (copy_from_user(rec_buf, buf, len) > 0){
        pr_err("ERROR: Not all the bytes have been copied from user\n");
    }

    now_word = rec_buf[0];

    return len;
}


/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
    /*Allocating Major number*/
    if ((alloc_chrdev_region(&dev, 0, 1, "mydev")) < 0){
        pr_err("Cannot allocate major number\n");
        goto r_unreg;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev, &fops);

    /*Adding character device to the system*/
    if ((cdev_add(&etx_cdev, dev, 1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_del;
    }

    /*Creating struct class*/
    if ((dev_class = class_create(THIS_MODULE, "etx_class")) == NULL){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if ((device_create(dev_class, NULL, dev, NULL, "mydev")) == NULL){
        pr_err("Cannot create the Device \n");
        goto r_device;
    }


    return 0;

r_device:
    device_destroy(dev_class, dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&etx_cdev);
r_unreg:
    unregister_chrdev_region(dev, 1);

    return -1;
}


/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!\n");
}


module_init(etx_driver_init);
module_exit(etx_driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
MODULE_VERSION("1.32");