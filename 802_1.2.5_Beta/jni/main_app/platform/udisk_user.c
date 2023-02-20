#include "udisk_user.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../linux_tool/busy_box.h"
#include "../player_task/playControl.h"
#include "hotplugdetect/hotplugdetect.h"

/**
 * @brief 在回调里设置路径
 * 注意：在U盘拔出后一定要同步设置为""
 * 
 */
const char *get_set_usb_path(char *setPath)
{
    static char usbPath[256] = {0};
    if (NULL != setPath)
    {
        memset(usbPath,0,sizeof(usbPath));
        strncpy(usbPath,setPath,256);
    }
    return (const char *)usbPath;
}

/**
 * @brief U盘插拔都会进入这个回调
 * 前提是运行了U盘检测线程
 * @attention 插入U盘可能产生多次回调
 * @param pstUsbParam 
 */
static void usb_action_callback(UsbParam_t *pstUsbParam)		// action 0, connect; action 1, disconnect
{
	static int oldPlugStatus = 0;
    printf("oldPlugStatus[%d] action[%d] udisk_path[%s]\n", oldPlugStatus,pstUsbParam->action, pstUsbParam->udisk_path);
    if (oldPlugStatus==pstUsbParam->action)
    {
        if(1 == pstUsbParam->action && (strlen(pstUsbParam->udisk_path)>U_DISK_LENGTH)){
            /* /vendor/udisk_sda1 usbhotplug.c中写死 最后的数字动态变化 */
            /*TODO:这里需要调用playControl中对播放列表的有效检查、列表切换、启停播放等操作*/
            otherMode2UList(pstUsbParam->udisk_path);
        }
    }else if (0 == pstUsbParam->action)
    {
        /*TODO:这里需要调用playControl中对播放列表切换、启停播放等操作*/
        UList2OtherMode();
    }        
    oldPlugStatus = pstUsbParam->action;
}


/**
 * @brief 初始化U盘配置
 * 解决启动前插U盘的问题
 * 绑定应用回调函数
 */
void initUDiskConfig(void)
{
    /*绑定系统监测的回调*/
    SSTAR_InitHotplugDetect();
    SSTAR_RegisterUsbListener(usb_action_callback);
    /*解决启动前客户已经插入U盘无回调的问题*/
    char output[128] = {0};
    get_system_output("ls /vendor",output,64);
    printf("cmd[%s]output[%s]\n","ls /vendor",output);
    if (strlen(output)<6)
    {
        memset(output,0,sizeof(output));
        get_system_output("ls /dev/sda1",output,64);
        if (strlen(output)>8)
        {
            call_system_cmd("mkdir -p /vendor/udisk_sda1");
            printf("cmd[%s]\n","mkdir -p /vendor/udisk_sda1");
            call_system_cmd("mount -t auto /dev/sda1 /vendor/udisk_sda1");
            printf("cmd[%s]\n","mount -t auto /dev/sda1 /vendor/udisk_sda1");
            call_system_cmd("sync");
        }
    }
}






