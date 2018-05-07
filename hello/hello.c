#include<linux/init.h> //模块加载卸载需要的头文件  
#include<linux/module.h> //需要内核的一些认证和许可    
// 上面两个头文件对一个内核模块程序来说是必不可少的  
  
//这是模块初始化函数，在模块加载时触发被调用  
static int __init hello_init(void)  
{  
   printk("hello world module init!\n");  
   return 0;  
}  
  
//这是模块收尾处理函数，在模块卸载时触发被调用  
static void __exit hello_exit(void)  
{  
   printk("hello world module exit!\n");  
}  
  
//下面这两个相当于是注册函数，用来加载模块和卸载模块时触发的  
module_init(hello_init);  
module_exit(hello_exit);  
  
//模块加载时一个许可认证，如果没有编译的时候会报警告  
MODULE_LICENSE("Dual BSD/GPL");  
MODULE_AUTHOR("YU ZHI HUI"); //额外的信息，模块作者信息