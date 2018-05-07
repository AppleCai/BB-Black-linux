#include <linux/init.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <linux/io.h>

#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/cdev.h>

#include <linux/types.h>
#include <linux/fs.h>
//#include <mach/gpio.h>
#include <config/leds/gpio.h>
//#include <plat/mux.h>
#include <config/omap/mux.h>
#include <linux/gpio.h>

/*******************************************/
#define NAME "leds" 
#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))

static int major =241;//定义主设备号 
/*******************************************/
void led_on(void)
{ 
gpio_set_value(GPIO_TO_PIN(1,22), 1);
}

void led_off(void)
{ 
gpio_set_value(GPIO_TO_PIN(1,22), 0);
}

void led_init(void)
{ 
int result;
/* Allocating GPIOs and setting direction */
result = gpio_request(GPIO_TO_PIN(1,22), "Leds");//usr1
if (result != 0)
printk("gpio_request(1_22) failed!\n");
result = gpio_direction_output(GPIO_TO_PIN(1,22), 1);
if (result != 0)
printk("gpio_direction(1_22) failed!\n");

}

struct light_dev
{
struct cdev cdev;
unsigned char value;
};

struct light_dev *light_devp;

MODULE_AUTHOR("chenzhufly");
MODULE_LICENSE("Dual BSD/GPL");

// 打开和关闭函数
int light_open(struct inode *inode,struct file *filp)
{
struct light_dev *dev;

// 获得设备结构体指针
dev = container_of(inode->i_cdev,struct light_dev,cdev);
// 让设备结构体作为设备的私有信息
filp->private_data = dev;

return 0;
}

int light_release(struct inode *inode,struct file *filp)
{
return 0; 
}


// ioctl
long light_ioctl(struct file *filp,unsigned int cmd,
unsigned long arg)
{
struct light_dev *dev = filp->private_data;

switch(cmd)
{
case 0:
dev->value = 0;
led_off(); 
break;

case 1:
dev->value = 1;
led_on();
break;

default:

return -ENOTTY;
// break;
}

return 0;
} 

struct file_operations light_fops = 
{
.owner = THIS_MODULE,
.unlocked_ioctl = light_ioctl,
.open = light_open,
.release = light_release,
};
// 模块加载函数
int light_init(void)
{
int ret;
led_init();
printk(KERN_ALERT "led modules is install\n"); 
ret=register_chrdev(major,NAME,&light_fops); 
if(ret<0) 
{ 
printk("unable to register myled driver!\n"); 
return ret; 
} 
return 0; 

}

// 模块卸载函数
void light_cleanup(void)
{
unregister_chrdev(major,NAME); 
printk("Goodbye,cruel world!\n"); 
}

module_init(light_init);
module_exit(light_cleanup);
