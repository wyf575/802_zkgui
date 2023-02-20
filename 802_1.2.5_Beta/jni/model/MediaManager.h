/*
 * MediaManager.h
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_MEDIAMANAGER_H_
#define JNI_MODEL_MEDIAMANAGER_H_

#include "JsonParse.h"
using namespace std;

class MediaManager{
public:
	static MediaManager* getInstance();
	void initPlayList();
	JsonParse::MediaResourceList getLocalPlayList();
	std::vector<std::string> getNotIncludeVideoTaskId(JsonParse::MediaResourceList list1, JsonParse::MediaResourceList list2);
	JsonParse::MediaResource* getMediaResourcesForPlay();
	static string getMediaPath(string url);
	bool idPicMediaType(JsonParse::MediaResource resource);
	void scan_one_dir(const char * dir_name);
	void scan_dir_and_savePath(const char * dir_name);
	void check_local_video();

private:
	MediaManager();
	int getPlayIndex();
	void downloadMediaFile(JsonParse::MediaResource mediaResource);
	bool checkVideoFileIsDownLoad(string url, int mediaType);
	bool checkPicFileIsDownLoad(Json::Value arr);

};

#define MEDIAMANAGER 	MediaManager::getInstance()
#endif /* JNI_MODEL_MEDIAMANAGER_H_ */
