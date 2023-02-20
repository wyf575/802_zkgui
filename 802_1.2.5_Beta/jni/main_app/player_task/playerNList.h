#ifndef PLAYER_NLIST_H
#define PLAYER_NLIST_H

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "beanCList.h"
#include "../cJSON_file/json_app.h"
#include "../main_app.h"

/**
 * @brief 初始化播放列表
 * 
 */
void initPlayerNList(void);


/**
 * @brief 判定网络列表当前视频是否在本地存在
 * 
 * @param filePath 视频文件路径
 * @return true 本地存在
 * @return false 本地不存在
 */
bool isExistLocal(const char *mediaId);


/**
 * @brief 判定当前视频是否过期(或未开始)
 * 过期需要调用者删除 并通知服务器
 * @param dateStart 13位时间戳
 * @param dateEnd 13位时间戳
 * @return true 过期
 * @return false 未过期
 */
bool isExpiredDate(long int dateStart,long int dateEnd);


/**
 * @brief 判定当前视频是否在当天播放时间段
 * 在播放时间段内 则调用播放器播放该视频
 * @param timeStart 时分的0到4位整数(23:59为2359)
 * @param timeEnd 时分的0到4位整数(00:01为1)
 * @return true 处于播放时间段
 * @return false 不处于播放时间段
 */
bool isPlayPeriod(long int dateStart,int timeStart,int timeEnd);


/**
 * @brief 获取整个播放列表中视频总大小KB
 * 注意：蜜连平台下发的大小单位为B
 * @param ppFirst 链表首节点
 * @return long int 返回整个列表中视频大小
 */
long int getListAllSize(void);

/**
 * @brief 判定添加当前视频后是否超出总内存限制
 * 超出总内存 则拒绝下载该视频 并回复服务器
 * @param mediaSize 即将添加的广告大小
 * @param dataRoot 广告所在根目录
 * @return true 
 * @return false 
 */
bool isOverflowFree(long int mediaSize,const char *dataRoot);


/**
 * @brief 根据任务id查找该任务
 * 
 * @param taskID 任务ID
 * @return playTask* 返回任务地址
 */
playTask *findByTaskID(int taskID);


/**
 * @brief 新增一个任务到播放列表
 * 
 * @param task 新增的任务
 */
void addPlayTask(playTask task); 


/**
 * @brief 从任务列表删除一个指定任务
 * 
 * @param taskID 需要删除的任务ID
 */
void delPlayTaskByID(int taskID);


/**
 * @brief Get the Next Task object
 * 根据当前任务ID获取下一个任务地址
 * @param taskID 当前任务ID
 * @return playTask* 下一个任务地址
 */
playTask *getNextTask(int taskID);

/**
 * @brief 删除本地无效的文件
 * @attention 无效即RAM中已经被剔除而本地依然存在
 */
bool deleteInvalidFiles(void);

/**
 * @brief 解析网络播放任务
 * @caller 控制中心
 * @attention 负责将原始数据转换成RAM链表
 * @attention 负责将RAM链表转换成JSON组以及文件
 * @return true 需要停启播放器避免文件删除出错
 * @return false 不需要
 */
bool parseNListparams(cJSON *playList);

/**
 * @brief 单个任务添加到RAM链表
 * @caller parseNListparams
 * id号检查 本地存在 内存余量
 * @param item 单任务JSON
 * @caller 网络下发任务/启动时读取本地JSON
 * @attention 能下发的JSON必然未过期
 * @attention 可能触发下载视频和图片的操作
 * @return 0: 数据有效 其他：数据无效
 */
int playItem2RAMList(cJSON *item);


/**
 * @brief RAM链表转JSON数组和JSON文件
 * @caller 过期删除任务后/parseNListparams
 */
void playRAM2JSONListAndFile(void);

/**
 * @brief JSON组文件转换为RAM链表
 * @caller 设备启动 
 * @attention 保证无网络的情况下能播放任务
 */
bool JSONListFile2playRAM(void);

/**
 * @brief 上报广告任务处理结果
 * 
 */
void reportADResult(void);

/**
 * @brief 本地RAM去同步网络列表
 * @attention 删除本地文件前请先停止播放
 * @attention 删除操作再控制中心那层
 * @return true 需要删除本地文件
 * @return false 不需要
 */
bool RAMListSyncItem(cJSON *playList);

/**
 * @brief 获取Nlist列表中的长度
 * 
 * @return int 
 */
int getNListCount(void);

/**
 * @brief 给getNListNextPlayTask打补丁
 * @attention 
 */
void resetNFirstCall(void);

/**
 * @brief 获取下一个即将播放的广告
 * @caller 供控制中心调用
 * @return const char* 
 */
const playTask *getNListNextPlayTask();
const playTask *getNListNextPlayTaskForPic();


/**
 * @brief 给外部提供打印mediaID接口
 * 
 * @param caller 外部调用者
 */
void printRAMNMediaID(char *caller);

/**
 * @brief 给外部提供打印taskID接口
 * 
 * @param caller 外部调用者
 */
void printRAMNTaskID(char *caller);

/**
 * @brief 给外部提供打印taskName接口
 * @attention 方便与平台对照查看
 * @param caller 外部调用者
 */
void printRAMNTaskName(char *caller);

#endif // !PLAYER_LIST_H
