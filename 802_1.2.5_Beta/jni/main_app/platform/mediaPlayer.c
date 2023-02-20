#include "../main_app.h"
#include "widgetUI.h"
#include "../hardware_tool/user_gpio.h"
#include "../player_task/playControl.h"
#include "../player_task/configPlayer.h"
#include "mediaPlayer.h"
#include "appconfig.h"
#include "model/MediaPlayer.h"
#include "../message_queue/message_queue.h"


/*
*接收播放进程的反馈？？
*/
int userVideoCompleteCallback(int type,char *tips,int status)
{
    if (NEED_CONTINUE == type)
    {
        printf("==========tips[%s]status[%d]type[%d]\n",tips,status,type);
        audio_close_control();
        //UnSleep_MS(500);//ethan del 2021.12.01
        stop_video_play();
        request_player_start_stop(REQUEST_START,NO_LIST,__FUNCTION__);
    }else
    {
        if (6 != status && 17 != status && 7 != status)
        {
            printf("tips[%s]status[%d]type[%d]\n",tips,status,type);
        }
    }
    return 0;
}

/**
 * @brief 初始化播放器线程
 * @attention 阻塞接口
 */
void initPlayThread(void)
{
    /*加载音频控制*/
    init_audio_control();

    /*加载播放器*/
    while (init_player_thread(userVideoCompleteCallback))
    {
        printf("init_player_thread failed!\n");
        UnSleep_S(1);
    }
}

void startPlayVideo(const char *videoPath)   
{
    //setVideoWidgetVisible(true);//ethan del 20211201
    start_video_play(videoPath,CAROUSEL_DISABLE);
}

void userSetVideoVolumeLevel(void)
{
    configParam param =  getConfigParams(); 
    set_video_vol_level(param.vol);
}

void stopPlayVideo(void)   
{
    stop_video_play();
}

void stopPlayPicture(void)   
{
    setPictureWidgetVisible(false);
}



