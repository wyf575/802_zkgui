#include "appconfig.h"

#include "baselib/WatchdogManager.h"
#include "main_app/net_listener.h"
#include "main_app/main_app.h"
#include "main_app/linux_tool/busy_box.h"
#include "main_app/AT_tool/user_AT.h"
#include "main_app/hardware_tool/user_gpio.h"
#include "main_app/sntp_client/sntp_client.h"
#include "main_app/device_info.h"
#include "main_app/ibeelink_app/ibeelink_http.h"
#include "main_app/user_app/user_app.h"
#include "player_task/configPlayer.h"
#include "platform/widgetUI.h"

#include "main_app/player_task/playControl.h"//honestar karl.hong add 20210830

#define CMD_0 "ping -c 1 -q www.baidu.com"
#define CMD_1 "ping -c 1 -q  www.qq.com"
#define CMD_2 "ping -c 1 -q mqtt.ibeelink.com"
#define CMD_3 "/sys/class/net/eth0/carrier"
#define CMD_4 "/sys/class/net/wlan0/carrier"

#include "utils/Log.h"
#define printf LOGD
extern const char *UnGetDeviceSN(void);
/**
 * 线程间需要用到的网络状态信息 
*/
typedef struct
{
    int net_status_now;  /*现在的网络状态 一般进程的线程仅用此变量即可*/
    int net_status_last; /*上一次的网络状态 网络状态维护的进程中需要*/
} pthread_net_status;    /*@ref:net_status_type*/

/*start_ethan_2021.11.26优化仅播放图片黑屏添加*/
extern int getNListCount(void);

/*end*/

/*用于线程间同步网络状态 注意只能在一个线程中写数据*/
static pthread_net_status thread_net_status_t = {NET_STATUS_UNKNOW};
/**
* @brief         : 本进程其他线程获取网络状态接口
* @param[in]	 : None 静态变量 仅能通过此接口访问
* @param[out]    : None 
* @return        : None
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.08.21 
*/
int get_network_status(void)
{
	printf("get_network_status\n");
    return thread_net_status_t.net_status_now;
}


static int __get_network_status(void)
{
    thread_net_status_t.net_status_now = NET_STATUS_FAILE;
    if (0 == call_system_cmd(CMD_0) || 0 == call_system_cmd(CMD_1) || 0 == call_system_cmd(CMD_2))
    {
        thread_net_status_t.net_status_now = NET_STATUS_OK;
    }
    return thread_net_status_t.net_status_now;
}

static int app_status = NET_STATUS_FAILE;
int get_app_network_status(void)
{
	printf("get_app_network_status\n");
	return app_status;
}
void set_app_network_status(net_status_type status)
{
	app_status = status;
}

static int getNetConnectStatus(char* netType){
	char path[50];
	sprintf(path, "/sys/class/net/%s/carrier", netType);
	printf("path = %s\n", path);
	int skfd = open(path, O_RDONLY);
	if(skfd < 0){
		 printf("cat %s error!\n", netType);
		 return 0;
	}else
		 printf("cat %s success!\n", netType);
	char netStatus[2];
	int netRet = read(skfd, netStatus, 1);
	printf("getNetConnectStatus - %s - %d\n", netStatus, netRet);
	if(netRet > 0 && strncmp(netStatus, "1", 1) == 0){
		printf("1\n") ;
		close(skfd);
		return 1;
	}else{
		printf("0\n") ;
		close(skfd);
		return 0;
	}
}

networkType get_network_type(void)
{   
    if (1 == getNetConnectStatus("eth0"))
    {
        return NETWORKTYPE_ETHERNET;
    }
	/*no wifi */
	/*
    if (1 == getNetConnectStatus("wlan0"))
    {
        return NETWORKTYPE_WIFI;
    }
	*/
    return NETWORKTYPE_4G;
}

void sysReboot(void)
{
    modem_open_control();
    sleep(15);
    call_system_cmd("sync");
    call_system_cmd("reboot");
}

