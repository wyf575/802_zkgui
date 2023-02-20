#pragma once
#include "../net/Module4GManager.h"
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

#include "model/JsonParse.h"
#include "model/ModelManager.h"
#include "model/MediaManager.h"
#include "net/DeviceSigManager.h"
#include "include/os/UpgradeMonitor.h"
#include "uart/ba407ss/BA407SSParser.h"
#include "baselib/FileUtils.h"
#include "baselib/DeviceManager.h"
#include "baselib/WatchdogManager.h"
#include "hotplugdetect.h"
#include "nz3802/mifare.h"
#include "nz3802/nz3802.h"
#include "panelconfig.h"
#include "model/SNManager.h"
#include "mouse/mouse.h"
#include "storage/StoragePreferences.h"


#include "appconfig.h"
#define printf LOGD



// //配置鼠标
//extern int PanelWidthValue;
//extern int PanelHeightValue;
//extern int mRotateScreenValue;

// static long downTime = 0;

// void ShowWiredNetworkStatus(unsigned int index, int status, char *pstIfName)
// {
// 	if (status)
// 		LOGD("网络已连接\n");
// 	else
// 		LOGD("网络已断开\n");
// }

// void ShowUsbStatus(UsbParam_t *pstUsbParam)		// action 0, connect; action 1, disconnect
// {
// 	LOGD("ShowUsbStatus=%d, %s\n", pstUsbParam->action, pstUsbParam->udisk_path);
// 	if(pstUsbParam->action){
// 		sleep(5);
// 		MEDIAMANAGER->scan_dir_and_savePath(pstUsbParam->udisk_path);
// 	}else{
// 		MEDIAMANAGER->initPlayList();
// 	}
// }

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
//	{0,  5000}, //定时器id=0, 时间间隔5秒
//	{1,  1800000},//间隔30分钟
};

/**
 * 当界面构造时触发
 */
static void onUI_init(){
	DEBUG_INFO();
	EASYUICONTEXT->openActivity("playerSelectActivity");
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
//	initMouseDev();
//	WATCHDOGMANAGER->openWatchdog();

//	MODULE4GMANAGER->init();
//
//	SSTAR_InitHotplugDetect();
////	SSTAR_RegisterWiredNetworkListener(ShowWiredNetworkStatus);
//	SSTAR_RegisterWifiStaConnListener(DEVICESIGMANAGER->WifiConnStatusCallback);
//	SSTAR_RegisterWifiStaScanListener(DEVICESIGMANAGER->WifiSignalStatusCallback);
//	SSTAR_RegisterUsbListener(ShowUsbStatus);
//
//	SNMANAGER->startGetSnThread();
//	MODELMANAGER->startUpgradeApp();

}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
	DEBUG_INFO();
    if (intentPtr != NULL) {
        //TODO
    }
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {
	DEBUG_INFO();
	

//	NZ3802HwReset();
//	MifarePresent(1);
//	NZ3802Test();
//	std::string filePath = "../data/testPass";
//	bool result = FILEUTILS->isFileExists_stat(filePath);
//	if(result || !IS_FACTORY_MOD){
//		switch (UI_TYPE) {
//			case UI_DEF_LANDSCAPE:
////				EASYUICONTEXT->openActivity("playerActivity");
//				// EASYUICONTEXT->openActivity("player800x640Activity");
//	//			EASYUICONTEXT->openActivity("player1280x1024Activity");
////				EASYUICONTEXT->openActivity("player1920x1080Activity");
//				EASYUICONTEXT->openActivity("playerSelectActivity");
////				EASYUICONTEXT->openActivity("player1440x900Activity");
//				break;
//			case UI_PORTRAIT_DOUBLE_SCREEN:
//				EASYUICONTEXT->openActivity("doubleScreenActivity");
//				break;
//			default:
//				break;
//		}
//
//	}else{
//		EASYUICONTEXT->openActivity("factoryTestActivity");
//	}

}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {
	DEBUG_INFO();
}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
	DEBUG_INFO();
//	SSTAR_UnRegisterWiredNetworkListener(ShowWiredNetworkStatus);
	// SSTAR_UnRegisterWifiStaConnListener(DEVICESIGMANAGER->WifiConnStatusCallback);
	// SSTAR_UnRegisterWifiStaScanListener(DEVICESIGMANAGER->WifiSignalStatusCallback);
	// SSTAR_DeinitHotPlugDetect();
//	WATCHDOGMANAGER->closeWatchdog();
	
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
	// case 0:
	// 	WATCHDOGMANAGER->keep_alive();
	// 	break;
	// case 1:
	// 	MODELMANAGER->startUpgradeApp();
	// 	BA407SSPARSER->upgradeBA407SS();
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
static bool onmainActivityTouchEvent(const MotionEvent &ev) {
	DEBUG_INFO();
//    switch (ev.mActionStatus) {
//		case MotionEvent::E_ACTION_DOWN://触摸按下
//			LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
//			downTime = ev.mEventTime;
//			break;
//		case MotionEvent::E_ACTION_MOVE://触摸滑动
////			setMousePos(ev.mX, ev.mY);
//			break;
//		case MotionEvent::E_ACTION_UP:  //触摸抬起
//			long time = ev.mEventTime - downTime;
//			if(time > 3000){
//				EASYUICONTEXT->openActivity("networkSettingActivity");
//			}
//			break;
//		default:
//			break;
//	}
	return false;
}
static bool onButtonClick_Button1(ZKButton *pButton) {
	DEBUG_INFO();
    LOGD(" ButtonClick Button1 !!!\n");
    return false;
}
