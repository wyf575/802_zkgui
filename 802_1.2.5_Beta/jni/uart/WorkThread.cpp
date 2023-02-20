/*
 * WorkThread.cpp
 *
 *  Created on: 2021年4月6日
 *      Author: Administrator
 */

#include "WorkThread.h"
#include "Utils/Log.h"
#include "appconfig.h"
#include "sing7/Sing7Parser.h"
#include "SQ800/SQ800Parser.h"
#include "ba407ss/BA407SSParser.h"

WorkThread::WorkThread(){

}

WorkThread::~WorkThread(){
//	requestExit();
}

WorkThread* WorkThread::getInstance(){
	static WorkThread mInstance;
	return &mInstance;
}

void WorkThread::startWork(){
	LOGD("startWork=%d！\n", isRunning());
	if(!isRunning()){
		bool result = run("work_mqtt");
		if(result){
			LOGD("成功启动工作线程！\n");
		}else{
			LOGD("启动工作线程失败！\n");
		}
	}
}
/**
 * 线程创建成功后会调用该函数，可以在该函数中做一些初始化操作
 * return true   继续线程
 *        false  退出线程
 */
bool WorkThread::readyToRun() {
	return true;
}

/**
* 线程循环函数
*
* return true  继续线程循环
*        false 推出线程
*/
bool WorkThread::threadLoop() {
#ifdef BA407SS_SERIAL
	BA407SSPARSER->doTaskforThread();
#elif defined SQ800_SERIAL
	SQ800PARSER->doTaskforSQ800();
#elif defined SING7_SERIAL
	SING7PARSER->doTaskforSING7();
#else
	return false;
#endif
	Thread::sleep(2000);
	return true;
}


