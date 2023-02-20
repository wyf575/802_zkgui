/*
 * DownloadManager.h
 *
 *  Created on: 2021年1月19日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_DOWNLOADMANAGER_H_
#define JNI_MODEL_DOWNLOADMANAGER_H_

#include "system/Thread.h"
#include <string>
#include <queue>

using namespace std;



class DownloadManager:public Thread{

public:
	typedef void (*DownloadCallbackFun)(bool result, string path, string version);
	static DownloadManager* getInstance();
	bool isBusy;
	int sleepCount;
	typedef struct {
		string url;
		string path;
		string version;
		DownloadCallbackFun callback = NULL;
	}s_downloadInfo;

	queue<s_downloadInfo> downloadQueue;
	void startDownloadFile(s_downloadInfo info);

protected:
	virtual bool readyToRun();
	virtual bool threadLoop();

private:
	DownloadManager();

};

#define DOWNLOADMANAGER		DownloadManager::getInstance()
#endif /* JNI_MODEL_DOWNLOADMANAGER_H_ */
