#ifndef PLAY_CONTROL_H
#define PLAY_CONTROL_H

typedef enum{
    GET_LIST = 0,
    NO_LIST ,
    ULIST ,
    NLIST_PIC ,
    NLIST_VIDEO ,
    DEFAULT_MP4,
    DEFAULT_PNG,
}ListType;

typedef enum{
    REQUEST_START = 0,
    REQUEST_STOP ,
}PlayType;

/**
 * @brief Get the set currentList object
 * 设置和获取当前播放列表类型
 * @param setList 
 * @return int 
 */
int get_set_currentList(int setList);


/**
 * @brief 解析播放列表与配置
 * @caller http拉取播放任务与配置
 * @param rootStr http获取的JSON字串
 */
void playerListAndConfig(char *rootStr);


/**
 * @brief 校验、切换、启停播放
 * 
 * @param UDiskPath U盘目录
 */
void otherMode2UList(char *UDiskPath);

/**
 * @brief 切换和启停播放
 * 
 */
void UList2OtherMode(void);

/**
 * @brief 对外提供发起线程请求的API
 * 
 */
int get_ad_and_config_params(void);

/**
 * @brief 负责处理广告/参数组
 * @caller 启动时/MQTT发起线程请求
 * @param arg 
 */
void *handleConfigAndADThread(void *arg);

/**
 * @brief 向播放策略任务请求启停
 * @attention 目的是让任务能够以队列形式执行
 * @param needStart true:启动 false:停止
 * @return int 
 */
//int request_player_start_stop(PlayType needStart,const char *caller);
int request_player_start_stop(PlayType needStart,ListType stop_type,const char *caller);
/**
 * @brief 负责处理播放器启停
 * @caller 启动时/播放回调/U盘回调
 * @param arg 
 * @attention 采用ifelse避免频繁操作文件系统
 * @return void* 
 */
void *handleStartPlayerThread(void *arg);

/*
*@brief: 负责图片播放线程
*@caller: register_user_thread
*@param arg: none
*@return: void*
*/
void *handlePlayPicThread(void *arg);//honestar karl.hong add 20210829


void stopPlayPicture(void);
bool GetUserPicPalyState(void);//获取用户是否在控制播放图片

#endif // !PLAY_CONTROL_H
