#pragma once
#include "uart/ProtocolSender.h"
/*
*此文件由GUI工具生成
*文件功能：用于处理用户的逻辑相应代码
*功能说明：
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数，XXX代表GUI工具里面的[ID值]名称，
如Button1,当返回值为false的时候系统将不再处理这个按键，返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index) 
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress) 
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX() 
当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[ID值]名称，
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[ID值]名称，
如List1;pListItem 是贴图中的单条目对象，index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mTextXXXPtr->setText("****") 在控件TextXXX上显示文字****
*mButton1Ptr->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBarPtr->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1Ptr->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1Ptr->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/

#include "model/SNManager.h"
#include "storage/StoragePreferences.h"
#include "model/MediaManager.h"
#include "model/ModelManager.h"
#include "baselib/UrlManager.h"
#include "model/MediaPlayer.h"
#include "net/NetManager.h"
#include "net/DeviceSigManager.h"

#include "appconfig.h"
#include "main_app/main_app.h"


#if FANG_PING_1280x1024
	#define VERTICAL_SCREEN_WIDTH 1024//竖屏
	#define VERTICAL_SCREEN_HEIGHT 1280

	#define HORIZONTAL_SCREEN_WIDTH 1280//横屏
	#define HORIZONTAL_SCREEN_HEIGHT 1024
#else
	#define VERTICAL_SCREEN_WIDTH 480//竖屏
	#define VERTICAL_SCREEN_HEIGHT 800

	#define HORIZONTAL_SCREEN_WIDTH 800//横屏
	#define HORIZONTAL_SCREEN_HEIGHT 480
#endif

#if FANG_PING_1280x1024_SPECIAL_DEMAND
	#define HORIZONTAL_SCREEN_HEIGHT_SPECIAL 720//保证方屏视频播放区域16：9
#endif

#define printf LOGD
#define myPrintf printf

#define CHECK_TRUE(x) {if(false==x){printf("please init UI first!");return -1;}}

// static long downTime = 0;
// static bool isUpgradeSig = false;
// void onNetChange1366x768(char* netType){
// 	LOGD("___________onNetChange-----------\n");
// 	isUpgradeSig = true;
// }
static bool pageStarted = false;
volatile bool isPlaying = false;

bool get_playerSelect_status(void)
{
	return pageStarted;
}
/**
 * @brief Set the version widget object
 * 本界面动态控制版本显示
 * @param version 
 */
int set_version_widget(char *version,char *sn)
{
	CHECK_TRUE(get_playerSelect_status());
	char showstr[50] = {0};
	snprintf(showstr,50,"%s-%s",version,sn);
	printf("set_version_widget................\n");
	mTVVersionPtr->setText(showstr);
	return 0;
}

int set_version_visible(bool visible)
{
	CHECK_TRUE(get_playerSelect_status());
	printf("set_version_visible................\n");
	mTVVersionPtr->setVisible(visible);
	return 0;
}

/**
 * 优化只有网络状态改变或信号质量改变后才刷新控件
 * Ethan 20211209
*/
// void DeviceSigManager::updateSigUI(ZKTextView* sigView);
int set_signal_widget(networkType type,int level)
{
	/*ethan add 20211209*/
	static int old_level = 0;
	static networkType old_type = NETWORKTYPE_2G;
	/*ethan add end 20211209*/
	CHECK_TRUE(get_playerSelect_status());
	printf("set_signal_widget[%d][%d]",type,level);
	char bgPicPath[100] = {0};
	switch (type)
	{
		case NETWORKTYPE_ETHERNET:
			snprintf(bgPicPath,100,"%s","sig/stat_sys_eth_connected.png");
		break;
		case NETWORKTYPE_4G:
		case NETWORKTYPE_WIFI:
			switch (level)
			{
			case 0:
				snprintf(bgPicPath,100,"%s","sig/no_sig.png");
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				if (NETWORKTYPE_4G == type)
				{
					snprintf(bgPicPath,100,"sig/sig_4g_%d.png",level);
				}else
				{
					snprintf(bgPicPath,100,"sig/sig_wifi_%d.png",level);
				}
				break;
			default:
				break;
			}
		break;
		case NETWORKTYPE_NONE:
	default:
		snprintf(bgPicPath,100,"%s","sig/no_sig.png");
		break;
	}
	if((old_type != type) || (old_level != level))
	{
		printf("setBackgroundPic[%s]\n",bgPicPath);
		mTV_sigPtr->setBackgroundPic(bgPicPath);
	}

	/**ethan del 20211209
	if (strlen(bgPicPath)>0)
	{
		printf("setBackgroundPic[%s]\n",bgPicPath);
		mTV_sigPtr->setBackgroundPic(bgPicPath);
	}
	*/
	return 0;
}
// void ModelManager::parseParamsTask(ZKQRCode* zkQr);
int set_QRcode_widget_str(const char *pStr)
{  
	CHECK_TRUE(get_playerSelect_status());
	printf("set_QRcode_widget_size[设置二维码内容][%s]\n",pStr);
	if (strlen(pStr)>0)
	{
		mQRCode1Ptr->loadQRCode(pStr);
	}
	return 0;
}

