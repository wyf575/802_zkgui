#ifndef USER_UART_H
#define USER_UART_H

#include "uart_driver.h"
#define CACHE_LEN 1024
typedef struct 
{
	unsigned char data[CACHE_LEN];
	unsigned int length;//在不超过200ms间隔真实收到的数据长度
}_sysUartDate;

int user_uart_receive(void *data, int needLen);
int user_uart_send(int uartID, void *data, int needLen);
void *user_uart_init_thread(void *arg);

#endif // !USER_UART_H
