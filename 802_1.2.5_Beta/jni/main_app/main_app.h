#ifndef MAIN_APP_H
#define MAIN_APP_H

/**
 * @file main_app.h
 * @author aiden.xiang@ibeelink.com
 * @brief 业务逻辑总入口
 * @version 0.1
 * @date 2021-04-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/stat.h>
/*
- T.X.X.X ：18.5寸屏参1366x768-刀头设备USER_APP_MASK
- S.X.X.X ：18.5寸屏参1366x768-407SS设备USER_APP_CARGO
- M.X.X.X ：15.4寸屏参1280x800-刀头设备USER_APP_MASK
- N.X.X.X ：15.4寸屏参1280x800-407SS设备USER_APP_CARGO
- P.X.X.X ：19.0寸屏参1440x900-刀头设备USER_APP_MASK
- R.X.X.X ：19.0寸屏参1440x900-407SS设备USER_APP_CARGO
- P.3.X.X ：19.0寸屏参1440x900-竖屏-刀头设备USER_APP_MASK
- R.3.X.X ：19.0寸屏参1440x900-竖屏-407SS设备USER_APP_CARGO
- Q.X.X.X ：21.5寸屏参1920x1080-刀头设备USER_APP_MASK
- U.X.X.X ：方屏1280x1024-刀头设备USER_APP_MASK
- V.X.X.X ：方屏1280x1024-407SS设备USER_APP_CARGO
- U.3.X.X ：方屏1280x1024-竖屏-刀头设备USER_APP_MASK
- V.3.X.X ：方屏1280x1024-竖屏-407SS设备USER_APP_CARGO
- U.6.X.X ：方屏1280x1024-横屏分屏-刀头设备USER_APP_MASK
- V.6.X.X ：方屏1280x1024-横屏分屏-407SS设备USER_APP_CARGO
- W.X.X.X ：19.0寸屏参1440x900-新厂家刀头设备USER_APP_NEW_MASK
- K.X.X.X ：10.1寸屏参1024-576-新厂家刀头设备USER_APP_NEW2_MASK
*/
#define USER_APP_CARGO 0
#define USER_APP_MASK 0//鼎基刀头
#define USER_APP_NEW_MASK 0//切刀刀头
#define USER_APP_NEW2_MASK 1//打印机式口罩机头
#define USER_APP_TYPE USER_APP_NEW2_MASK

/*
*适配方屏,1280x1024
*/
#define FANG_PING_1280x1024 0//方屏未适配拉伸，所以方屏需要按原分辨率进行使用
#define FANG_PING_1280x1024_SPECIAL_DEMAND 0//方屏特殊要求，横放状态下以16：9播放视频，最下面播放图片

//注：使用1280x1024屏幕时，需要修改宏FANG_PING_1280x1024为1
//选择不同屏幕时需要确认响应屏幕fbdev.ini中分辨率是否为800*480；
//修改EasyUI.cfg中rotateScreen选择屏幕方向,90竖屏
//修改panel.ini中，m_targetPanel，选择屏参

#define MAIN_APP_VERSION "K.1.3.2"


//不可更改 用于ota脚本升级时读取版本号使用
#define APP_VERSION_KEY "app_version"
#define APP_VERSION_FILE "/data/version.txt"
#define USER_NLIST_ROOT "/data/nlist"
#define USER_DEFAULT_ROOT "/customer/res/ui"
#define PLAY_LIST_FILE "/customer/PLAYLIST.json"
#define PLAY_BUFF_FILE "/customer/PLAYBUFF.json"
#define CONFIG_FILE_NAME "/customer/CONFIG.json"

/**
 * @brief 对外提供的接口
 */
uint8_t Get1280x1024SpecialDemand(void);
void *app_main_thread(void *arg);
int register_user_thread(void *p_thread, void *function_callback, void *param);
void unregister_user_thread(void);

/**
 * @brief 对内提供的接口
 * 
 */

#include "entry/EasyUIContext.h"
#include "utils/Log.h"
#define printf LOGD
#define UnSleep_MS(x) usleep(x*1000);
#define UnSleep_S(x) sleep(x);

#endif // !MAIN_APP_H

