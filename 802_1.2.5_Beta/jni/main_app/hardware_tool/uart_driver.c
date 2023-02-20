#include "uart_driver.h"
#include "errno.h"
#include "user_gpio.h"
extern void initRS485Driver(int fd);

static struct termios old_opt;
static struct termios new_opt;
#define GPIO_DEBUG  0

/*
fd:文件描述符
rate:波特率
data_bits:数据位
parity:奇偶校验位
stop_bits:停止位
*/
static void set_terminal(int fd, int rate, int data_bits, char parity, int stop_bits)
{
	int set_speed = 0;
	/* get current options*/
	tcgetattr(fd, &old_opt); //用于获取终端的相关参数
	new_opt = old_opt;

	/* set baud rate*/
	switch (rate)
	{
	case 0:
		//set_speed = B0;
		set_speed = B115200;
		break;
	case 50:
		set_speed = B50;
		break;
	case 75:
		set_speed = B75;
		break;
	case 110:
		set_speed = B110;
		break;
	case 134:
		set_speed = B134;
		break;
	case 150:
		set_speed = B150;
		break;
	case 200:
		set_speed = B200;
		break;
	case 300:
		set_speed = B300;
		break;
	case 600:
		set_speed = B600;
		break;
	case 1200:
		set_speed = B1200;
		break;
	case 1800:
		set_speed = B1800;
		break;
	case 2400:
		set_speed = B2400;
		break;
	case 4800:
		set_speed = B4800;
		break;
	case 9600:
		set_speed = B9600;
		break;
	case 19200:
		set_speed = B19200;
		break;
	case 38400:
		set_speed = B38400;
		break;
	case 57600:
		set_speed = B57600;
		break;
	//case 76800: set_speed = B76800; break;
	case 115200:
		set_speed = B115200; 
		break;
	default:
		printf("Unknow terminal speed!! set terminal rate to B0\n");
		set_speed = B0;
		break;
	}
	//设置波特率
	cfsetispeed(&new_opt, set_speed);
	cfsetospeed(&new_opt, set_speed);
	/*
	   cfsetispeed(&new_opt, B9600);
	   cfsetospeed(&new_opt, B9600);
	*/

	/* enable the receive and set local mode...	*/
	new_opt.c_cflag |= (CLOCAL | CREAD);

	// disble software flow control
	new_opt.c_iflag &= ~(IXON | IXOFF | IXANY |ICRNL);
	/* Disable hardware flow control */
	new_opt.c_cflag &= ~CRTSCTS;

	// set parity
	switch (parity)
	{
	case 'n':
	case 'N': // no parity
		new_opt.c_cflag &= ~PARENB;

		// must have this line, or data bit will be removed as parity bit.
		new_opt.c_iflag &= ~ISTRIP; //don't strip input parity bit
		/*
			new_opt.c_iflag &= ~INPCK;
			new_opt.c_iflag &= ~ISTRIP; //don't strip input parity bit
			*/
		break;
	case 'o':
	case 'O': // odd parity奇校验
		new_opt.c_cflag |= (PARODD | PARENB);
		new_opt.c_iflag |= INPCK;
		new_opt.c_iflag |= ISTRIP; //strip input parity bit
		break;
	case 'e':
	case 'E': // even parity偶校验
		new_opt.c_cflag |= PARENB;
		new_opt.c_cflag &= ~PARODD;
		new_opt.c_iflag |= INPCK;
		new_opt.c_iflag |= ISTRIP; //strip input parity bit
		break;
	default:
		printf("Unknow parity, set to even parity\n");
		new_opt.c_cflag |= PARENB;
		new_opt.c_cflag &= ~PARODD;
		new_opt.c_iflag |= INPCK;
		new_opt.c_iflag |= ISTRIP; //strip input parity bit
	}

	// set stop bits
	if (1 == stop_bits)
	{
		new_opt.c_cflag &= ~CSTOPB; // one stop bit
	}
	else if (2 == stop_bits)
	{
		new_opt.c_cflag |= CSTOPB; // two stop bit
	}
	else
	{
		printf("Unknow terminal stop bit, set to one stop bit!!!\n");
		new_opt.c_cflag &= ~CSTOPB;
	}

	// set how many data bits
	new_opt.c_cflag &= ~CSIZE;
	if (8 == data_bits)
	{
		new_opt.c_cflag |= CS8; // eight data bits
	}
	else if (7 == data_bits)
	{
		new_opt.c_cflag |= CS7; // seven data bits
	}
	else if (6 == data_bits)
	{
		new_opt.c_cflag |= CS6; // six data bits
	}
	else if (5 == data_bits)
	{
		new_opt.c_cflag |= CS5;
	}
	else
	{
		printf("Unknow terminal data bits, set to 8 data bits!!\n");
		new_opt.c_cflag |= CS8;
	}

	/* classical input*/

	//new_opt.c_lflag |= (ICANON | ECHO | ECHOE);

	/* raw input*/
	new_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* select raw output*/
	new_opt.c_oflag &= ~OPOST;

	/* set the new option of the port*/
	int ret;
	//设置终端参数
	ret = tcsetattr(fd, TCSANOW, &new_opt);
	if (ret < 0)
	{
		printf("Can't set terminol\n");
	}
	//tcflush函数用于清空输入、输出缓冲区
	tcflush(fd, TCIFLUSH);
}

