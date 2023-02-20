#include <sys/time.h>
#include "widgetUI.h"
#include "../main_app.h"
#include "appconfig.h"
#include "../player_task/configPlayer.h"
#include "../net_listener.h"
#include "../debug_printf/debug_printf.h"
#include "../model/MediaPlayer.h"

extern const char *get_popularize_str(void);
extern const char *UnGetDeviceSN(void);

/*
*设置视频控件是否显示
*/
void setVideoWidgetVisible(bool enabled)   
{
    set_video_widget_visible(enabled);
}

/*
*设置图片控件是否显示
*/
void setPictureWidgetVisible(bool enabled)   
{
    set_pic_widget_visible(enabled);
}

/*
*设置图片控件加载图片地址
*/
void setPictureWidgetLoad(char *picPath)   
{
    set_pic_widget_load(picPath);
}


void setQRcodeWidgetVisible(bool enabled)   
{
    set_QRcode_widget_visible(enabled);
}


void setQRcodeWidgetLoad(char *pStr)   
{
    set_QRcode_widget_str(pStr);
}
//没用到？
void setQRcodeWidgetSize(int qrSizeInt)   
{
    set_QRcode_widget_size(qrSizeInt);
}

void setSignalWidgetLoad(networkType type,int level)   
{
    set_signal_widget(type,level);
}

void setVersionWidgetVisible(bool enabled)   
{
    set_version_visible(enabled);
}
//void setVersionWidgetLoad(char *version,char *sn)//2021.10.19 ethan delete
void setVersionWidgetLoad(char *version,char *sn)//2021.10.19 ethan add
{
    set_version_widget(version,sn);
}


/*
2021.9.28修改在联网与否的情况下都显示二维码
*/
void updateQRcodeWidget(void)
{
    configParam param =  getConfigParams(); 
    //if (NET_STATUS_OK == get_app_network_status() && NET_STATUS_OK ==get_network_status())
    if(NET_STATUS_OK == get_app_network_status())
    {
        if (1 == param.QRCodeDisplay)
        {
            //显示自定义二维码
            set_QRcode_widget_str(param.qr_link);
        }else if(2 == param.QRCodeDisplay)
        {
            //显示客户定制二维码+SN
            const char *netSN = UnGetDeviceSN();
            if (NULL != netSN)
            {
                char tmp_str[262] = {0};
                snprintf(tmp_str,262,"%s%s",param.qr_link,netSN);
                set_QRcode_widget_str(tmp_str);
            }
        }else{
            //显示推广二维码
            const char *tmp_str = get_popularize_str();
            if (strlen(tmp_str)>6)
            {
                set_QRcode_widget_str(tmp_str);
            }
        }
        set_QRcode_widget_size(param.qr_size);
        set_QRcode_widget_visible(true);
    }
    //else
    //
    //    set_QRcode_widget_visible(false);
    //
}


void updateOtherConfigs(void)
{
    struct timeval tv1;
    struct timeval tv2;
    gettimeofday(&tv1, NULL);

    configParam param =  getConfigParams();

    gettimeofday(&tv2, NULL);
    printf("@@@@@@@@@@@@@@@@getConfigParams:%ld;\n", (tv2.tv_sec-tv1.tv_sec)*1000000+tv2.tv_usec-tv1.tv_usec);
    SSTAR_setVolume(param.vol);
    debuglog_yellow("app_network[%d]qr_link[%s]id[%d]qr_size[%d]vol[%d]QRCodeDisplay[%d] \n",get_app_network_status(),param.qr_link,param.id,param.qr_size,param.vol,param.QRCodeDisplay);
}