//honestar karl.hong add 20210918
static int getPidByProcName(const char *name)
{
	int fd;
	int pid;
	char tmp[10];
	char *p;
	
	char cmd[100];

	fd = open(name,O_RDONLY);
	if(fd < 0){
		printf("open %s fail\n",name);
		return -1;
	}
	p = tmp;
	for(int i = 0;i < sizeof(tmp);i++){
		read(fd,p,1);
		if(*p == '\n') break;
		else if(*p == EOF) goto err;
		p++;
	}

	close(fd);

	pid = atoi(tmp);

	return pid;	
	
err:
	close(fd);
	return -1;
}

static void restart4GModem(void)
{
	int status;
	int pid;
	char cmd[50];
	int ret;

	printf("modem_poweroff\n");
	ret = modem_poweroff();
	printf("modem_poweroff %d\n",ret);
	sleep(2);
	printf("modem_poweron\n");
	modem_poweron();
	printf("modem_poweron_on\n");

	status = UnIsDeviceWorking();

	if(status == -1)	return;

	system(" ps -ef pid,args | grep -w \"udhcpc -i eth2 -s /etc/init.d/udhcpc.script\" | grep -v 'grep' | grep -v '{' | awk '{print $1}' > /tmp/udhcpc_eth2");

	pid = getPidByProcName("/tmp/udhcpc_eth2");

	if(pid == -1) return;

	sprintf(cmd,"kill -9 %d",pid);

	system(cmd);

	system("udhcpc -i eth2 -s /etc/init.d/udhcpc.script &");
}
//honestar karl.hong add 20210918

/*
*检测播放
*input true-分屏，false-不分屏
*视频播放进度喝视频总长度获取不到
*ethan add 2021.11.30
*/
void CheckPlayTask(uint8_t split_screen)
{
	static uint8_t video_count = 0;
	static uint8_t pic_count = 0;
	static uint8_t no_video_count = 0;
	static uint8_t no_pic_count = 0;
	static bool old_video_widget_state = false;
	static bool video_widget_state = false;
	bool split_screen_play = false;
	bool un_split_screen_play = false;

	//1、获取是否有广告
	if(getNListCount() > 0)
	{
		if(true == get_video_isPlaying())//用户层检测是否在控制视频播放
		{
			printf("[%s] dingpa1\n",__FUNCTION__);
			video_widget_state = GetVideoWidgetState();//获取播放进度
			if((video_widget_state == old_video_widget_state) && (false == video_widget_state))
			{
				if(video_count++ >= 2)
				{
					video_count = 0;
					request_player_start_stop(REQUEST_STOP,NLIST_VIDEO,__FUNCTION__);
					printf("[%s] check video restart play\n",__FUNCTION__);
					request_player_start_stop(REQUEST_START,NLIST_VIDEO,__FUNCTION__);
				}
			}
			else
			{
				video_count = 0;
			}

			printf("[%s] video_widget_state %d old_video_widget_state %d\n",__FUNCTION__,video_widget_state,old_video_widget_state);
			old_video_widget_state = video_widget_state;
			if(0 != no_video_count){no_video_count = 0;}
			split_screen_play = true;
		}
		else//未在播放视频
		{
			no_video_count++;
			printf("[%s] no_video_count[%d]\n",__FUNCTION__,no_video_count);
			if(no_video_count > 10)
			{
				no_video_count = 0;
			}
		}

		if(true == GetUserPicPalyState())//用户层，检测图片是否在播放
		{
			printf("[%s] dingpa2\n",__FUNCTION__);
			if(false == GetPicisVisible())//flything层，检测图片是否在播放
			{
				if(pic_count++ >= 2)
				{
					pic_count = 0;
					request_player_start_stop(REQUEST_STOP,NLIST_PIC,__FUNCTION__);
					request_player_start_stop(REQUEST_START,NLIST_PIC,__FUNCTION__);
					printf("[%s] check pic restart play\n",__FUNCTION__);
				}
			}
			else
			{
				pic_count = 0;
			}

			if(0 != no_pic_count){no_pic_count = 0;}
			split_screen_play = true;
		}
		else
		{
			no_pic_count++;
			printf("[%s] no_pic_count[%d]\n",__FUNCTION__,no_pic_count);
			if(no_pic_count > 10)
			{
				no_pic_count = 0;
			}
		}

		if((true == split_screen) && (false == split_screen_play))//分屏时视频图片都未播放，开始修复
		{
			printf("[%s] fenping no pic and video play\n",__FUNCTION__);
			request_player_start_stop(REQUEST_STOP,NLIST_PIC,__FUNCTION__);
			request_player_start_stop(REQUEST_STOP,NLIST_VIDEO,__FUNCTION__);
			request_player_start_stop(REQUEST_START,NLIST_VIDEO,__FUNCTION__);
		}

		if((no_pic_count > 2) && (no_video_count > 2))//未分屏时视频图片都未播放，开始修复
		{
			no_pic_count = 0;
			no_video_count = 0;
			printf("[%s] no fenping no pic and video play\n",__FUNCTION__);
			request_player_start_stop(REQUEST_STOP,NLIST_PIC,__FUNCTION__);
			request_player_start_stop(REQUEST_STOP,NLIST_VIDEO,__FUNCTION__);
			request_player_start_stop(REQUEST_START,NLIST_VIDEO,__FUNCTION__);
		}
	}
}

