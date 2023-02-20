#include "appconfig.h"
#include "baselib/WatchdogManager.h"
#include "net/Module4GManager.h"
#include "hotplugdetect/hotplugdetect.h"

#include "main_app/main_app.h"
#include "main_app/user_app/user_app.h"
#include "main_app/device_info.h"
#include "main_app/net_listener.h"
#include "main_app/platform/udisk_user.h"
#include "main_app/sntp_client/sntp_client.h"
#include "main_app/cJSON_file/read_json_file.h"
#include "main_app/ibeelink_app/ibeelink_http.h"
#include "main_app/ibeelink_app/ibeelink_mqtt.h"
#include "main_app/linux_tool/busy_box.h"
#include "main_app/debug_printf/debug_printf.h"
#include "main_app/message_queue/message_queue.h"
#include "main_app/hardware_tool/user_uart.h"

/*平台无关的播放策略及任务处理层*/
#include "player_task/playerUList.h"
#include "player_task/playerNList.h"
#include "player_task/playControl.h"
#include "platform/mediaPlayer.h"

/**
 * @brief 线程注册封装函数
 * 
 */
#define THREAD_MAX_COUNT 20
static pthread_t user_pthread_id[THREAD_MAX_COUNT];
static int thread_count = 0;
int register_user_thread(void *p_thread, void *function_callback, void *param)
{
	/*留足两个线程空间以便后面创建新线程*/
	DEBUG_LOGIC_CHECK_V2(thread_count > (THREAD_MAX_COUNT - 1), "thread_count overflow!");
	int ret = pthread_create(&user_pthread_id[thread_count], NULL, p_thread, (void *)function_callback);
	if (ret != 0)
	{
		printf("Create pthread[%d] error!\n", thread_count);
	}
	else
	{
		printf("Create pthread[%d] success!\n", thread_count);
		thread_count++;
	}
	return ret;
}

void unregister_user_thread(void)
{
    /*用户显示注册的线程资源回收*/
    int i = 0;
    for(i=0;i<thread_count;i++)
    {
        printf("\nthread_count i[%d]", i);
        pthread_join(user_pthread_id[i],NULL);
    }
}

/*ethan del 20211208
void* system_reboot_thread(void *arg)
{
    WATCHDOGMANAGER->openWatchdog(WDG_TIMEOUT_S);
    while (1)
    {
        long int nowTime = get_system_time_stamp(NULL);
        int min_set = get_min_sec_int(nowTime);
        debuglog_yellow("nowTime[%ld]min_set[%d]\n",nowTime,min_set);
		//printf("5min[%s]5min\n",__FUNCTION__);
        // if (nowTime>1623131142 && get_min_sec_int(nowTime)>2357)
        // {
        //     system("sync");
        //     system("reboot");
        // }
        //TODO: 在这里喂狗 当前看门狗 只看应用是否挂了
        WATCHDOGMANAGER->keep_alive();
        sleep(60);
    }
}
*/

/*
*获取1280x1024是否需要在横屏状态下需要分屏
*return 1-1280x1024有特殊需求；0-没得
*/
uint8_t Get1280x1024SpecialDemand(void)
{
#if FANG_PING_1280x1024
#if FANG_PING_1280x1024_SPECIAL_DEMAND
    return 1;
#endif
#endif
    return 0;
}
void *app_main_thread(void *arg)
{
    /*检查必要目录是否存在不存在则创建*/
	check_creat_file_dir(APP_VERSION_FILE,FILE_TYPE);
    check_creat_file_dir(PLAY_LIST_FILE,FILE_TYPE);
    check_creat_file_dir(PLAY_BUFF_FILE,FILE_TYPE);
    check_creat_file_dir(CONFIG_FILE_NAME,FILE_TYPE);
	check_creat_file_dir(USER_NLIST_ROOT,DIR_TYPE);
    check_creat_file_dir(USER_DEFAULT_ROOT,DIR_TYPE);

    /*用于U盘升级系统时的脚本提取版本号*/
    debuglog_yellow("\n============================ Device Version [%s] ========================\n",MAIN_APP_VERSION);
    save_pairs_file(APP_VERSION_KEY,MAIN_APP_VERSION,APP_VERSION_FILE);

    /*用于初始化U盘相关配置*/
    initUDiskConfig();

    /*进行业务线程创建*/
    initPlayThread();
    register_user_thread(keep_net_thread, NULL, NULL);
    register_user_thread(handleConfigAndADThread,NULL,NULL);
    register_user_thread(handleStartPlayerThread,NULL,NULL);
    if(1 == Get1280x1024SpecialDemand())
    {
        register_user_thread(handlePlayPicThread,NULL,NULL);
    }
	else if((GetRotateSetting() == 90) || (GetRotateSetting() == 270))
    {
		register_user_thread(handlePlayPicThread,NULL,NULL);//honestar karl.hong add 20210829
	}

    register_user_thread(user_uart_init_thread, NULL, NULL);
    //register_user_thread(system_reboot_thread, NULL, NULL);
    
    return 0;
}


