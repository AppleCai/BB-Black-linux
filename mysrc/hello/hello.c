#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/fs.h>  
#include <linux/cdev.h>  
#include <asm/uaccess.h>  
#include <linux/device.h>  
  
MODULE_LICENSE ("GPL");  
  
int hello_major       = 240;  
int hello_minor       = 0;  
int number_of_devices = 1;  
char read_data[128]   = "foobar not equal to barfoo";  
char write_data[128]  = {};  
  
struct cdev cdev;  
struct class *my_class;  
dev_t devno;  
  
static int hello_open (struct inode *inode, struct file *file)  
{  
    printk (KERN_INFO "Hey! device opened\n");  
  
    return 0;  
}  
  
static int hello_release (struct inode *inode, struct file *file)  
{  
    printk (KERN_INFO "Hmmm... device closed\n");  
  
    return 0;  
}  
  
ssize_t hello_read (struct file *filp, char *buff, size_t count, loff_t *offp)  
{  
    ssize_t result = 0;  
  
    if (count > 127) count = 127;  
  
    if (copy_to_user (buff, read_data, count))   
    {  
        result = -EFAULT;  
    }  
    else  
    {  
        printk (KERN_INFO "wrote %d bytes\n", (int)count);  
        result = count;  
    }  
  
    return result;  
}  
  
ssize_t hello_write (struct file *filp, const char  *buf, size_t count, loff_t *f_pos)  
{  
    ssize_t ret = 0;  
  
    if (count > 127) return -ENOMEM;  
    if (copy_from_user (write_data, buf, count)) {  
        ret = -EFAULT;  
    }  
    else {  
        write_data[count] = '\0';  
        printk (KERN_INFO"Received: %s\n", write_data);  
        ret = count;  
    }  
  
    return ret;  
}  
  
struct file_operations hello_fops = {  
    .owner   = THIS_MODULE,  
    .open    = hello_open,  
    .release = hello_release,  
    .read    = hello_read,  
    .write   = hello_write  
};  
  
static void char_reg_setup_cdev (void)  /* 在系统中添加一个设备 */  
{  
    int error;  
  
    cdev_init (&cdev, &hello_fops);  
    cdev.owner = THIS_MODULE;  
    error = cdev_add (&cdev, devno, number_of_devices);  
    if (error)  
        printk (KERN_NOTICE "Error %d adding char_reg_setup_cdev", error);  
}  
  
static int char_dev_create (void)  /* 为设备自动创建节点 */  
{  
    my_class = class_create(THIS_MODULE,"char_hello");  // 类名为char_class  
    if(IS_ERR(my_class))   
    {  
        printk("Err: failed in creating class.\n");  
        return -1;   
    }  
    device_create(my_class,NULL,devno,NULL,"hello");      // 设备名为hello  
  
    return 0;  
}  
  
static int __init hello_2_init (void)  
{  
    int result;  
    devno = MKDEV (hello_major, hello_minor);  
  
    result = register_chrdev_region (devno, number_of_devices, "hello");  
  
    if (result < 0) {  
        printk (KERN_WARNING "hello: can't get major number %d\n", hello_major);  
        return result;  
    }  
    char_dev_create();  
    char_reg_setup_cdev();  
  
    printk (KERN_INFO "char device registered\n");  
  
    return 0;  
}  
  
static void __exit hello_2_exit (void)  
{  
    cdev_del (&cdev);  
    device_destroy(my_class, devno);         //delete device node under /dev//必须先删除设备，再删除class类  
    class_destroy(my_class);                 //delete class created by us  
    unregister_chrdev_region (devno, number_of_devices);  
    printk (KERN_INFO "char driver cleaned up\n");  
}  
  
module_init (hello_2_init);  
module_exit (hello_2_exit);  