/**
* @brief         : 网络维护(重新询网以及通知)
* @param[in]	 : None 
* @param[out]    : None 
* @return        : 成功返回0 失败其他
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.09.06
* 将喂狗操作移至此线程中。看门狗超时时间由原来300s缩短至60s
*/
void *keep_net_thread(void *arg)
{
    int failedCount = 0;
	char device_sn[10];
    modem_init_control();
	memset(device_sn,0,strlen(device_sn));
	WATCHDOGMANAGER->openWatchdog(WDG_TIMEOUT_S);
    while (1)
    {
		
		//printf("5min[%s]5min\n",__FUNCTION__);
        networkType type = get_network_type();
        printf("=========================type[%d]=========================\n",type);
        if(NET_STATUS_OK == __get_network_status())
        {
            failedCount = 0;
			printf("+++++++++++++++++++++++++failedCount[%d]+++++++++++++++++++++++++\n",failedCount);
            static char flag = 0;
            if (0==flag)
            {
                /*联网后第一时间 处理的操作 且只处理一次*/
                flag = 1;
                update_sntp_time(NTP_SERVER1);
                start_user_app();
            }
			int signal_level = 4;
            if(type == NETWORKTYPE_4G)
            {
                signal_level = UnGetCsqLevel();
            }
            setSignalWidgetLoad(type,signal_level);

			/*2021.10.19 ethan add start*/
			if(strlen(device_sn) < 6)
			{
				char *copy_sn = UnGetDeviceSN();
				if (NULL != copy_sn)
				{
					sprintf(device_sn,"%s",copy_sn);
				}
				printf("get_sn_%s\n",device_sn);
			}
			/*2021.10.19 ethan add end*/
        }else
        {
            failedCount++;
            printf("=========================failedCount[%d]=========================\n",failedCount);
            if (failedCount%6==0)
            {
                //sysReboot();
                if (type == NETWORKTYPE_4G){
					printf("[karl]restart 4G module======================================\n");
					restart4GModem();
				}
            }
            setSignalWidgetLoad(NETWORKTYPE_NONE,0);
        }
        updateQRcodeWidget();
		//updateOtherConfigs();
        setVersionWidgetLoad(MAIN_APP_VERSION,device_sn);

		//ethan add 2021.11.30
		if((1 == Get1280x1024SpecialDemand()) || (GetRotateSetting() == 90) || (GetRotateSetting() == 270))//判断是否是分屏播放
		{
			CheckPlayTask(true);
		}
		else//横屏未分屏播放
		{
			CheckPlayTask(false);
		}

		WATCHDOGMANAGER->keep_alive();
        sleep(10);
    }
}

