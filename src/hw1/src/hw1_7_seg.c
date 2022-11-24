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

// LED is connected to this GPIO
#define GPIO_First_Pin (11)
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

static char seg_for_c[11][9] = {
    // dp, g, f, e, d, c, b, a
    "00111111", // 0
    "00000110", // 1
    "01011011", // 2
    "01001111", // 3
    "01100110", // 4
    "01101101", // 5
    "01111101", // 6
    "00000111", // 7
    "01111111", // 8
    "01101111", // 9
    "00000000"
}
;

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t *off);
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
static int etx_open(struct inode *inode, struct file *file)
{

    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{


    int i = 0;
    printk("Now closing 7 seg since our program is done.\n");
    for (i = 0; i < 9 ;i++){
        printk("Now closing 7 seg.\n");
        gpio_set_value(GPIO_First_Pin + i, 0);
    }
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    uint8_t gpio_state = 0;

    // reading GPIO value
    gpio_state = gpio_get_value(GPIO_First_Pin);

    // write to user
    len = 1;
    if (copy_to_user(buf, &gpio_state, len) > 0)
    {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }

    pr_info("Read function : GPIO_21 = %d \n", gpio_state);

    return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{

    uint8_t rec_buf[10] = {0};
    uint8_t input_number;

    int i = 0;

    if (copy_from_user(rec_buf, buf, len) > 0)
    {
        pr_err("ERROR: Not all the bytes have been copied from user\n");
    }

    input_number = rec_buf[0] - 48;
    for (i = 0; i < 9 ;i++){
        gpio_set_value(GPIO_First_Pin + i, seg_for_c[input_number][i] - 48);
    }
    printk("Now controlling 7 seg.\n");
    printk("The number we're going to show: %d\n", input_number);
    printk("The gpio val we're going to set: %s\n", seg_for_c[input_number]);


    return len;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void){

    int i = 0;

    /*Allocating Major number*/
    if ((alloc_chrdev_region(&dev, 0, 1, "hw1_7seg")) < 0)
    {
        pr_err("Cannot allocate major number\n");
        goto r_unreg;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev, &fops);

    /*Adding character device to the system*/
    if ((cdev_add(&etx_cdev, dev, 1)) < 0)
    {
        pr_err("Cannot add the device to the system\n");
        goto r_del;
    }

    /*Creating struct class*/
    if ((dev_class = class_create(THIS_MODULE, "hw1_7seg_class")) == NULL)
    {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if ((device_create(dev_class, NULL, dev, NULL, "hw1_7seg")) == NULL)
    {
        pr_err("Cannot create the Device \n");
        goto r_device;
    }

    for (i = 0; i < 9; i++)
    {

        // Checking the GPIO is valid or not
        if (gpio_is_valid(GPIO_First_Pin + i) == false)
        {
            pr_err("GPIO %d is not valid\n", GPIO_First_Pin + i);
            goto r_device;
        }

        // Requesting the GPIO
        if (gpio_request(GPIO_First_Pin + i, "7seg_hw1") < 0)
        {
            pr_err("ERROR: GPIO %d request\n", GPIO_First_Pin + i);
            goto r_gpio;
        }

        // configure the GPIO as output
        gpio_direction_output(GPIO_First_Pin + i, 0);

        /* Using this call the GPIO will be visible in /sys/class/gpio/
        ** Now you can change the gpio values by using below commands also.
        ** echo 1 > /sys/class/gpio/gpio_first_pin/value (turn ON the LED)
        ** echo 0 > /sys/class/gpio/gpio_first_pin/value (turn OFF the LED)
        ** cat /sys/class/gpio/gpio_first_pin/value (read the value LED)
        **
        ** the second argument prevents the direction from being changed.
        */
        gpio_export(GPIO_First_Pin + i, false);

        pr_info("Device Driver Insert...Done!!!\n");
    }
    return 0;

r_gpio:
    for (i = 0; i < 9; i++)
        gpio_free(GPIO_First_Pin + i);
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
    int i = 0;
    for (i = 0; i < 9; i++)
    {
        gpio_unexport(GPIO_First_Pin + i);
        gpio_free(GPIO_First_Pin + i);
    }
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
MODULE_DESCRIPTION("hw1 led driver");
MODULE_VERSION("1.32");