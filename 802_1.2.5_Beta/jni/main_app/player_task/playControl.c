/**
 * @file playControl.c
 * @author aiden.xiang@ibeelink.com
 * @brief 播放任务控制中心: 列表维护/播放策略
 * @version 0.1
 * @date 2021-06-22
 * 总体结构：
 *      @controller：playControl.c
 *      @service:    playNlist.c ; playUlist.c
 *      @bean:       beanClist
 * @copyright Copyright (c) 2021
 * 
 */

#include "playerNList.h"
#include "playerUList.h"
#include "configPlayer.h"
#include "playControl.h"
#include "../ibeelink_app/ibeelink_http.h"
#include "../message_queue/message_queue.h"
#include "../linux_tool/busy_box.h"
#include "../linux_tool/user_util.h"
#include "../device_info.h"
#include "../platform/udisk_user.h"
#include "../platform/mediaPlayer.h"
#include "../platform/widgetUI.h"
#include "../debug_printf/debug_printf.h"

static msg_queue_thread ad_task_queue_id = {0};
static msg_queue_thread player_task_queue_id = {0};
static msg_queue_thread play_pic_queue_id = {0};//停止播放图片用

extern void startPlayPicture(playTask picTask); 


/**
 * @brief Get the set currentList object
 * 设置和获取当前播放列表类型
 * @param setList 
 * @return int 
 */
int get_set_currentList(int setList)
{
    static int currentList = NO_LIST;
    if (setList!=GET_LIST)
    {
        currentList = setList;
    }
    return currentList;
}


/**
 * @brief 解析播放列表与配置
 * @caller http拉取播放任务与配置
 * @param rootStr http获取的JSON字串
 */
void playerListAndConfig(char *rootStr)
{
    cJSON *root = cJSON_Parse(rootStr);
    cJSON *data = cJSON_GetObjectItem(root,"data");
    cJSON *param = cJSON_GetObjectItem(data,"param");//参数配置信息
    cJSON *paramData = cJSON_GetArrayItem(param, 0);
    if (NULL != paramData)
    {
        /*只取第一个参数组*/
        int id = parseConfigParams(paramData);
        replyConfigResult(id);
    }
    cJSON *playList = cJSON_GetObjectItem(data,"playList");//广告信息
    if (NULL != playList)
    {
        parseNListparams(playList);
    }
    cJSON_Delete(root);

    bool needStart = deleteInvalidFiles();
    /*需要重置视频播放游标 从列表头开始播放*/
    resetNFirstCall();
    reportADResult();
    static bool firstCall = true;
    if (true == firstCall || needStart )
    {
        firstCall = false;
        request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
    }
}


/**
 * @brief 校验、切换、启停播放
 * @attention 具有抢占优先级
 * @param UDiskPath U盘目录
 * U盘播放用
 */
void otherMode2UList(char *UDiskPath)
{
    /* 校验是否存在有效广告文件 否则不处理 每次扫描都会销毁之前的列表 */
    /*如果当前正在播放U盘视频则忽略处理*/
    if(ULIST != get_set_currentList(GET_LIST) && scanUDiskFile(UDiskPath)>0)
    {
        //request_player_start_stop(REQUEST_STOP,__FUNCTION__);//ethan del 2021.11.29
        request_player_start_stop(REQUEST_STOP,NO_LIST,__FUNCTION__);//第二个参数无实际意义
        get_set_usb_path(UDiskPath);
        request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
    }
}

/**
 * @brief 切换和启停播放
 * @attention 具有抢占优先级
 * U盘播放用
 */
void UList2OtherMode(void)
{
    
    //request_player_start_stop(REQUEST_STOP,__FUNCTION__);//ethan del 2021.11.29
    request_player_start_stop(REQUEST_STOP,NO_LIST,__FUNCTION__);//第二个参数无实际意义
    get_set_usb_path("");
    request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
}


