#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "main_app/main_app.h"

/*串口设备节点*/
#define COM_GS0 "/dev/ttyHS0"
#define COM_GS1 "/dev/ttyHS1"
#define COM_AT "/dev/ttyUSB1"
#define COM_UART1_485 "/dev/ttyS1"


typedef struct
{
	char *uart_node;  /*串口设备节点选择*/
	int uart_fd;	  /*串口设备描述符*/
	int sync_or_asyn; /*同步(1)或异步选择(2)*/
	int set_baud;	  /*设置串口波特率 默认9600*/
	int data_bits;	  /*设置串口数据位 默认8位*/
	char parity;	  /*奇偶校验 N/n O/o E/e(默认) */
	int stop_bits;	  /*停止位 1(默认) 2*/
	bool enable_485;  /*默认关闭false 打开为true*/
}uart_init_t ;

#define UART_INIT_DEFAULT {COM_UART1_485,-1,1,9600,8,'n',1,false}
/**
* @brief         : 初始化串口设备
* @param[in]	 : uart_init_t *param 
* @param[out]    : uart_init_t *param 
* @return        : 负数错误 正数描述符
* @author        : aiden.xiang@ibeelink.com
* @date          : 2020.07.21
* @example       : uart_node=COMGS0,parity='n'
*/
int uart_init(uart_init_t *param);

/**
* @brief         : 串口发送数据
* @param[in]	 : const void *data
* @param[in]	 : data_lenth数据长度
* @return        : 负数错误 正数实际发送长度
* @author        : aiden.xiang@ibeelink.com
* @date          : 2020.07.21
*/
int uart_send(const int com_fd, const void *data, const int data_lenth);

/**
* @brief         : 串口接收数据
* @param[in]	 : 串口描述符 数据长度 超时时间 
* @param[out]	 : void *data
* @return        : 负数错误 正数实际接收长度
* @author        : aiden.xiang@ibeelink.com
* @date          : 2020.07.21
*/
int uart_receive(const int com_fd, void *data, const int data_len, int timeout_s);

typedef int (*uart_user_callback)(int,char *,int);//函数指针

#endif /*UART_DRIVER_H*/
