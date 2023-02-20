#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <linux/serial.h>
#include <sys/ioctl.h>

#define RS485_GPIO 17

void initRS485Driver(int fd)
{
    struct serial_rs485 rs485conf;

    memset(&rs485conf,0,sizeof(rs485conf));

    rs485conf.padding[0] = RS485_GPIO;   //用来控制slaver收发的gpio index
    rs485conf.delay_rts_after_send = 1000;  //发送完最后一个字节需要的delay，单位：us
    rs485conf.flags |= SER_RS485_RTS_ON_SEND; //发送前拉高gpio，打开SER_RS485_RTS_AFTER_SEND指的是发送后拉高
    rs485conf.flags |= SER_RS485_ENABLED; // 使能本串口485模式，默认禁用
    // ioctl(iHandle, TIOCSRS485, &rs485conf);

    if (ioctl (fd, TIOCSRS485, &rs485conf) < 0) {
        printf("Error handling. See errno 2 \n");
		/* Error handling. See errno. */
	}

}




