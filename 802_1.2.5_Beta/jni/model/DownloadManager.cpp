/*
 * DownloadManager.cpp
 *
 *  Created on: 2021年1月19日
 *      Author: Administrator
 */


#include "model/DownloadManager.h"

#include "utils/log.h"
#include "restclient-cpp/restclient.h"
#include "../curl/curl.h"
#include "baselib/FileUtils.h"

DownloadManager::DownloadManager(){

}

DownloadManager* DownloadManager::getInstance(){
	static DownloadManager instance;
	return &instance;
}

void DownloadManager::startDownloadFile(s_downloadInfo info){
	LOGD("---startDownloadFile----%s\n", info.url.c_str());
	if(!isRunning()){
		run("download");
	}
	downloadQueue.push(info);
}

/**
 * 线程创建成功后会调用该函数，可以在该函数中做一些初始化操作
 * return true   继续线程
 *        false  退出线程
 */
bool DownloadManager::readyToRun() {
	return true;
}

/**
 * 线程循环函数
 *
 * return true  继续线程循环
 *        false 推出线程
 */
bool DownloadManager::threadLoop(){

	 if (exitPending()) {
		 LOGD("DownloadManager - 退出线程\n");
		 return false;
	 }

	if(!downloadQueue.empty()){
		isBusy = true;
		sleepCount = 0;
		s_downloadInfo info = downloadQueue.front();
		downloadQueue.pop();
		int index = info.url.find("https");
		LOGD("Download file - url:%s  -path:%s -index:%d\n",info.url.c_str(), info.path.c_str(), index);
		if(index >= 0){
			info.url.replace(0, index + 5, "http");
		}

		if(FILEUTILS->isFileExists_stat(info.path)){
			remove(info.path.c_str());
		}

		RestClient::Response r = RestClient::download(info.url, info.path);
		if(r.code == 200){
			LOGD("下载成功 -- %s\n", info.path.c_str());
			if(info.callback != NULL){
				info.callback(true, info.path, info.version);
			}
		}else{
			LOGD("下载失败 --%d  -- %s --- %s\n", r.code, r.body.c_str(), info.path.c_str());
			if(info.callback != NULL){
				info.callback(false, info.path, info.version);
			}
		}
	}else{
		isBusy = false;
		LOGD("Download Thread sleep\n");
		sleep(1000 * 10);
		sleepCount++;
		if(sleepCount >= 5){
			return false;
		}
	}

	return true;
}