int set_QRcode_widget_size(int qrSizeInt)
{  
	CHECK_TRUE(get_playerSelect_status());
	printf("set_QRcode_widget_size[设置二维码大小][%d]\n",qrSizeInt);
	LayoutPosition position = mQRCode1Ptr->getPosition();
	position.mLeft = position.mLeft - (qrSizeInt - position.mWidth);
	position.mHeight = qrSizeInt;
	position.mWidth = qrSizeInt;
	mQRCode1Ptr->setPosition(position);
	return 0;
}

int set_QRcode_widget_visible(bool visible)
{  
	CHECK_TRUE(get_playerSelect_status());
	printf("set_QRcode_widget_size[设置二维码显示/隐藏][%d]\n",visible);
	mQRCode1Ptr->setVisible(visible);
	return 0;
}

int set_pic_widget_load(char *picPath)
{
	CHECK_TRUE(get_playerSelect_status());
	mTextView_picPtr->setBackgroundPic(picPath);
	return 0;
}

int set_pic_widget_visible(bool visible)
{  
	CHECK_TRUE(get_playerSelect_status());
	printf("set_pic_widget_visible[设置图片显示/隐藏][%d]\n",visible);
	mTextView_picPtr->setVisible(visible);
	return 0;
}

//////////////////////////////////////////////////视频播放器部分////////////////////////////////////////////////////
/*
*视频播放器初始化
*/
static bool init_player_created = false;
int init_player_thread(user_player_callback function_callback)
{
	CHECK_TRUE(get_playerSelect_status());
	if (false == init_player_created)
	{
		//MEDIAMANAGER->initPlayList();
		MEDIAPLAYER->initPlayer(mVideoview_videoPtr, mTextView_picPtr);
		MEDIAPLAYER->showPlayer(false,function_callback);
		printf("init_player_thread[视频线程启动完成]\n");
		init_player_created = true;
	}
	return 0;
}

/*
*视频播放器开始播放
*/
int start_video_play(char *videoPath,carouselType carousel)
{
	if (NULL == videoPath)
	{
		return -1;
	}
	CHECK_TRUE(init_player_created);
	if(true == get_video_isPlaying())
	{
		user_stop_play_video();
	}
	set_video_widget_visible(true);
	user_start_play_video(videoPath,carousel);
	isPlaying = true;
	return 0;
}

/*
*视频播放器停止播放
*/
int stop_video_play(void)
{
	CHECK_TRUE(init_player_created);
	if(true == get_video_isPlaying())
	{
		user_stop_play_video();
	}
	isPlaying = false;
	set_video_widget_visible(false);
	return 0;
}

/*
*设置视频播放器控件显示或隐藏
*/
int set_video_widget_visible(bool visible)
{  
	CHECK_TRUE(get_playerSelect_status());
	printf("set_video_widget_visible[设置视频显示/隐藏][%d]\n",visible);
	mVideoview_videoPtr->setVisible(visible);
	return 0;
}

/*
*设置视频播放器音量
*/
int set_video_vol_level(int level)
{
	CHECK_TRUE(get_playerSelect_status());
	printf("set_video_vol_level[设置视频音量等级][%d]\n",level);
	MEDIAPLAYER->setPlayerVol(level);
	return 0;
}

/*
*获取视频播放器是否在运行
*/
bool get_video_isPlaying(void)
{
	return isPlaying;
}

/*
*获取flyting层视频控件是否显示中
*/
bool GetVideoWidgetState(void)
{
	return mVideoview_videoPtr->isVisible();
}

/*
*获取图片是否在显示中
*/
bool GetPicisVisible(void)
{
	return mTextView_picPtr->isVisible();
}

//honeatar karl.hong add 20210826
static void set_wind_rotate_0_180(void)
{
	LayoutPosition position;

	position.mLeft = 0;
	position.mTop = 0;
	position.mWidth = HORIZONTAL_SCREEN_WIDTH;
	position.mHeight = HORIZONTAL_SCREEN_HEIGHT;
	mActivityPtr->setPosition(position);

#if FANG_PING_1280x1024_SPECIAL_DEMAND
	position.mHeight = position.mWidth*9/16;
	mVideoview_videoPtr->setPosition(position);

	position.mTop = position.mHeight;
	position.mHeight = HORIZONTAL_SCREEN_HEIGHT - position.mHeight;
	mTextView_picPtr->setPosition(position);
#else
	position.mHeight = HORIZONTAL_SCREEN_HEIGHT;
	mVideoview_videoPtr->setPosition(position);
	mTextView_picPtr->setPosition(position);
#endif

	mTextView_picPtr->setVisible(false);

	position.mLeft = 15;//15
	position.mTop = 1;//15
	position.mWidth =200;//120
	position.mHeight = 26;//22
	mTVVersionPtr->setPosition(position);

#if FANG_PING_1280x1024
	position.mLeft = HORIZONTAL_SCREEN_WIDTH - 70;
#else
	position.mLeft = HORIZONTAL_SCREEN_WIDTH - 70;//720
#endif
	position.mTop = 5;//20
	position.mWidth =60;
	position.mHeight = 60;
	mQRCode1Ptr->setPosition(position);
	mQRCode1Ptr->setVisible(false);

	position.mLeft = 15;//25
	position.mTop = 26;//44
	position.mWidth =24;
	position.mHeight = 24;
	mTV_sigPtr->setPosition(position);
}

