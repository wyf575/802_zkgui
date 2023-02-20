#ifndef PLAYER_ULIST_H
#define PLAYER_ULIST_H

#include <stdio.h>
#include <stdbool.h>
#include "beanCList.h"

/**
 * @brief 初始化U盘播放列表
 * 
 */
void initPlayerUList(void);

/**
 * @brief 新增一个任务到播放列表
 * 
 * @param task 新增的任务
 */
void addPlayUTaskByMediaID(const char *mediaId);


/**
 * @brief 从任务列表删除一个指定任务
 * 
 * @param mediaId 需要删除的U盘文件名称(唯一)
 */
void delPlayUTaskByMediaId(const char *mediaId);


/**
 * @brief Get the Next Task object
 * 根据当前文件名称获取下一个任务地址
 * @param mediaId 需要删除的U盘文件名称(唯一)
 * @return playTask* 下一个任务地址
 */
playTask *getNextUTaskByMediaId(const char *mediaId);


/**
 * @brief 扫描U盘目录下的文件
 * 每次插拔时调用
 * @param dirPath U盘路径
 * @return int 文件个数
 */
int scanUDiskFile(char *dirPath);

/**
 * @brief 给外部提供打印mediaID接口
 * 
 * @param caller 外部调用者
 */
void printRAMUMediaID(char *caller);


/**
 * @brief 给外部提供打印taskID接口
 * 
 * @param caller 外部调用者
 */
void printRAMUTaskID(char *caller);


/**
 * @brief 获取下一个即将播放的广告
 * @caller 供控制中心调用
 * @return const char* 
 */
const playTask *getUListNextPlayTask(void);

#endif