/**
 * @brief 向列表/参数维护任务启停
 * 联网请求获取最新视频JSON列表 联网请求最新参数组
 * @author aiden.xiang@ibeelink.com
 * @date 2021.05.11
 * @return int 成功返回0
 */
int get_ad_and_config_params(void)
{
    if (ad_task_queue_id.max_depth>0)
    {
        char *url =  message_queue_message_alloc_blocking(&ad_task_queue_id);
        snprintf(url, 256, GET_AD_AND_CONFIG_URL , UnGetMD5DeviceId()); 
        debuglog_yellow("get_ad_and_config_params[%s]\n",url);
        message_queue_write(&ad_task_queue_id, url);
        return 0;
    }
    return -1;
}

void stopPlayPictureBlock(void)
{
    uint8_t *tmp = message_queue_message_alloc_blocking(&play_pic_queue_id);
    *tmp = 0;
    message_queue_write(&play_pic_queue_id,tmp);
}


/**
 * @brief 向播放策略任务请求启停
 * @attention 目的是让任务能够以队列形式执行
 * @param needStart true:启动 false:停止
 * @return int 
 * 2021.11.29根据图片或者视频分别停止播放
 * 开始播放时不用管播放图片还是视频
 */
//int request_player_start_stop(PlayType needStart,const char *caller)//ethan del 2021.11.29
int request_player_start_stop(PlayType needStart,ListType stop_type,const char *caller)
{
    if (REQUEST_STOP == needStart)
    {        
        debuglog_yellow("request_player_stop caller[%s][%d][%d]停止播放\n",caller,get_set_currentList(GET_LIST),stop_type);
        /*ethan add 2021.11.29*/
        if(stop_type == NLIST_PIC)//停止图片显示
        {
            stopPlayPictureBlock();
            setPictureWidgetVisible(false);
        }
        else if(stop_type == NLIST_VIDEO)//停止视频播放
        {
            stopPlayVideo();
        }
        else/*ethan add 2021.11.29*/
        {//兼容插拔U盘时对播放的操作
            if (NLIST_PIC == get_set_currentList(GET_LIST))
            {
                stopPlayPictureBlock();
                setPictureWidgetVisible(false);
            }
            else
            {
                stopPlayVideo();
            }
        }
        return 0;
    }
    else if (player_task_queue_id.max_depth>0)//通知图片和视频播放线程，开始播放
    {
        char *needCaller =  message_queue_message_alloc_blocking(&player_task_queue_id);
        strncpy(needCaller,caller,256);
        debuglog_yellow("request_player_start caller[%s]\n",caller);
        message_queue_write(&player_task_queue_id, needCaller);
        return 0;
    }
    return -1;
}

/*
*开始播放图片
*播放图片时间使用消息队列等待，需要重启播放器时此消息队列会收到停止消息
*/
void startPlayPicture(char *absolutePath,int interval)
{
    setPictureWidgetVisible(true);
    debuglog_red("setPictureWidgetLoad absolutePath[%s]interval[%d]\n",absolutePath,interval);
    setPictureWidgetLoad(absolutePath); 
    uint8_t *data_byte = message_queue_timeout(&play_pic_queue_id,interval*10);
    if (NULL != data_byte)
    {
        message_queue_message_free(&play_pic_queue_id,data_byte);
        return true;
    }
    setPictureWidgetVisible(false);

	if((GetRotateSetting() == 0) || (GetRotateSetting() == 180))
    {
        if(0 == Get1280x1024SpecialDemand())
        {
            request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
        }
    }
}


/**
 * @brief 列表/参数维护: 负责处理广告/参数组 
 * @caller 启动时/MQTT发起线程请求
 * @param arg 
 */
void *handleConfigAndADThread(void *arg)
{
    message_queue_init(&ad_task_queue_id, 256, 40);
    //重启时先从文件里读取RAM链表
    if(true == JSONListFile2playRAM())
    {
        request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
    }
    while (1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
        printf("[thread]handleConfigAndADThread!\n");
        // printRAMNMediaID("handleConfigAndADThread");
        /*读取到队列请求 则将请求地址给到http处理 再回到控制中心处理*/
        char *url = message_queue_read(&ad_task_queue_id); 
        handleConfigAndAD(url);
        message_queue_message_free(&ad_task_queue_id, url);
    }
}

