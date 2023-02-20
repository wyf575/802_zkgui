#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

/*
本项目适用的理想的播放器功能
1.播放视频(播放文件路径)
2.播放视频的状态(回调中可以给出参数)
3.视频音量设置
4.停止视频播放
5.播放图片(播放文件路径，播放时长)
6.停止图片播放
7.图片解析及播放线程
*/
void initPlayThread(void);
void startPlayVideo(const char *videoPath);
void userSetVideoVolumeLevel(void);
void stopPlayVideo(void);


#endif // !MEDIA_PLAYER_H

