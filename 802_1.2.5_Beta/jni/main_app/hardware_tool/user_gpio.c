#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <linux/input.h>
#include <linux/ioctl.h>
#include <pthread.h>
#include "user_gpio.h"


#define RS485_GPIO 17
#define AUDIO_CTL_GPIO 23
#define AUDIO_EN_GPIO 22
#define MODEM_EN_GPIO 13


#define MODEM_WAKEUP_MCU_GPIO  50 //sim7600.pin69
#define GPIO77_LOCKNAME "GPIO77_Wakeup_modem"
#define MCU_WAKEUP_MODEM_GPIO  74 //sim7600.pin72
#define GPIO74_LOCKNAME "GPIO74_Wakeup_modem"
#define GPIO_SEELP_MODEM_MS    200

#define MAX_BUF 128


#define PIN72_ISR_LOCK  system_power_lock(GPIO74_LOCKNAME)
#define PIN72_ISR_UNLOCK system_power_unlock(GPIO74_LOCKNAME)

#define PIN87_LOCK  system_power_lock(GPIO77_LOCKNAME)
#define PIN87_UNLOCK system_power_unlock(GPIO77_LOCKNAME)


int g_netlight_status = 1;
int get_netlight_status(){
	return g_netlight_status;
}


void system_power_lock(const char *lock_id)
{
	int fd;

	fd = open("/sys/power/wake_lock", O_WRONLY);
	if (fd < 0) {
		printf("wake_lock,error %d\n",fd);
		return;
	}

	write(fd, lock_id, strlen(lock_id));

	close(fd);
}
void system_power_unlock(const char *lock_id)
{
	int fd;
	fd = open("/sys/power/wake_unlock", O_WRONLY);
	if (fd < 0) {
		printf("wake_unlock,error %d\n",fd);
		return;
	}

	write(fd, lock_id, strlen(lock_id));

	close(fd);
}

int gpio_file_create(int gpio)
{
	int sfd = -1;
	char checkstr[50] = {0};
	char configstr[10] = {0};
	int reto,len;

	sprintf(checkstr,"/sys/class/gpio/gpio%d/value",gpio);
	if(0 == access(checkstr, F_OK)){
		return 0;
	}

	sfd = open("/sys/class/gpio/export",O_WRONLY);
	if(sfd < 0){
		printf("%s:%d,open file error,%d\n",__FUNCTION__,__LINE__,sfd);
		return sfd;
	}

	len = sprintf(configstr,"%d",gpio);
	reto = write(sfd,configstr,len);

	if(reto != len){
		printf("create gpio(%d) files:%d,%d,%d\n",gpio,__LINE__,len,reto);
		return reto;
	}
	usleep(1000);
	reto = access(checkstr, F_OK);
	if(0 > reto){
		printf("%s:%d,gpio file(%s)not exist\n",__FUNCTION__,__LINE__,checkstr);
	}

	close(sfd);
	return reto;
}
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd;
	char buf[MAX_BUF];
 
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("gpio(%d)/direction\n",gpio);
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}
int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];
 
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value\n");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
	close(fd);
	return 0;
}
static int gpio_set_isr(unsigned gpio)
{
	char fvalue_path[MAX_BUF];
	int fd = -1,ret = -1;

	sprintf(fvalue_path,"/sys/class/gpio/gpio%d/edge",gpio);
	fd = open(fvalue_path,O_RDWR);
	if(fd < 0){
		printf("gpio_set_isr write %s error %d\n",fvalue_path,ret);
		return fd;
	}
	
	if((ret = write(fd,"both",5)) < 1){
		close(fd);
		printf("gpio_set_isr write %s error,%d\n",fvalue_path,ret);
		return ret;
	}
	close(fd);

	return (ret < 0)?ret:0;
}