static void set_wind_rotate_90_270(void)
{
	LayoutPosition position;
	
	position.mLeft = 0;
	position.mTop = 0;
	position.mWidth = VERTICAL_SCREEN_WIDTH;
	position.mHeight = VERTICAL_SCREEN_HEIGHT;
	mActivityPtr->setPosition(position);

	position.mWidth = VERTICAL_SCREEN_WIDTH;//480x270
	position.mHeight = position.mWidth*9/16;
	mVideoview_videoPtr->setPosition(position);

	position.mTop = position.mHeight;
	position.mHeight = VERTICAL_SCREEN_HEIGHT - position.mHeight;
	mTextView_picPtr->setPosition(position);
	mTextView_picPtr->setVisible(false);

	position.mLeft = 15;
	position.mTop = 1;
	position.mWidth = 200;
	position.mHeight = 26;
	mTVVersionPtr->setPosition(position);

	position.mLeft = VERTICAL_SCREEN_WIDTH - 70;
	position.mTop = 5;
	position.mWidth =60;
	position.mHeight = 60;
	mQRCode1Ptr->setPosition(position);
	mQRCode1Ptr->setVisible(false);

	position.mLeft = 15;
	position.mTop = 26;
	position.mWidth =24;
	position.mHeight = 24;
	mTV_sigPtr->setPosition(position);
}

static void Adjust_Wind_Position(void)
{
	switch(GetRotateSetting()){
		case 0:
		case 180:
			set_wind_rotate_0_180();
			break;
		case 90:
		case 270:
			set_wind_rotate_90_270();
			break;
		default:break;
	}
}
//honestar karl.hong add 20210826
/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
	// {0,  60000},//参数配置
	// {1,  6000},
};

/**
 * 当界面构造时触发
 */
static void onUI_init(){
	DEBUG_INFO();
	Adjust_Wind_Position();
	mTV_sigPtr->setBackgroundPic("sig/no_sig.png");
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
	// initNetManager(onNetChange1366x768);
    // //MEDIAPLAYER->initPlayer(mVideoview_videoPtr, mTextView_picPtr);


}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
	DEBUG_INFO();

    // if (intentPtr != NULL) {
    //     //TODO
    // }
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {
	DEBUG_INFO();
	pageStarted = true;
	// std::string version = APP_VERSION;
	// mTVVersionPtr->setText(version + "-sstar");
	// SNMANAGER->initUpdateParam(mQRCode1Ptr);
	// MODELMANAGER->parseParamsTask(mQRCode1Ptr);
	// //MEDIAPLAYER->showPlayer(false);
}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {
	DEBUG_INFO();
	
	// //MEDIAPLAYER->quitPlayer();
}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
	DEBUG_INFO();
	MEDIAPLAYER->quitPlayer();
	pageStarted = false;
}

/**
 * 串口数据回调接口
 */
static void onProtocolDataUpdate(const SProtocolData &data) {
	DEBUG_INFO();
}

/**
 * 定时器触发函数
 * 不建议在此函数中写耗时操作，否则将影响UI刷新
 * 参数： id
 *         当前所触发定时器的id，与注册时的id相同
 * 返回值: true
 *             继续运行当前定时器
 *         false
 *             停止运行当前定时器
 */
static bool onUI_Timer(int id){
	DEBUG_INFO();
	// switch (id) {
	// case 0://参数配置
	// 	MODELMANAGER->parseParamsTask(mQRCode1Ptr);
	// 	break;
	// case 1:
	// 	if(isUpgradeSig){
	// 		isUpgradeSig = false;
	// 		DEVICESIGMANAGER->updateSigUI(mTV_sigPtr);
	// 	}
	// 	break;
	// 	default:
	// 		break;
	// }
    return true;
}

/**
 * 有新的触摸事件时触发
 * 参数：ev
 *         新的触摸事件
 * 返回值：true
 *            表示该触摸事件在此被拦截，系统不再将此触摸事件传递到控件上
 *         false
 *            触摸事件将继续传递到控件上
 */
static bool onplayerSelectActivityTouchEvent(const MotionEvent &ev) {
	DEBUG_INFO();
    switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			break;
		default:
			break;
	}
	return false;
}

