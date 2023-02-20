#include "main_app/hardware_tool/uart_driver.h"
#define COM_AT "/dev/ttyUSB1"

#include "utils/Log.h"
#define printf LOGD

static int openATUart(const char *pFileName, unsigned int baudRate)
{
    int uartId = open(pFileName, O_RDWR|O_NOCTTY);
    if (uartId <= 0) {
        return -1;
    } else {
        struct termios oldtio = { 0 };
        struct termios newtio = { 0 };
        tcgetattr(uartId, &oldtio);

        newtio.c_cflag = baudRate|CS8|CLOCAL|CREAD;
        newtio.c_iflag = 0;	// IGNPAR | ICRNL
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;	// ICANON
        newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
        newtio.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */
        tcflush(uartId, TCIOFLUSH);
        tcsetattr(uartId, TCSANOW, &newtio);

        // 设置为非阻塞
        fcntl(uartId, F_SETFL, O_NONBLOCK);
        return uartId;
    }
}

static int sendATData(int uartId,const unsigned char* mData, int len)
{
	if (uartId<=0) {
		return -1;
	}
	if (write(uartId, mData, len) != (int) len) {	// fail
		return 0;
	}
	return len;
}

static int receiveATData(const int com_fd, void *data, const int data_len,int timeout_500ms)
{
	for(int i = 0; i < timeout_500ms; i++)
    {
        int readNum = read(com_fd, data, data_len);
        if(readNum > 0)
        {
            return readNum;
        }else{
            printf("receiveATData-未读到数据\n");
            usleep(500000);
            continue;
        }
    }
    return 0;
}

/**
* @brief         : 发送AT指令及接收回复
* @param[in]	 : None 
* @param[out]    : None 
* @return        : 负数：非期望结果 0：串口失败 正数：实际长度
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.09.05
*/
int send_recv_AT_cmd(const char *cmd, char *need_ret, char *recv_buf, int buf_len)
{
    static int busy_flag = 0;
    while (busy_flag)
    {
        sleep(1);
    }
    busy_flag = 1;
//honestar karl.hong add 20210918
#if 1
	int uart_fd = -1;

	uart_fd = openATUart(COM_AT,B115200);
	if(uart_fd < 0){
		printf("open %s fail\n",COM_AT);
		busy_flag = 0;
		return 0;
	}

	sendATData(uart_fd, cmd, strlen(cmd));

	int length = receiveATData(uart_fd,recv_buf,buf_len,10);

	close(uart_fd);

	//printf("send_recv_AT_cmd[%d][%s]\n",length,recv_buf);

	if (strstr(recv_buf,need_ret)||NULL == need_ret){
		busy_flag = 0;
        return length;
	}else{
		busy_flag = 0;
        return -1;
	}
#else
//honestar karl.hong add 20210918
    static int uart_fd = -1;
    if (uart_fd < 0)
    {
        uart_fd = openATUart(COM_AT,B115200);
    }
    printf("==================cmd[%s] COM_AT[%s] uart_fd[%d]===================\n",cmd,COM_AT,uart_fd);
    if (uart_fd>0)
    {
        sendATData(uart_fd, cmd, strlen(cmd));
        int length = receiveATData(uart_fd,recv_buf,buf_len,10);
        printf("send_recv_AT_cmd[%d][%s]\n",length,recv_buf);
        if (strstr(recv_buf,need_ret)||NULL == need_ret)
        {
            busy_flag = 0;
            return length;
        }else
        {
            busy_flag = 0;
            return -1;
        }
    }else
    {
        close(uart_fd);
    }


    busy_flag = 0;
    return 0;	
#endif//honestar karl.hong add 20210918
}
