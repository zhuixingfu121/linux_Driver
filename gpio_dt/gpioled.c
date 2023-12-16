#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <linux/of_gpio.h>




/**
 * file name：gpioled
 * date: 2021-08-31  15:51
 * version：1.0
 * author:luatao
 * describe：gpioLed device drive
 */


#define GPIOLED_CNT  1     /* 设备号个数 */
#define GPIOLED_NAME        "gpioled"      /* 设备名*/

#define LEDOFF 1  /* 关灯 */
#define LEDON  0  /* 开灯 */


/* 设备结构体 自定义 */
struct gpioled_dev{
    dev_t devid;     /*设备号  */
    struct cdev cdev;  /* cdev */
    struct class *class;  /* 类*/
    struct device *device;  /* 设备 */
    int major;   /* 主设备号 */
    int minor;  /* 次设备号 */

    struct device_node *nd;  /* 设备节点 */
    int led_gpio;    /* led所使用的GPIO编号 */
 };

/* 定义一个设备结构体 */
struct gpioled_dev gpioled;   /* led 设备 */


/* 打开设备 */
static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &gpioled;  /* 设置私有数据 */
    printk("led open!\r\n");
    return 0;
}

/* 从设备读取数据 */
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    printk("led read !\r\n");
    return 0;
}

/* 往设备写数据 */
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret;
    unsigned char datebuf[1];  // 接收数据缓冲区
    unsigned char cmd;  // LED的命令 0 打开  1 关闭

    struct gpioled_dev *dev = filp->private_data;  // 获取私有数据 

     /* 被写入的内核空间的数据 ，需要用户空间向内核空间发送数据 */
    ret = copy_from_user(datebuf, buf, cnt);  // 接收发送过来的数据
    if(ret ==0){  // 成功返回0  失败返回有多少个B未完成copy
        // printk("kernel receivedata: %s !\r\n", datebuf);
    }else{
        printk("kernel receivedata failed!\r\n");
        return -1;
    }

    /* 处理接收的数据 */
    cmd = datebuf[0];  

    /* 控制LED执行命令 */
    if(cmd == LEDON){  // 打开LED
        gpio_set_value(dev->led_gpio, 0);  // 设置值
    }else if(cmd ==LEDOFF){  // 关闭LED
        gpio_set_value(dev->led_gpio, 1);
    }else{
        printk("cmd is invalid!\r\n");
    }
    return 0;
}

/* 释放设备 */
static int led_release(struct inode *inode, struct file *filp)
{
    //printk("led release!\r\n");
    return 0;
}


/* 设备操作函数结构体  */
static struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

/* 驱动入口函数 */
static int __init led_init(void)
{
    int ret;  // 返回值

    /* 获取设备数中的属性数据  */
    /* 1. 获取设备节点 /led*/
    gpioled.nd = of_find_node_by_path("/leds/red_led");  // 通过绝对路径查找设备节点
    if(gpioled.nd == NULL){
        printk("led node no find!\r\n");
        return -EINVAL;  /* 无效参数 不知道这个返回值是啥意思，我觉得返回一个负数就可以，这个值是23，不知道有没有处理*/
    }

    /* 2. 获取设备树中的gpio属性 得到LED所使用的gpio编号 */
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "gpios", 0);
    if(gpioled.led_gpio < 0 ){
         printk("can't get led-gpio\r\n");
        return -EINVAL;  /* 无效参数 不知道这个返回值是啥意思，我觉得返回一个负数就可以，这个值是23，不知道有没有处理*/
    }
    printk("led-gpio num = %d \r\n", gpioled.led_gpio);  // 打印获取的led-gpio属性值

    /* 3. 设置GPIO1_IO03为输出，并且输出高电平，默认关闭LED灯 */
    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if(ret < 0){
        printk("can't set gpio!\r\n");
    }

    /* 注册字符设备驱动 */

    /* 1. 创建设备号 */
    if(gpioled.major){  // 定义了设备号 
        gpioled.devid = MKDEV(gpioled.major, 0 );  // 根据主设备号和次设备号合成设备号 
        register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);  // 注册设备号 
    }else{  // 没有定义设备号 动态生成 
        alloc_chrdev_region(&gpioled.devid,0,GPIOLED_CNT, GPIOLED_NAME ); // 申请设备号
        gpioled.major = MAJOR(gpioled.devid);  // 获取主设备号
        gpioled.minor = MINOR(gpioled.devid);  // 获取次设备号
    }
    printk("gpioled major = %d,minor = %d\r\n",gpioled.major, gpioled.minor);  // 打印主设备号和次设备号

    /* 2. 初始化 cdev */
    gpioled.cdev.owner = THIS_MODULE;  
    cdev_init(&gpioled.cdev, &gpioled_fops);  // 初始化cdev
    /* 3. 添加cdev */
    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT ); // 向linux系统添加cdev

     /* 自动创建设备节点文件 */
    /* 4. 创建类 */
    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);  // 创建类 
    if(IS_ERR(gpioled.class)){
        return PTR_ERR(gpioled.class);
    }
    /* 创建设备 */
    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
      if(IS_ERR(gpioled.device)){
        return PTR_ERR(gpioled.device);
    }

    return 0;
}


/* 驱动出口函数 */
static void __exit led_exit(void)
{

    /*  注销字符设备驱动 */
    cdev_del(&gpioled.cdev);  /* 删除 cdev */
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT ); /* 注销设备号 */

    device_destroy(gpioled.class, gpioled.devid);  /* 注销设备  */
    class_destroy(gpioled.class);  /* 注销类 */


    printk("led drive unregsister ok !\r\n");
}

/* 加载驱动入口和出口函数 */
module_init(led_init);
module_exit(led_exit);

/* LICENSE 和 AUTHOR 信息*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("luatao");

