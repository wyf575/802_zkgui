/*
 * PlayListAndParamsParse.cpp
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#include "JsonParse.h"

#include <string>
#include "json/json.h"
#include "utils/Log.h"
#include "storage/StoragePreferences.h"
#include "ModelManager.h"
#include "NetUtils.h"
#include "MediaManager.h"
#include "baselib/Base64Utils.h"
#include "baselib/DeviceManager.h"
#include "baselib/Utils.h"
#include "uart/ba407ss/BA407SSParser.h"
#include "uart/sing7/Sing7Parser.h"
#include "uart/SQ800/SQ800Parser.h"
#include "utils/TimeHelper.h"
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include "../appconfig.h"
#include "../net/Module4GManager.h"
#include "baselib/FileUtils.h"
#include "net/DeviceSigManager.h";

static queue<string> mqttMsgQueue;
static bool mqttIsConnect = false;

#ifdef __cplusplus
extern "C" {
#endif

void paseMqttSubMsg(const char* msg){
	mqttMsgQueue.push(msg);
	LOGD("---paseMqttSubMsg ---%d ----%s\n", mqttMsgQueue.size(), mqttMsgQueue.front().c_str());
}

void mqttConnectStatus(int status){
	LOGD("mqttConnectStatus - %d\n", status);
	mqttIsConnect = status;
	if(status){
		MODELMANAGER->sendMsgToMqttService(JsonParse::buildDeviceRegister());
	}
}

#ifdef __cplusplus
};
#endif

void JsonParse::do3812Task(Json::Value extend){
	LOGD("----------do3812Task--------\n");
#ifdef BA407SS_SERIAL
	BA407SSPARSER->parseMsg(extend);
#elif defined SQ800_SERIAL
	SQ800PARSER->parseSQ8003812Msg(extend);
#elif defined SING7_SERIAL
	SING7PARSER->parseSING73812Msg(extend);
#endif
}

void* mqttTaskThread(){
 while(1){
	 if(mqttMsgQueue.size() > 0){
		string msg = mqttMsgQueue.front();
		mqttMsgQueue.pop();
		std::string msgStr = BASE64UTILS->base64_decode(msg.c_str());
		LOGD("MQTT消息 - %s\n", msgStr.c_str());

		JsonParse::S_MqttMsg mqttMsg = JsonParse::parseMqttMsg(msgStr);
		if(mqttMsg.cmd > 0){
			switch (mqttMsg.cmd) {
				case JsonParse::CMD_3812:
					JsonParse::do3812Task(mqttMsg.extend);
					break;
				case JsonParse::CMD_5812:

					break;
				case JsonParse::CMD_PING_3802:
					MODELMANAGER->sendMsgToMqttService(JsonParse::buildPingResponse(true, &mqttMsg));
					break;
				case JsonParse::CMD_REBOOT:
					MODELMANAGER->sendMsgToMqttService(JsonParse::buildParamRebootResponse());
					DEVICEMANAGER->rebootDevice();
					break;
				case JsonParse::CMD_PARAM_UPDATE:
					MODELMANAGER->getPlayListAndParams();
					MODELMANAGER->sendMsgToMqttService(JsonParse::buildParamUpdateResponse());
					break;
				default:
					break;
			}
		}

	 }
	 else{
		 sleep(2);
//		 LOGD("   ---- sleep --- 10-----   \n");
	 }
 }
 return NULL;
}

void JsonParse::initThreadDoMqttMsg(){
	pthread_t thread;
	int ret = pthread_create(&thread, NULL, (void*)mqttTaskThread, NULL);
	LOGD("---------initThreadDoMqttMsg------------%d\n", ret);
	if(ret){
		LOGD("创建MQTT消息处理线程失败\n");
	}else{
		LOGD("创建MQTT消息处理线程成功\n");
	}
}

std::string JsonParse::buildDeviceRegister(){
	Json::Value root;
	root[CMD] = CMD_REGISTER;
	root[DIGITAL] = 0;

	Json::Value extend;
	extend[VERSION] = APP_VERSION;
	extend[IMEI] = MODULE4GMANAGER->getIMEI();
	extend[ENABLE] = 1;
	extend[SIG] = DEVICESIGMANAGER->getNetSignal()->sig;
	extend[ICCID] = MODULE4GMANAGER->getICCID();
	extend[BASELOC] = MODULE4GMANAGER->getBaseLoc();
	root[EXTEND] = extend;
	root[MSG] ="success";
	return root.toStyledString();
}

std::string JsonParse::buildPingResponse(bool result, S_MqttMsg* mqttMsg){
	Json::Value root;

	root[CMD] = CMD_PING_4802;
	root[MSG] = "pong";
	root[EXTEND] = NULL;
	root[DIGITAL] = 0;


	return root.toStyledString();
}

std::string JsonParse::buildParamUpdateResponse(){
	Json::Value root;

	root[CMD] = CMD_PARAM_UPDATE_REPLY;
	root[MSG] = "paramUpdate";
	root[EXTEND] = NULL;
	root[DIGITAL] = 0;

	return root.toStyledString().c_str();
}

std::string JsonParse::buildCmdResponse(Json::Value extend, DataState state){
	Json::Value root;
	root[CMD] = CMD_4812;
	root[MSG] = "success";

	Json::Value curExtend;
	if(state.state.size() > 0){
		curExtend[RESULT] = state.state == "wait" ? "wait" : (state.isResult ? "true" : "false");
		curExtend[STATUS] = state.state;
	}else{
		curExtend[RESULT] = state.isResult;
	}

	if(extend.isMember(CMD_ID)){
		curExtend[CMDID] = extend[CMD_ID].asString();
	}
	if(extend.isMember(CMD)){
		curExtend[CMD] = extend[CMD].asString();
	}
	root[EXTEND] = curExtend;
	root[DIGITAL] = UTILS->lltoString(TimeHelper::getCurrentTime());

	return root.toStyledString();
}

std::string JsonParse::buildCmdResponse(int cmd, std::string curCmd_Id, std::string state, bool isResult){
	return buildCmdResponse(cmd, curCmd_Id, 1, 1, state, isResult);
}

std::string JsonParse::buildCmdResponse(int cmd, std::string curCmd_Id, int current, int total, std::string state, bool isResult){
	Json::Value root;
	root[CMD] = CMD_4812;
	root[MSG] = "success";

	Json::Value curExtend;
	if(state.size() > 0){
		curExtend[RESULT] = state == "wait" ? "wait" : (isResult ? "true" : "false");
		curExtend[STATUS] = state;
	}else{
		curExtend[RESULT] = isResult;
	}
	curExtend[CMD] = cmd;
	curExtend[CMD_ID] = curCmd_Id;

	if(current != 0 && total > 1){
		curExtend[TOTAL] = total;
		curExtend[CURRENT] = total;
	}
	root[EXTEND] = curExtend;
	root[DIGITAL] = UTILS->lltoString(TimeHelper::getCurrentTime());

	return root.toStyledString();
}

std::string JsonParse::buildParamRebootResponse(){
	Json::Value root;

	root[CMD] = CMD_REBOOT_REPLY;
	root[MSG] = "reboot";
	root[EXTEND] = NULL;
	root[DIGITAL] = 0;

	return root.toStyledString().c_str();
}

JsonParse::S_MqttMsg JsonParse::parseMqttMsg(std::string msgStr){
	Json::Reader reader;
	Json::Value value;
	S_MqttMsg mqttMsg;
	if(reader.parse(msgStr, value, false)){
		LOGD("-----解析Mqtt消息------%s\n", msgStr.c_str());

		if(value.isMember(CMD)){
			mqttMsg.cmd = value[CMD].asInt();
			LOGD("-----解析Mqtt消息------%d\n", mqttMsg.cmd);
		}
		if(value.isMember(MSG)){
			mqttMsg.msg = value[MSG].asString();
		}
		if(value.isMember(EXTEND)){
			mqttMsg.extend = value[EXTEND];
		}
		if(value.isMember(SEQ)){
			mqttMsg.seq = value[SEQ].asString();
		}
		LOGD("解析Mqtt消息成功\n");
	}
	return mqttMsg;
}
void JsonParse::parsePlayListData(Json::Value& playList,
		MediaResourceList& videoList) {
	if (playList.isArray() && playList.size() > 0) {
		for (Json::ArrayIndex i = 0; i < playList.size(); i++) {
			JsonParse::MediaResource resource;
			if (playList[i].isMember(DATESTARTANAL)) {
				resource.mStartDate = playList[i][DATESTARTANAL].asString();
			}
			if (playList[i].isMember(DATEENDANAL)) {
				resource.mEndDate = playList[i][DATEENDANAL].asString();
			}
			if (playList[i].isMember(TIMESTARTANAL)) {
				resource.mStartTime = playList[i][TIMESTARTANAL].asString();
			}
			if (playList[i].isMember(TIMEENDANAL)) {
				resource.mEndTime = playList[i][TIMEENDANAL].asString();
			}
			if (playList[i].isMember(MEDIARESOURCE)) {
				resource.mSrcUrl = playList[i][MEDIARESOURCE].asString();
			}
			if (playList[i].isMember(MEDIAID)) {
				resource.mSrcId = playList[i][MEDIAID].asString();
			}
			if (playList[i].isMember(MEDIANAME)) {
				resource.mSrcName = playList[i][MEDIANAME].asString();
			}
			if (playList[i].isMember(INTERVAL)) {
				resource.interval = playList[i][INTERVAL].asInt();
			}
			if (playList[i].isMember(PICRESOURCE)) {
				resource.srcPicArray = playList[i][PICRESOURCE];
			}
			if (playList[i].isMember(MEDIATYPE)) {
				resource.mMediaType = playList[i][MEDIATYPE].asInt();
			}
			if (playList[i].isMember(TASKID)) {
				resource.teskId = playList[i][TASKID].asString();
			}
			videoList.push_back(resource);
		}
	}
}

void JsonParse::parsePlayList(std::string msg){
	Json::Reader reader;
	Json::Value root;
	MediaResourceList videoList;
	LOGD("-------parsePlayList--------\n");
	if(reader.parse(msg, root, false)){
		LOGD("视频播放列表解析成功\n");
		if(root.isMember(DATA)){
			Json::Value data = root[DATA];
			if(data.isObject()){
				std::string newList = data.toStyledString();
				std::string oldList = StoragePreferences::getString(SPKEY_PLAYLIST_AND_PARAM, "");
				if(strcmp(newList.c_str(), oldList.c_str()) != 0){
					StoragePreferences::putString(SPKEY_PLAYLIST_AND_PARAM, newList);
					StoragePreferences::putBool(SPKEY_PARAMCHANGE, true);
				}
				if(data.isMember(PLAYLIST)){
					Json::Value playList = data[PLAYLIST];
					JsonParse::parsePlayListData(playList, videoList);
					MediaResourceList localList = MEDIAMANAGER->getLocalPlayList();
					std::vector<std::string> newVideoId = MEDIAMANAGER->getNotIncludeVideoTaskId(localList, videoList);
					std::vector<std::string> oldVideoId = MEDIAMANAGER->getNotIncludeVideoTaskId(videoList, localList);
					if (newVideoId.size() > 0 || oldVideoId.size() > 0) {
						NetUtils::http_post_video_list_result(newVideoId, oldVideoId);
					}
					StoragePreferences::putString(SPKEY_PLAYLIST, playList.toStyledString());
					MEDIAMANAGER->initPlayList();
				}
			}
		}
	}else{
		LOGD("视频播放列表解析失败");
	}
	LOGD("-------parsePlayList--------end\n");
}

void JsonParse::parseParam(Json::Value jsonObj, Param &param){
	if(jsonObj.isObject()){
		if(jsonObj.isMember(CHECKED)){
			param.isChecked = jsonObj[CHECKED].asString();
		}
		if(jsonObj.isMember(DESC)){
			param.desc = jsonObj[DESC].asString();
		}
		if(jsonObj.isMember(KEY)){
			param.key = jsonObj[KEY].asString();
		}
		if(jsonObj.isMember(NAME)){
			param.name = jsonObj[NAME].asString();
		}
		if(jsonObj.isMember(REG)){
			param.reg = jsonObj[REG].asString();
		}
		if(jsonObj.isMember(SHOW)){
			param.show = jsonObj[SHOW].asBool();
		}
		if(jsonObj.isMember(VALUE)){
			Json::Value value = jsonObj[VALUE];
			if(value.isArray()){
				for(Json::ArrayIndex i = 0; i < value.size(); i++){
					Json::Value valueObjJson = value[i];
					if(valueObjJson.isObject()){
						Schedule schedule;
						if(valueObjJson.isMember(STARTDATE)){
							schedule.startDate = valueObjJson[STARTDATE].asString();
						}
						if(valueObjJson.isMember(ENDDATE)){
							schedule.endDate = valueObjJson[ENDDATE].asString();
						}
						if(valueObjJson.isMember(CONTENT)){
							schedule.content = valueObjJson[CONTENT].asString();
						}
						if(valueObjJson.isMember(STARTTIME)){
							schedule.startTime = valueObjJson[STARTTIME].asString();
						}
						if(valueObjJson.isMember(ENDTIME)){
							schedule.endTime = valueObjJson[ENDTIME].asString();
						}
						LOGD("schedule.content=%s\n", schedule.content.c_str());
						param.values.push_back(schedule);
					}
				}
			}
		}
		LOGD("param.values=%s\n", param.values[param.values.size() - 1].content.c_str());
	}
}

void JsonParse::parseParamArr(Json::Value jsonArr, Params &params){
	LOGD("parseParamArr start\n");
	if(jsonArr.isArray() && jsonArr.size() > 0){
		Json::Value obj = jsonArr[0];
		if(obj.isObject()){
			if(obj.isMember(ID)){
				params.paramId = obj[ID].asInt();
			}
			if(obj.isMember(PARAMS)){
				Json::Value arr = obj[PARAMS];
				if(arr.isArray()){
					for(Json::ArrayIndex i = 0; i < arr.size(); i++){
						Json::Value param= arr[i];
						Param parm;
						parseParam(param, parm);
						if(parm.key.length() > 0){
							LOGD("parseParamArr parm.key=%s\n", parm.key.c_str());
							params.paramList.push_back(parm);
						}
					}
				}
			}
			LOGD("parseParamArr finish\n");
		}
	}
	LOGD("parseParamArr end\n");
}

void JsonParse::parseParams(std::string msg, Params &params){
	Json::Reader reader;
	Json::Value root;
	if(reader.parse(msg, root, false)){
		if(root.isMember(DATA)){
			Json::Value data = root[DATA];
			if(data.isObject() && data.isMember(PARAM)){
				Json::Value param = data[PARAM];
				LOGD("----parseParams---\n");
				parseParamArr(param, params);
				LOGD("----parseParams---%d\n", params.paramList.size());
			}
		}
	}
}

bool JsonParse::parseSN(std::string json){
	Json::Value root;
	Json::Reader reader;

	if(reader.parse(json, root, false)){
		if(root.isMember(DATA)){
			Json::Value jsonObject = root[DATA];
			if(jsonObject.isMember(SN)){
				string sn = jsonObject[SN].asString();
				string tmpSN = StoragePreferences::getString(SPKEY_SN, "");
				LOGD("---------------SN=%s\n", sn.c_str());
				if(sn.size() > 0 && tmpSN != sn){
					StoragePreferences::putString(SPKEY_SN, sn.c_str());
				}
			}
			if(jsonObject.isMember(CHANNEL)){
				string channel = jsonObject[CHANNEL].asString();
				string tmpChannel = StoragePreferences::getString(SPKEY_CHANNEL, "");
				LOGD("---------------channel=%s\n", channel.c_str());
				if(channel.size() > 0 && channel != tmpChannel){
					StoragePreferences::putString(SPKEY_CHANNEL, channel);
				}
			}
			return true;
		}
	}
	return false;
}

bool JsonParse::parseUpgradeData(string json, string curVersion, string path, DownloadManager::DownloadCallbackFun callback){
	Json::Value root;
	Json::Reader reader;
	bool result = true;
	LOGD("----parseUpgradeData----\n");
	if(reader.parse(json, root, false)){
		if(root.isMember(ERRORCODE) && root[ERRORCODE].asInt() != 0){
			result = false;
		}else if(root.isMember(DATA)){
			Json::Value data = root[DATA];
			if(data.isMember(FILE) && data.isMember(VERSION_ALL_WORD) && data.isMember(CRC)){
				string version = data[VERSION_ALL_WORD].asString();
				if(curVersion.compare(version) < 0){
					string url = data[FILE].asString();

					DownloadManager::s_downloadInfo info;
					info.path = path;
					info.url = url;
					info.version = version;
					info.callback = callback;
					DOWNLOADMANAGER->startDownloadFile(info);
				}
			}
		}
	}
	return result;
}

bool JsonParse::getMqttConnectStatus(){
	return mqttIsConnect;
}