static inline char *getdevname(char *path)
{
	char *ptr;

	if (path == NULL)
		return NULL;

	ptr = path;

	while (*ptr != '\0')
	{
		if ((*ptr == '\\') || (*ptr == '/'))
			path = ptr + 1;

		ptr++;
	}

	return path;
}

static int uart_wakeup(char *devpath, unsigned char bwakeup)
{
	int ufd = -1;
	char powrpath[256] = {0};
	sprintf(powrpath, "/sys/class/tty/%s/device/power/control", getdevname(devpath));
	ufd = open(powrpath, O_WRONLY);
	if (ufd < 0)
	{
		printf("%s error %d\n", powrpath, ufd);
		return -1;
	}
	else
	{
		if (bwakeup)
		{
			write(ufd, "on", 2);
		}
		else
		{
			write(ufd, "auto", 4);
		}
		close(ufd);
	}
	return 0;
}


/*
* 关闭串口
* com_fd        串口的文件描述符号
* oldstdtio     串口配置信息
*/

static int close_uart(const int com_fd, const struct termios oldstdtio)
{
	if (com_fd < 0)
	{
		return 3;
	}
	tcsetattr(com_fd, TCSANOW, &oldstdtio);
	tcsetattr(0, TCSANOW, &oldstdtio);
	//uart_wakeup(COMGS0,0);
	close(com_fd);
	printf("UART Close OK,fd = %d\n", com_fd);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_UART_COUNT 1
typedef struct 
{
	int com_fd;
	bool enable_485;
}matchTable;
static matchTable table[MAX_UART_COUNT] = {0};
int insert_table(int com_fd,bool enable_485)
{
	if (com_fd<=0 || false == enable_485)
	{
		return -1;
	}
	int i = 0;
	for ( i = 0;table[i].com_fd>0 && i < MAX_UART_COUNT; i++)
	{
		if (table[i].com_fd ==  com_fd)
		{
			table[i].enable_485 = enable_485;
			return com_fd;
		}
	}
	if (i<MAX_UART_COUNT)
	{
		table[i].enable_485 = enable_485;
		table[i].com_fd = com_fd;
		return com_fd;
	}
	return -1;
}

bool find_enable485_by_com(int com_fd)
{
	for ( size_t i = 0;table[i].com_fd>0 && i < MAX_UART_COUNT; i++)
	{
		if (table[i].com_fd ==  com_fd)
		{
			return table[i].enable_485;
		}
	}
	return false;
}

int uart_init(uart_init_t *param)
{
	struct termios oldtio, newtio, newstdtio;
	pthread_t th_a, th_b;
	void *retval;
	if (0 != access(param->uart_node, F_OK))
	{
		printf("device %s not exist\n", param->uart_node);
		return -1;
	}
	if (1 == param->sync_or_asyn)
	{
		param->uart_fd = open(param->uart_node, O_RDWR | O_NOCTTY);/*同步方式*/
	}else
	{
		param->uart_fd = open(param->uart_node,O_RDWR|O_NOCTTY | O_NDELAY |O_NONBLOCK);
	}
	if (param->uart_fd < 0)
	{
		printf("Open UART ERR:param->uart_fd = %d\n", param->uart_fd);
		return -2;
	}
	set_terminal(param->uart_fd, param->set_baud, param->data_bits, param->parity, param->stop_bits);
	uart_wakeup(param->uart_node, 1);/*阻止uart休眠 否则该uart只能发送 而一旦进入休眠后无法再接收数据*/
	if (true == param->enable_485)
	{
		if(param->uart_fd == insert_table(param->uart_fd,true))
		{
			/*这里打开485对应的控制引脚*/
			#if GPIO_DEBUG
			init_rs485_control();
			#else
			initRS485Driver(param->uart_fd);
			#endif
		}
	}
	return param->uart_fd; /*返回描述符 一定大于0*/
}

int uart_send(const int com_fd, const void *data, const int data_length)
{
	static int busy_flag = 0; /*默认忙标志为空闲*/
	int cnt = -1;
	int i;
	int err_cnt = 0;
	// printf("\nbusy_flag[%d] Send_data[%d]:[\n", busy_flag, data_length);
	while (1 == busy_flag)
	{
		usleep(1000 * 200);
	}
	busy_flag = 1; /*忙标志置为忙状态*/
#if GPIO_DEBUG
	if (true == find_enable485_by_com(com_fd))
	{
		/* 做控制gpio的操作 更改为发送状态 */
		rs485_send_control();
		usleep(100);//50
	}
#endif
	cnt = write(com_fd, data, data_length);
	// printf("send mse num-%d\n", cnt);
	if (cnt != data_length)
	{
		printf("Write_error_again:");
		write(com_fd, data, data_length);
		printf("--LEN write:%d LEN actual:%d\n", cnt, data_length);
	}

#if GPIO_DEBUG
	if (true == find_enable485_by_com(com_fd))
	{ 
		/* 做控制gpio的操作 更改为接收状态*/
		if (data_length>10)
		{
			usleep(12*1000);//确保在9600情况下都能发送完成
		}else
		{
			usleep(100);//20
		}
		rs485_recv_control();
	}
#endif
	busy_flag = 0; /*忙标志置为空闲状态*/
	return cnt;
}

int uart_receive(const int com_fd, void *data, const int data_len,int timeout_s)
{
	int readlen = 0;
	int time_count = 0;
	int i = 0;
	while (readlen <= 0)
	{
		readlen = read(com_fd, data, data_len);
		/*有接收到数据 直接返回*/
		if (readlen <= 0)
		{
			usleep(500*1000);
			time_count++;
			if (time_count>(timeout_s*2))
			{
				printf("uart_receive timeout[%d]\n", timeout_s);
				break;
			}
		}
	}
	return readlen;
}


#if 0
#define DEBUG_UART 0
void my_printf(const char *file,const char *func,int line,const char *format,...)
{
    char *format_data = (char *)malloc(1024);
    char *send_data = (char *)malloc(1024*2);
	va_list list;
	va_start(list, format);
	vsnprintf(format_data,1024,format,list);
	va_end(list);
	snprintf(send_data,1024*2,"[%s-%d] %s",func,line,format_data);
    uart_send(DEBUG_UART,send_data,strlen(send_data));
	free(send_data);
    free(format_data);
}
#endif

/*
sync.是强制将所有页面缓冲区都更新到磁盘上。
fsync.是强制将某个fd涉及到的页面缓存更新到磁盘上(包括文件属性等信息).
fdatasync.是强制将某个fd涉及到的数据页面缓存更新到磁盘上。
*/

/*
1F 0F 00 01 05 00 34 A2
1F 0F 00 04 02 04 C8 00 00 00 54 36
1F 0F 00 04 02 04 C8 00 00 F0 
1F 0F 00 04 02 04 C8 00 00 F0


1F 0F 00 01 05 01 00 A4 B8
*/
