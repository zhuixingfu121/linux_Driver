#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/**
 * file name：led_twinkle
 * date: 2021-08-31  16:26
 * version：1.0
 * author:luatao
 * describe：字符设备驱动LED测试APP
 * 执行命令：./led_twinkle n LED闪烁延时间隔  单位s
 */


#define LEDOFF  1  /* 关闭LED */
#define LEDON   0  /* 打开LED*/

/* 主程序 */
int main(int argc, char *argv[])
{
    char *filename;  // 可执行文件名
    int fd,ret,n_interval ;  //  fd: 文件句柄 ret:函数操作返回值 n_interval Led的闪烁时间间隔
    unsigned char databuf[1] = {0}; // 缓冲区


    /* 先判断输入的参数 */
    if(argc !=  3){  // 本身文件名带1个 执行文件1个  读出或者写入一个 
       printf("parameter error!\r\n");
       return -1;
    }

    /* 分析参数 ，提取有用的信息 */
    filename = argv[1];  // 可执行文件名 
    n_interval = atoi(argv[2]); // 闪烁的延时间隔 


    
    /* 打开LED文件 */
    fd = open(filename, O_RDWR);  // 可读可写 
    if(fd < 0){
        printf("can't open file:%s\r\n",filename);
        return -1;
    }

    while(1){

        ret = write(fd, databuf, sizeof(databuf));
        if(ret < 0){
                printf("write file %s failed !\r\n",filename);
                goto close_file;   // 关闭文件 
        }else{ // 写入成功
        //   printf("led operation ok!\r\n");
        }
        
        /* 延时加翻转 */
        if(databuf[0]  == 1)
            databuf[0]  = 0;
         else
            databuf[0] = 1;
        sleep(n_interval); //延时n_interval秒

    }
 


close_file:
    /* 关闭文件 */
    ret = close(fd);
    if(ret < 0){
        printf("can't close file %s \r\n", filename);
        return -1;
    }

    return 0;
}

