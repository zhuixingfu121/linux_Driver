/**
*********************************************************************************************************
*                                        		driver_gpio
*                                      (c) Copyright 2021-2031
*                                         All Rights Reserved
*
* @File    : 
* @By      : liwei
* @Version : V0.01
* 
*********************************************************************************************************
**/

/**********************************************************************************************************
Includes 
**********************************************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/io.h>   
#include <linux/ioport.h>

#define    DEVICE_NAME     "driver_gpio"

#define PIO_BASE_ADDRESS 0x01C20800

#define   IOC_MAGIC  'w'
#define   IOCTL_TEST_ON   _IO(IOC_MAGIC,0)
#define   IOCTL_TEST_OFF   _IO(IOC_MAGIC,1)

#define GPIO_LED   (1)

//gpio寄存器
static volatile unsigned int __iomem		*gpio_pb_cf;    	
static volatile unsigned int __iomem		*gpio_pb_data;
static volatile unsigned char __iomem		*gpioe_base_va;
static volatile unsigned char __iomem		*gpioe_res;


/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int ioremap_init(void)
{
    gpioe_res = (__force volatile unsigned char __iomem *)request_mem_region(PIO_BASE_ADDRESS,0x300,"GPIOE_MEM");
	// struct resource *gpio_mem = request_mem_region(PIO_BASE_ADDRESS,0x300,"GPIOE_MEM");

    // if (gpio_mem == NULL) {
    //     pr_err("Failed to request GPIO memory region\n");
    //     return -ENODEV;
    // }

    // // 将 GPIO 寄存器映射到虚拟地址空间
    // gpioe_res = ioremap(PIO_BASE_ADDRESS, 0x300);

	if(gpioe_res == NULL)
	{
		printk("request_mem_region PIO_BASE_ADDRESS,0x28 fail\n");		
	}
    else
	  printk(KERN_EMERG DEVICE_NAME " ======================request_mem_region  ok ======================\n");
	
    //IO映射
	gpioe_base_va = (unsigned char*)ioremap(PIO_BASE_ADDRESS,0x300);


	if(gpioe_base_va == NULL)
	{	
		printk("ioremap PIO_BASE_ADDRESS,0x28 fail\n");	
	}	
  	    else
	  printk(KERN_EMERG DEVICE_NAME " ======================ioremap  ok ======================\n");
	
    //GPIO寄存器地址映射
	gpio_pb_cf 		= (unsigned int*)(gpioe_base_va+0xD8);
	gpio_pb_data 	=(unsigned int*)(gpioe_base_va+0xE8);

}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int gpio_set_bit(int bit)
{
    *gpio_pb_data 		|=  (0x0000001<<bit);    
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int gpio_clear_bit(int bit)
{
    *gpio_pb_data 		&= ~ (0x0000001<<bit);    
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int  gpio_config(void)
{
	printk(KERN_EMERG "gpio_pb_cf=%x\n" , *gpio_pb_cf );
	*gpio_pb_cf 		&= ~(0xff);
	printk(KERN_EMERG "gpio_pb_cf=%x\n" , *gpio_pb_cf );
	*gpio_pb_cf 		|= (0x11);
	printk(KERN_EMERG "gpio_pb_cf=%x\n" , *gpio_pb_cf );
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int gpio_init(void)
{
	//IO映射
    ioremap_init();
	//GPIO配置
    gpio_config();
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int gpio_open(struct inode *inode, struct file *file)
{
    gpio_init(); 
    printk(KERN_EMERG "======================gpio_open======================\n");
    return 0;
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static ssize_t gpio_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
    printk(KERN_EMERG "======================gpio_write======================\n");
    return 0;
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static ssize_t gpio_read(struct file *file,  char __user * buf, size_t count, loff_t *ppos)
{
    printk(KERN_EMERG "======================gpio_read ======================\n"); 
    return 0;
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int gpio_close(struct inode *inode, struct file *file)
{
    printk(KERN_EMERG "======================gpio_close ======================\n");
    return 0;
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static long gpio_ioctl( struct file *file,  unsigned int  cmd, unsigned long arg)
{
    switch(cmd)
    {
        case IOCTL_TEST_ON:
            gpio_set_bit(GPIO_LED);
            printk(KERN_EMERG "======================gpio_ioctl =======IOCTL_TEST_ON===\n");
        break;
        case IOCTL_TEST_OFF:                    
			gpio_clear_bit(GPIO_LED);
            printk(KERN_EMERG "======================gpio_ioctl =======IOCTL_TEST_OFF===\n");
        break;
        default:
        return -EINVAL;
    }
    return 0;
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static struct file_operations gpio_fops =
{
	.owner  =   THIS_MODULE,
	.open   =   gpio_open,     
	.write  =   gpio_write,
	.read =  gpio_read,
	.unlocked_ioctl = gpio_ioctl,
	.release =  gpio_close,
};

static struct cdev gpio_dev;
static     dev_t devno;
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static int __init gpio_driver_init(void)
{
    int ret;
    //申请设备号
   	ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		pr_err("alloc_chrdev_region failed!");
		return ret;
	}
	printk("MAJOR is %d\n", MAJOR(devno));
	printk("MINOR is %d\n", MINOR(devno)); 
	//注册设备
	cdev_init(&gpio_dev,  &gpio_fops);
	ret = cdev_add(&gpio_dev ,  devno, 1);
	if (ret < 0) {
		pr_err("cdev_add failed!");
		return ret;
	}
    printk(KERN_EMERG DEVICE_NAME " ======================gpio_driver_init======================\n");
	return 0;
}
/***********************************************************************************************************
* @描述	:  
***********************************************************************************************************/
static void __exit gpio_driver_exit(void)
{
    //注销设备
	cdev_del(&gpio_dev);
    //释放设备号
	unregister_chrdev_region(devno, 1);
    printk(KERN_EMERG DEVICE_NAME " ======================gpio_driver_exit======================\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
/***********************************************END*******************************************************/

