/*
 * WorkThread.h
 *
 *  Created on: 2021年4月6日
 *      Author: Administrator
 */

#ifndef JNI_UART_WORKTHREAD_H_
#define JNI_UART_WORKTHREAD_H_

#include "system/Thread.h"

class WorkThread : Thread{
public:
	virtual ~WorkThread();
	static WorkThread* getInstance();
	void startWork();

protected:
	virtual bool readyToRun();
	virtual bool threadLoop();

private:
	WorkThread();
};


#define WORKTHREAD 		WorkThread::getInstance()
#endif /* JNI_UART_WORKTHREAD_H_ */
