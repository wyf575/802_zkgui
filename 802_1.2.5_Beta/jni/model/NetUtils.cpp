/*
 * NetUtils.cpp
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#include "NetUtils.h"

#include "baselib/UrlManager.h"
#include "restclient-cpp/restclient.h"
#include "json/json.h"
#include "model/JsonParse.h"
#include "model/ModelManager.h"
#include "storage/StoragePreferences.h"
#include "Utils/Log.h"

void NetUtils::sendConfigSuccessMsg(int id){
	Json::Value root;
	std::string sn = StoragePreferences::getString(SPKEY_SN, "未找到SN");
	root[JsonParse::SN] = sn;
	root["id"] = id;

	RestClient::Response response = RestClient::post(URLMANAGER->getConfigSuccessUrl(), "application/json", root.toStyledString());
	if(response.code == 200){
		LOGD("参数更新上报成功！\n");
	}else{
		LOGD("参数更新上报失败！\n");
	}
}

std::string buildArray(std::vector<std::string> vectorList){
	Json::Value array;
	for(std::string taskId : vectorList){
		if(!taskId.empty()){
			array.append(atoi(taskId.c_str()));
		}
	}
	return array.toStyledString();
}

void NetUtils::http_post_video_list_result(std::vector<std::string> newVideoId, std::vector<std::string> oldVideoId){
	std::string content_type = "application/json";
	std::string url = URLMANAGER->getReportVideolistUrl();
	std::string sn = StoragePreferences::getString(SPKEY_SN, "未找到SN");

	Json::Value root;
	Json::Value failed;
	root[JsonParse::SN] = sn;
	root["success"] = buildArray(newVideoId);
	root["failed"] = failed.toStyledString();
	root["delete"] = buildArray(oldVideoId);
	root["reason"] = "";
	RestClient::Response response = RestClient::post(url, content_type, root.toStyledString());

	if(response.code == 200){
		LOGD("视频更新上报成功！");
	}else{
		LOGD("视频更新上报失败！");
	}
}
