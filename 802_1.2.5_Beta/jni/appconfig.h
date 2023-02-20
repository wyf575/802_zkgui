/*
 * appconfig.h
 *
 *  Created on: 2019年10月22日
 *      Author: koda.xu
 */

#ifndef JNI_APPCONFIG_H_
#define JNI_APPCONFIG_H_

#define USE_PANEL_1024_600		0
#define USE_AMIC				1
#define ENABLE_BF				0
//旋转视频
#define ENABLE_ROTATE			0
#define ENABLE_ROTATE_180		0

#define DSPOTTER_DATA_PATH			"/customer/res/DSpotter/data"

#define CREADER_SAVE_FILE			0
#define CREADER_LIB_PATH			"/customer/lib"
#define CREADER_DATA_PATH			"/customer/res/CReader/data"

// #define NETWORKTYPE_NONE 			0
// #define NETWORKTYPE_2G 				2
// #define NETWORKTYPE_4G 				4
// #define NETWORKTYPE_WIFI 			5
// #define NETWORKTYPE_ETHERNET 		6

//#define BA407SS_SERIAL			//BA407ss
#define SQ800_SERIAL				//刀头
//#define SING7_SERIAL 				//大可马电机


#ifdef SING7_SERIAL
#define IS_OPEN_READ_THREAD
#endif

#define UI_DEF_LANDSCAPE 				0		//横屏
#define UI_PORTRAIT_DOUBLE_SCREEN 		1		//竖屏双屏
//配置UI类型
#define UI_TYPE  UI_DEF_LANDSCAPE

//版本号
#define APP_VERSION				"V1.0.7"
//是否是测试环境
#define IS_TEST_MODE     		0
//是否打开工厂测试
#define IS_FACTORY_MOD 			0


//////////////////////////////////////////////////////////////////////
#define DEBUG_INFO() LOGD("[DEBUG_INFO][%s %s] %s: %s: %d\n",__DATE__, __TIME__, __FILE__, __func__, __LINE__)
typedef enum {
    NETWORKTYPE_NONE        = 0 ,
    NETWORKTYPE_2G          = 2 ,
    NETWORKTYPE_4G          = 4 ,
    NETWORKTYPE_WIFI        = 5 ,
    NETWORKTYPE_ETHERNET    = 6 ,
} networkType;



/**
 * @brief player1666*768界面提供的方法
 * 
 * @param version 
 * @return int 
 */
int set_version_widget(char *version,char *sn);
int set_version_visible(bool visible);
int set_signal_widget(networkType type,int level);
int set_QRcode_widget_str(const char *pStr);
int set_QRcode_widget_size(int qrSizeInt);
int set_QRcode_widget_visible(bool visible);
typedef enum MyEnum
{
	CAROUSEL_DISABLE = 0,
	CAROUSEL_ENABLE = 1,
}carouselType;

int start_video_play(char *videoPath,carouselType carousel);
int stop_video_play(void);
bool get_video_isPlaying(void);
bool GetVideoWidgetState(void);//获取flyting层视频是否在播放
bool GetPicisVisible(void);//获取图片是否咋显示中
int set_video_widget_visible(bool visible);
int set_video_vol_level(int level);
int set_pic_widget_load(char *picPath);
int set_pic_widget_visible(bool visible);
//honestar karl.hong add 20210826
void GetPanelSetting(void);
int GetRotateSetting(void);
int GetPanelWidth(void);
int GetPanelHeigth(void);
int GetFbWidth(void);
int GetFbHeight(void);
//honestar karl.hong add 20210826


///////////////////////////////////////////////////////////////////////






#endif /* JNI_APPCONFIG_H_ */