const char *get_set_current_media(char *media)
{
    static char mediaName[256] = {0};
    if (media!=NULL)
    {
        memset(mediaName,0,sizeof(mediaName));
        strncpy(mediaName,media,256);
    }
    return (const char *)mediaName;
}

/*
*@brief: 负责图片播放线程
*@caller: register_user_thread
*@param arg: none
*@return: void*
*横屏分屏和竖屏分屏时播放图片用
*/
static bool b_PicRunning = false;//避免在播放图片广告时切换到缺省图片
static bool UserPicPalyState = false;
void *handlePlayPicThread(void *arg)
{
	message_queue_init(&play_pic_queue_id, 10, 40);
	while(1)
    {
        printf("5min[%s]5min\n",__FUNCTION__);
		if(getNListCount() > 0)
        {
			playTask *currentTask = getNListNextPlayTaskForPic();
			if(currentTask != NULL)
            {
				get_set_current_media(currentTask->mediaId);
				if(PICTURE_TASK != currentTask->mediaType) continue;//get next task

				char absolutePath[512] =  {0};
                snprintf(absolutePath,512,"%s/%s",USER_NLIST_ROOT,currentTask->mediaId);
                printf("handlePlayPicThread ========================RAM播放图片[%d][%s]\n",currentTask->interval,absolutePath);
				b_PicRunning = true;
                UserPicPalyState = true;
                startPlayPicture(absolutePath,currentTask->interval);
                UserPicPalyState = false;
			}
            else
            {
				printf("[%s][line: %d]Invalid currentTask\n",__FUNCTION__,__LINE__);
				b_PicRunning = false;
				sleep(5);
			}
		}
        else
        {
			b_PicRunning = false;
			sleep(5);
		}
	}
}

/**
 * @brief 负责处理播放器启停
 * @caller 启动时/播放回调/U盘回调
 * @param arg 
 * @attention 采用ifelse避免频繁操作文件系统
 * @return void* 
 * 
 */
