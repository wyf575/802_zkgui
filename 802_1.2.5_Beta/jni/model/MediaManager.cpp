/*
 * MediaManager.cpp
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#include "MediaManager.h"
#include "storage/StoragePreferences.h"
#include "ModelManager.h"
#include "baselib/TimeUtils.h"
#include "baselib/FileUtils.h"
#include "restclient-cpp/restclient.h"
#include "../curl/curl.h"
#include <pthread.h>
#include "utils/log.h"
#include "model/DownloadManager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

using namespace std;

const int  MEDIA_TYPE_PICTURE_1 = 1;
const int  MEDIA_TYPE_VIDEO_1 = 2;
const int  MEDIA_TYPE_VIDEO = 3;
const int  MEDIA_TYPE_PICTURE = 4;

static JsonParse::MediaResourceList mResources;
static int mIndexOfNow;

char* curScanDir = NULL;

static string videoType[]={
		".mp4",
		".avi"
};

static string imgType[]={
		".jpg",
		".png"
};

MediaManager::MediaManager(){
}

MediaManager* MediaManager::getInstance(){
	static MediaManager mInstance;
	return &mInstance;
}

void MediaManager::initPlayList(){
	std::string playListstr = StoragePreferences::getString(SPKEY_PLAYLIST, "");
	LOGD(" initPlayList : %s \n", playListstr.c_str());
	if(playListstr.length() > 0){
		Json::Reader reader;
		Json::Value root;
		if(reader.parse(playListstr, root, false)){
			mResources.clear();
			JsonParse::parsePlayListData(root, mResources);
			scan_dir_and_savePath(NULL);
			scan_one_dir("/mnt/sdcard");
			LOGD(" initPlayList  %d \n", mResources.size());
		}
	}
	check_local_video();
}

JsonParse::MediaResourceList MediaManager::getLocalPlayList(){
	return mResources;
}

std::vector<std::string> MediaManager::getNotIncludeVideoTaskId(JsonParse::MediaResourceList beArray, JsonParse::MediaResourceList comArray) {

	std::vector<std::string> srcIdList;
	int conArraySize = comArray.size();
	for (int i = 0; i < conArraySize; i++) {
		bool isOntheList = false;
		std::string srcIdLocal = comArray[i].teskId;
		if(srcIdLocal.length() < 1){
			continue;
		}

		for (int j = 0; j < beArray.size(); j++) {
			std::string remoteSrcId = beArray[i].teskId;
			if(remoteSrcId.length() < 1){
				continue;
			}else if (remoteSrcId.find(srcIdLocal) >= 0) {
				isOntheList = true;
				break;
			}
		}
		if (!isOntheList) {
			srcIdList.push_back(srcIdLocal);
		}
	}
	return srcIdList;
}

int MediaManager::getPlayIndex(){
	if(mResources.size() <= 0){
		return -1;
	}
	if(mIndexOfNow >= mResources.size()){
		mIndexOfNow = 0;
	}

	for(int i = mIndexOfNow; i < mResources.size(); i++){

		JsonParse::MediaResource mediaResource = mResources[i];

		int video_start_date = TIMEUTILS->convertDate(mediaResource.mStartDate);
		int video_end_date = TIMEUTILS->convertDate(mediaResource.mEndDate);

		int video_start_time = TIMEUTILS->convertTime(mediaResource.mStartTime);
		int video_end_time = TIMEUTILS->convertTime(mediaResource.mEndTime);

		int now_date = TIMEUTILS->getDateNow();
		int now_time = TIMEUTILS->getTimeNow();

		if ((video_start_date == 0 && video_end_date == 0) || (video_start_date <= now_date && now_date <= video_end_date)) {
			if ((video_start_time == 0 && video_end_time == 0) || (video_start_time <= now_time && now_time <= video_end_time)) {
				mIndexOfNow = i + 1;
				return i;
			}
		}

		mIndexOfNow = i + 1;
	}
	return 0;
}

static string MediaManager::getMediaPath(string url){

	int index = url.find_last_of("/");
	if(index == -1){
		return "";
	}
	return FILEUTILS->getDefPath() + url.substr(index);
}

bool MediaManager::idPicMediaType(JsonParse::MediaResource resource) {
	int mediaType = resource.mMediaType;
	if (mediaType == MEDIA_TYPE_PICTURE_1 || mediaType == MEDIA_TYPE_PICTURE) {
		return true;
	} else {
		return false;
	}
}

bool MediaManager::checkVideoFileIsDownLoad(string url, int mediaType){
	std::string video_now_playing_path = getMediaPath(url);
	if(!FILEUTILS->isFileExists_stat(video_now_playing_path)){
		return false;
	}
	LOGD("checkMediaFileIsDownLoad --%d - %s\n", DOWNLOADMANAGER->isBusy, video_now_playing_path.c_str());
	if(!DOWNLOADMANAGER->isBusy){
		int fileSize = FILEUTILS->getFileSize(video_now_playing_path);
		if(fileSize == 0){
			return false;
		}
		LOGD("checkMediaFileIsDownLoad --true\n");
		return true;
	}
	return false;
}

bool MediaManager::checkPicFileIsDownLoad(Json::Value arr){
	LOGD("-----checkPicFileIsDownLoad --- \n");
	for(Json::ArrayIndex i = 0; i < arr.size(); i++){
		string url = arr[i].asString();
		string path = getMediaPath(url);
		if(!FILEUTILS->isFileExists_stat(path)){
			return false;
		}

		int fileSize = FILEUTILS->getFileSize(path);
		if(fileSize == 0){
			return false;
		}
	}
	LOGD("-----checkPicFileIsDownLoad --- true\n");
	return true;
}

//取得当前应当播放的视频或图片地址,下载视频或图片
JsonParse::MediaResource* MediaManager::getMediaResourcesForPlay(){
	if(mResources.size() > 0){
		int resourceSize = mResources.size();
		for (int var = 0; var < resourceSize; ++var) {
			int index = getPlayIndex();
			LOGD("getPlayIndex --%d ---%d ---%d\n", index, var, resourceSize);
			if (index == -1 || index >= resourceSize) {
				return NULL;

			} else {
				//更新视频的下载状态
				//检查视频是否加载完成
//				static JsonParse::MediaResource mediaResource = mResources[index];
				LOGD("getMediaResourcesForPlay.mMediaType %d %d  %d\n", mResources[index].mMediaType, mResources[0].mMediaType, mResources[1].mMediaType);
				if (mResources[index].filePath.length() > 0) {
					LOGD("getMediaResourcesForPlay.filePath %s\n", mResources[index].filePath.c_str());
					return &mResources[index];

				} else if (mResources[index].mSrcUrl.length() > 0 && checkVideoFileIsDownLoad(mResources[index].mSrcUrl, mResources[index].mMediaType)) {
					string path = getMediaPath(mResources[index].mSrcUrl);
					mResources[index].filePath = path;
					LOGD("getMediaResourcesForPlay --- %s\n", mResources[index].filePath.c_str());
					return &mResources[index];

					//检查图片是否加载完成
				} else if (mResources[index].srcPicArray.size() > 0 && checkPicFileIsDownLoad(mResources[index].srcPicArray)) {
					return &mResources[index];

				} else {
					if(!DOWNLOADMANAGER->isBusy){
						LOGD("downloadThread ---  %d\n", mResources[index].mMediaType);
						if (mResources[index].mMediaType == MEDIA_TYPE_VIDEO || mResources[index].mMediaType == MEDIA_TYPE_VIDEO_1) {
							std::string path = getMediaPath(mResources[index].mSrcUrl);

							LOGD(" downloadThread  VIDEO - %d - %s\n", index, mResources[index].mSrcUrl.c_str());
							RestClient::Response r = RestClient::download(mResources[index].mSrcUrl, path);
							DownloadManager::s_downloadInfo info;
							info.path = path;
							info.url = mResources[index].mSrcUrl;
							DOWNLOADMANAGER->startDownloadFile(info);
						}else if(mResources[index].mMediaType == MEDIA_TYPE_PICTURE || mResources[index].mMediaType == MEDIA_TYPE_PICTURE_1){
							LOGD(" downloadThread  PIC %d\n", mResources[index].srcPicArray.size());
							if(mResources[index].srcPicArray.isArray()){
								LOGD(" downloadThread  PIC---\n");
								for(Json::ArrayIndex i = 0; i < mResources[index].srcPicArray.size(); i++){
									LOGD(" downloadThread  i -- %d\n", i);
									string url = mResources[index].srcPicArray[i].asString();

									LOGD(" downloadThread  url -- %s\n", url.c_str());
									std::string path = MediaManager::getMediaPath(url);
									LOGD(" downloadThread  path -- %s\n", path.c_str());

									DownloadManager::s_downloadInfo info;
									info.path = path;
									info.url = url;
									DOWNLOADMANAGER->startDownloadFile(info);
								}
							}else{
								LOGD("!mediaResource->srcPicArray.isArray()");
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

void MediaManager::scan_dir_and_savePath(const char * dir_name){
	if(NULL == dir_name && curScanDir == NULL)
	{
		LOGD(" dir_name is null !\n");
		curScanDir = "/vendor/udisk_sda";
	}else{
		curScanDir = dir_name;
	}
	scan_one_dir(curScanDir);
}

void MediaManager::scan_one_dir(const char * dir_name)
{
	printf("scan_one_dir = %s\n", dir_name);

	struct stat s;
	int result = lstat(dir_name , &s);
	LOGD("lstat - result = %s, %d, %d\n", dir_name, result, errno);
	if(!S_ISDIR(s.st_mode))
	{
		LOGD("!S_ISDIR\n");
		return;
	}

	struct dirent * filename;
	DIR * dir;
	dir = opendir(dir_name);
	if( NULL == dir )
	{
		LOGD(" dir == NULL\n");
		return;
	}

	while(( filename = readdir(dir)) != NULL )
	{
//		LOGD("filename->d_name= %s\n", filename->d_name);
		if(strcmp(filename->d_name , ".") == 0 ||
			strcmp(filename->d_name , "..") == 0)
			continue;

		char wholePath[128] = {0};
		JsonParse::MediaResource resource;
		sprintf(wholePath, "%s/%s", dir_name, filename->d_name);
		resource.filePath = wholePath;
		resource.mMediaType = -1;

		for(string type : videoType){
			if(resource.filePath.compare(resource.filePath.length() - type.length(), type.length(), type) == 0){
				resource.mMediaType = MEDIA_TYPE_VIDEO;
				mResources.push_back(resource);
				LOGD("wholePath= %s\n", wholePath);
				break;
			}
		}
//		LOGD("resource.mMediaType= %d\n", resource.mMediaType);
		if(resource.mMediaType != MEDIA_TYPE_VIDEO){
			for(string type : imgType){
				if(resource.filePath.compare(resource.filePath.length() - type.length(), type.length(), type) == 0){
					resource.mMediaType = MEDIA_TYPE_PICTURE;
					mResources.push_back(resource);
					LOGD("wholePath= %s\n", wholePath);
					break;
				}
			}
		}

	}
}

void MediaManager::check_local_video(){
	struct stat s;
	char * curScanDir = FILEUTILS->getDefPath().c_str();
	int result = lstat(curScanDir , &s);
	LOGD("lstat - result = %s, %d, %d\n", curScanDir, result, errno);
	if(!S_ISDIR(s.st_mode))
	{
		LOGD("!S_ISDIR\n");
		return;
	}

	struct dirent * filename;
	DIR * dir;
	dir = opendir(curScanDir);
	if( NULL == dir )
	{
		LOGD(" dir == NULL\n");
		return;
	}

	while(( filename = readdir(dir)) != NULL )
	{
//		LOGD("filename->d_name= %s\n", filename->d_name);
		if(strcmp(filename->d_name , ".") == 0 ||
			strcmp(filename->d_name , "..") == 0)
			continue;

		char wholePath[128] = {0};
		sprintf(wholePath, "%s/%s", curScanDir, filename->d_name);
		if(mResources.size() > 0){
			for(int i = 0; i < mResources.size(); i++){
				JsonParse::MediaResource mediaResource = mResources[i];
				if (mediaResource.mMediaType == MEDIA_TYPE_PICTURE_1 || mediaResource.mMediaType == MEDIA_TYPE_PICTURE) {
					for(Json::ArrayIndex i = 0; i < mediaResource.srcPicArray.size(); i++){
							string url = mediaResource.srcPicArray[i].asString();

							int index = url.find_last_of("/");
							if(index != -1 && strcmp(url.substr(index + 1).c_str(), filename->d_name) != 0){
								remove(wholePath);
								LOGD("1-删除  %s\n", wholePath);
							}
					}
				}else{
					int index = mediaResource.mSrcUrl.find_last_of("/");
//					LOGD("--%s,%s\n", mediaResource.mSrcUrl.substr(index + 1).c_str(),filename->d_name);
					if(index != -1 && strcmp(mediaResource.mSrcUrl.substr(index + 1).c_str(), filename->d_name) != 0){
						remove(wholePath);
						LOGD("2-删除  %s\n", wholePath);
					}
				}
			}
		}else{
			remove(wholePath);
			LOGD("3-删除  %s\n", wholePath);
		}
	}
}