int gpio_get_value(unsigned int gpio, int* value)
{
	int fd;
	char buf[MAX_BUF];
	char int_char[2];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value\n");
		return fd;
	}
 
 	read(fd, int_char, 2);
	if (strstr(int_char,"1")){
		*value = 1;
	}else if (strstr(int_char,"0")){
		*value = 0;
	}else{
		*value = 2;
	}
	
	close(fd);
	return 0;
}

#define MODEM_WAKEUP_MCU_TIME 100
int modem_ri_notify_mcu(void)
{
	PIN87_LOCK;
	gpio_set_value(MODEM_WAKEUP_MCU_GPIO,1);
	poll(0,0,MODEM_WAKEUP_MCU_TIME);
	gpio_set_value(MODEM_WAKEUP_MCU_GPIO,0);
	PIN87_UNLOCK;
	return 0;
}
int gpio_can_wakeup(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/can_wakeup", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

/**
* @brief         : 初始化RS485
* @param[in]	 : None 
* @param[out]    : None
* @return        : 成功返回0 失败其他
* @author        : aiden.xiang@ibeelink.com
* @date          : 2021.05.15
*/
static int rs485_gpio_init_ok = 0;
int init_rs485_control(void)
{
	int ret = 0;
	if(0 == rs485_gpio_init_ok)
	{
		/*此引脚 专用于硬件看门狗 与指示灯相连*/
		ret = gpio_file_create(RS485_GPIO);
		ret += gpio_set_dir(RS485_GPIO,1);
		ret += gpio_set_value(RS485_GPIO,0);
		if(ret==0){rs485_gpio_init_ok=1;}
	}
	return ret;
}

int rs485_send_control(void)
{
	if(rs485_gpio_init_ok)
	{
		gpio_set_value(RS485_GPIO,1);
		// printf("\n RS485_GPIO [%d] success!\n",1);
		return 0;
	}else{
		printf("\nplease init RS485_GPIO[%d] !\n",RS485_GPIO);
		return -1;
	}
}

int rs485_recv_control(void)
{
	if(rs485_gpio_init_ok)
	{
		gpio_set_value(RS485_GPIO,0);
		// printf("\n RS485_GPIO [%d] success!\n",0);
		return 0;
	}else{
		printf("\nplease init RS485_GPIO[%d] first!\n",RS485_GPIO);
		return -1;
	}
}

static int audio_gpio_init_ok = 0;
int init_audio_control(void)
{
	int ret = 0;
	if(0 == audio_gpio_init_ok)
	{
		/*此引脚 专用于硬件看门狗 与指示灯相连*/
		ret += gpio_file_create(AUDIO_EN_GPIO);
		ret += gpio_file_create(AUDIO_CTL_GPIO);
		ret += gpio_set_dir(AUDIO_EN_GPIO,1);
		ret += gpio_set_dir(AUDIO_CTL_GPIO,1);
		ret += gpio_set_value(AUDIO_EN_GPIO,0);//默认失能
		ret += gpio_set_value(AUDIO_CTL_GPIO,0);//默认失能
		if(ret==0){audio_gpio_init_ok=1;}
	}
	return ret;
}

int audio_open_control(void)
{
	if(audio_gpio_init_ok)
	{
		gpio_set_value(AUDIO_CTL_GPIO,1);
		gpio_set_value(AUDIO_EN_GPIO,1);
		printf("\n AUDIO_CTL_GPIO [%d] success!\n",1);
		return 0;
	}else{
		printf("\nplease init AUDIO_CTL_GPIO[%d] !\n",AUDIO_CTL_GPIO);
		return -1;
	}
}

int audio_close_control(void)
{
	if(audio_gpio_init_ok)
	{
		gpio_set_value(AUDIO_EN_GPIO,0);
		gpio_set_value(AUDIO_CTL_GPIO,0);
		printf("\n AUDIO_CTL_GPIO [%d] success!\n",0);
		return 0;
	}else{
		printf("\nplease init AUDIO_CTL_GPIO[%d] first!\n",AUDIO_CTL_GPIO);
		return -1;
	}
}

static int modem_gpio_init_ok = 0;
int modem_init_control(void)
{
	int ret = 0;
	if(0 == modem_gpio_init_ok)
	{
		/*此引脚 专用于硬件看门狗 与指示灯相连*/
		ret += gpio_file_create(MODEM_EN_GPIO);
		ret += gpio_set_dir(MODEM_EN_GPIO,1);
		ret += gpio_set_value(MODEM_EN_GPIO,0);//默认使能
		if(ret==0){modem_gpio_init_ok=1;}
	}
	return ret;
}

int modem_open_control(void)
{
	if(modem_gpio_init_ok)
	{
		gpio_set_value(MODEM_EN_GPIO,1);
		// sleep(15);
		// gpio_set_value(MODEM_EN_GPIO,0);
		printf("\n MODEM_EN_GPIO [%d] success!\n",MODEM_EN_GPIO);
		return 0;
	}else{
		printf("\nplease init MODEM_EN_GPIO[%d] !\n",MODEM_EN_GPIO);
		return -1;
	}
}
//honestar karl.hong add 20210918
int modem_poweron(void)
{
	if(modem_gpio_init_ok){
		gpio_set_value(MODEM_EN_GPIO,0);
		return 0;
	}else{
		printf("modem power gpio is not init\n");
		return -1;
	}
}

int modem_poweroff(void)
{
	if(modem_gpio_init_ok){
		gpio_set_value(MODEM_EN_GPIO,1);
		return 0;
	}else{
		printf("modem power gpio is not init\n");
		return -1;
	}
}
//honestar karl.hong add 20210918

int app_gpio_init(int gpio,int dir)
{
	int ret = 0;
	ret = gpio_file_create(gpio);
	ret += gpio_set_dir(gpio,dir);
	return ret;
}

/////////////////////////////////////////////////////////////////////
int gpio_init()
{
	int ret;
	ret = gpio_file_create(MODEM_WAKEUP_MCU_GPIO);
	ret += gpio_set_dir(MODEM_WAKEUP_MCU_GPIO,1);
	ret += gpio_set_value(MODEM_WAKEUP_MCU_GPIO,0);

	PIN72_ISR_UNLOCK;
	ret += gpio_file_create(MCU_WAKEUP_MODEM_GPIO);
	ret += gpio_set_dir(MCU_WAKEUP_MODEM_GPIO,1);
	ret += gpio_set_isr(MCU_WAKEUP_MODEM_GPIO);
	ret += gpio_can_wakeup(MCU_WAKEUP_MODEM_GPIO,0);
	return ret;
}

int gpio74_pullup(){
	gpio_set_value(MCU_WAKEUP_MODEM_GPIO,1);
	gpio_set_value(MODEM_WAKEUP_MCU_GPIO,1);
	return 0;
}

int gpio74_pulldown(){
	gpio_set_value(MCU_WAKEUP_MODEM_GPIO,0);
	gpio_set_value(MODEM_WAKEUP_MCU_GPIO,0);
	return 0;
}


void * gipo_wakeup_init(void* cmdpipefd)
{
	int ret = 0;
	int fd;
	char fvalue_path[MAX_BUF]={0};
	struct pollfd read_pollfd[2];
	int ch_time = -1;
    int init_cmdpipefd = *(int *)cmdpipefd;
	if(gpio_init() != 0){
		printf("gpio_init error\n");
		pthread_exit((void*)-1);
	}

	sprintf(fvalue_path,"/sys/class/gpio/gpio%d/value",MCU_WAKEUP_MODEM_GPIO);
	fd = open(fvalue_path,O_RDONLY|O_NONBLOCK);
	if(fd< 0){
		printf("open %s,fd error %d\n",fvalue_path,fd);
		pthread_exit((void*)-2);
	}

	memset(&read_pollfd,0,sizeof(read_pollfd));
	read_pollfd[0].fd=fd;
	read_pollfd[0].events = (POLLERR |POLLPRI);
	read_pollfd[1].fd=init_cmdpipefd;
	read_pollfd[1].events = POLLIN;

	while(1){
		printf("polling,time = %d\n",ch_time);
		ret = poll(read_pollfd, 2, ch_time);
		if(ret < 0){
			break;
		}
		if((ret == 0) && (ch_time == GPIO_SEELP_MODEM_MS)){
			char rbuff[3]={0};
			int lvalue = 0;
			read_pollfd[0].revents = 0;
			ret = lseek(fd,(off_t)0, SEEK_SET);
			ret += read(fd,rbuff,3);
			printf("Timeout GPIO is %s\n", rbuff);
			lvalue = atoi(rbuff);
			if(!lvalue){
				PIN72_ISR_UNLOCK;
				printf("goto sleep\n");
			}else{
				printf("wakeup on\n");
			}
			ch_time = -1;
		}
		if((read_pollfd[0].revents & read_pollfd[0].events) == read_pollfd[0].events){
			char rbuff[3]={0};
			read_pollfd[0].revents = 0;
			PIN72_ISR_LOCK;
			lseek(fd,(off_t)0, SEEK_SET);
			read(fd,rbuff,3);
			ch_time = GPIO_SEELP_MODEM_MS;//wait 200ms
		}
		if(read_pollfd[1].revents & read_pollfd[1].events){
			char rbuff[30]={0};
			read_pollfd[1].revents = 0;
			ret += read(read_pollfd[1].fd,rbuff,30);
			if(!strncmp(rbuff,"exit",5)){
				break;
			}
		}
	}
	close(fd);
	printf("DTR interrupt exit ok\n");
	return ((void*)0);
}





#if 0

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	char fvalue_path[MAX_BUF]={0};
	struct pollfd read_pollfd;
	int ch_time = -1;

	if(gpio_init() != 0){
		printf("gpio_init error\n");
		return -1;
	}

	sprintf(fvalue_path,"/sys/class/gpio/gpio%d/value",MCU_WAKEUP_MODEM_GPIO);
	fd = open(fvalue_path,O_RDONLY|O_NONBLOCK);
	if(fd< 0){
		printf("open %s,fd error %d\n",fvalue_path,fd);
		return -1;
	}

	memset(&read_pollfd,0,sizeof(read_pollfd));
	read_pollfd.fd=fd;
	read_pollfd.events = (POLLERR |POLLPRI);

	while(1){
		printf("polling,time = %d\n",ch_time);
		ret = poll(&read_pollfd, 1, ch_time);
		if(ret < 0){
			break;
		}
		if((ret == 0) && (ch_time == GPIO_SEELP_MODEM_MS)){
			char rbuff[3]={0};
			int lvalue = 0;
			read_pollfd.revents = 0;
			ret = lseek(fd,(off_t)0, SEEK_SET);
			ret += read(fd,rbuff,3);
			printf("Timeout GPIO is %s\n", rbuff);
			lvalue = atoi(rbuff);
			if(!lvalue){
				PIN72_ISR_UNLOCK;
				printf("goto sleep\n");
			}else{
				printf("wakeup on\n");
			}
			ch_time = -1;
		}
		if((read_pollfd.revents & read_pollfd.events) == read_pollfd.events){
			char rbuff[3]={0};
			int lvalue = 0;
			read_pollfd.revents = 0;
			PIN72_ISR_LOCK;
			ret = lseek(fd,(off_t)0, SEEK_SET);
			ret += read(fd,rbuff,3);
			printf("GPIO is %s\n", rbuff);
			lvalue = atoi(rbuff);
			if(!lvalue){
				ch_time = GPIO_SEELP_MODEM_MS;//wait 100ms
			}else{
				ch_time = -1;
			}
		}
	}
	return ret;
}
#endif