void *handleStartPlayerThread(void *arg)
{
    playTask *currentTask = NULL;

    message_queue_init(&player_task_queue_id, 256, 40);

    /*Ethan modify 2021.11.29*/
	if(((GetRotateSetting() == 0) || (GetRotateSetting() == 180)) && (0 == Get1280x1024SpecialDemand()))
    {
        message_queue_init(&play_pic_queue_id, 10, 40);
    }
    
    // UnSleep_S(3);
    // request_player_start_stop(REQUEST_START,__FUNCTION__);
    if (getTypeFilesCount(USER_DEFAULT_ROOT,"def_ad_img",".png")>0)
    {
        /* 进入缺省图片播放列表 */
        get_set_currentList(DEFAULT_PNG);
        printf("handleStartPlayerThread ========================缺省图片播放\n");
        setPictureWidgetVisible(true);
        setPictureWidgetLoad(USER_DEFAULT_ROOT"/def_ad_img.png");
    }

    while (1)
    {
        /*读取到队列请求 读取播放列表目录 判定播放列表类型 播放文件类型*/
        printf("5min[%s]5min\n",__FUNCTION__);
        char *needCaller = message_queue_read(&player_task_queue_id);//开始播放
        printf("[thread]handleStartPlayerThread![%s]\n",needCaller);

        /*每次播放视频前 刷新一下UI设置以及音频声音设置 注意在网络检测线程里也有操作*/
        
        //updateQRcodeWidget();//ethan del 20211201
        //userSetVideoVolumeLevel();//ethan del 20211201 
        message_queue_message_free(&player_task_queue_id, &needCaller);

        const char *UDiskPath = get_set_usb_path(NULL);
        printf("UDiskPath ========================[%s]\n",UDiskPath);
        if(strlen(UDiskPath) > U_DISK_LENGTH)
        {
            /* 进入U盘播放列表 */
            get_set_currentList(ULIST);
            printf("handleStartPlayerThread ========================U盘播放\n");
            //playTask *currentTask = getUListNextPlayTask();//ethan del 2021.11.26
            currentTask = getUListNextPlayTask();
            if (currentTask!=NULL)
            {
                printf("$$$$$$$$$$$$$$$$$$$current player task is [%s][%d]\n", currentTask->mediaId,currentTask->mediaType);
                startPlayVideo(currentTask->mediaId);
                updateOtherConfigs();//add by wyf 20220930 to 视频播放音量调节问题 放在此处响应及时
                continue;
            }else
            {
                printf("current player task is [NULL] will goto def_ad_video!\n");
            }
        }/*end if(strlen(UDiskPath) > U_DISK_LENGTH)*/

        /*网络广告*/
        if (getNListCount()>0)
        {
            /* 进入RAM播放列表 */
            currentTask = getNListNextPlayTask();
            if (currentTask != NULL)
            {
                printf("$$$$$$$$$$$$$$$$$$$current player task is [%s][%d]\n", currentTask->mediaId,currentTask->taskId);
                get_set_current_media(currentTask->mediaId);
                if (PICTURE_TASK == currentTask->mediaType)
                {
					//hoenstar kar.hong add 20210829
					if((GetRotateSetting() == 90) || (GetRotateSetting() == 270) || (1 == Get1280x1024SpecialDemand()))
                    {
						request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
                       	continue;
					}
					//honestar karl.hong add 20210829
					
                    if(true == get_video_isPlaying())
                    {
                        request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
                       continue;
                    }
                    get_set_currentList(NLIST_PIC);
                    char absolutePath[512] =  {0};
                    snprintf(absolutePath,512,"%s/%s",USER_NLIST_ROOT,currentTask->mediaId);
                    printf("handleStartPlayerThread ========================RAM播放图片[%d][%s]\n",currentTask->interval,absolutePath);
                    UserPicPalyState = true;
                    startPlayPicture(absolutePath,currentTask->interval);
                    UserPicPalyState = false;
                }
                else if(VIDEO_TASK == currentTask->mediaType)
                {
                	if((GetRotateSetting() == 0) || (GetRotateSetting() == 180))
                    {
                        if(0 == Get1280x1024SpecialDemand())
                        {
                            setPictureWidgetVisible(false);
                        }
                    }

                    get_set_currentList(NLIST_VIDEO);
                    printf("handleStartPlayerThread ========================RAM播放视频\n");
                    startPlayVideo(currentTask->mediaId);
                    updateOtherConfigs();//add by wyf 20220930 to 视频播放音量调节问题 放在此处响应及时
                }
                continue;
            }
            else
            {
                printf("current player task is [NULL] will goto def_ad_video!\n");
            }

        }/*end if (getNListCount()>0) 网络广告*/
        
        if (getTypeFilesCount(USER_DEFAULT_ROOT,"def_ad_img",".png")>0)
        {
        	//honestar karl.hong add 20210830
        	if(b_PicRunning == true){
				//pic thread is running, exit and get next task
				request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
				continue;
			}
			//honestar karl.hong add 20210830
            /* 进入缺省图片播放列表 */
            get_set_currentList(DEFAULT_PNG);
            printf("handleStartPlayerThread ========================缺省图片播放\n");
            setPictureWidgetVisible(true);
            setPictureWidgetLoad(USER_DEFAULT_ROOT"/def_ad_img.png");
            UnSleep_S(10);
            setPictureWidgetVisible(false);  
            request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
            continue;
        }
        
        /*出错处理 待调试日志完成后使用*/
        get_set_currentList(NO_LIST);
        debuglog_red("handleStartPlayerThread ERROR!!! NO MATCH !\n");
    }
}

/*
*获取用户是否在播放图片
*/
bool GetUserPicPalyState(void)
{
    return UserPicPalyState;
}
