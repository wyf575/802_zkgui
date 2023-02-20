/*
 * PlayListAndParamsParse.h
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_JSONPARSE_H_
#define JNI_MODEL_JSONPARSE_H_

#include <String>
#include "json/json.h"
#include "DownloadManager.h"

namespace JsonParse{

	const std::string DATA = "data";
	const std::string ERRORCODE = "errorCode";
	const std::string ID = "id";
	const std::string PARAMS = "params";
	const std::string PLAYLIST = "playList";
	const std::string DATESTARTANAL = "eStartAnal";
	const std::string DATEENDANAL = "dateEndAnal";
	const std::string TIMESTARTANAL = "timeStartAnal";
	const std::string TIMEENDANAL = "timeEndAnal";
	const std::string MEDIARESOURCE = "mediaResource";
	const std::string MEDIAID = "mediaId";
	const std::string MEDIANAME = "mediaName";
	const std::string INTERVAL = "interval";
	const std::string PICRESOURCE = "picResource";
	const std::string MEDIATYPE = "mediaType";
	const std::string TASKID = "taskId";
	const std::string PARAM = "param";
	const std::string CHECKED = "checked";
	const std::string DESC = "desc";
	const std::string KEY = "key";
	const std::string NAME = "name";
	const std::string REG = "reg";
	const std::string SHOW = "show";
	const std::string VALUE = "value";
	const std::string ENDDATE = "endDate";
	const std::string STARTDATE = "startDate";
	const std::string STARTTIME = "startTime";
	const std::string ENDTIME = "endTime";
	const std::string CONTENT = "content";
	const std::string CMD = "cmd";
	const std::string MSG = "msg";
	const std::string EXTEND = "extend";
	const std::string SEQ = "seq";
	const std::string DIGITAL = "digital";
	const std::string INPUT = "input";
	const std::string COUNT = "count";
	const std::string REPLENISH = "Replenish";
	const std::string CMD_ID = "cmd_id";
	const std::string RESULT = "result";
	const std::string STATUS = "status";
	const std::string CMDID = "cmdId";
	const std::string VERSION = "v";
	const std::string SIG = "sig";
	const std::string SN = "sn";
	const std::string CHANNEL = "channel";
	const std::string IMEI = "imei";
	const std::string ENABLE = "enable";
	const std::string FILE = "file";
	const std::string CRC = "crc";
	const std::string VERSION_ALL_WORD = "version";
	const std::string ICCID = "iccid";
	const std::string BASELOC = "baseLoc";
	const std::string SIZE = "size";
	const std::string NICKNAME = "nickname";
	const std::string TOTAL = "total";
	const std::string CURRENT = "current";

	const int CMD_3812 = 3812;
	const int CMD_4812 = 4812;
	const int CMD_5812 = 5812;
	const int CMD_3816 = 3816;
	const int CMD_PING_3802 = 3802;
	const int CMD_PING_4802 = 4802;
	const int CMD_PARAM_UPDATE = 3807;
	const int CMD_PARAM_UPDATE_REPLY = 4807;
	const int CMD_REBOOT = 3803;
	const int CMD_REBOOT_REPLY = 4803;
	const int CMD_REGISTER = 3801;

	typedef struct {
	    std::string filePath;
	    int interval = 0;
	    Json::Value srcPicArray;
	    std::string mStartDate;
	    std::string mEndDate;
	    std::string mStartTime;
	    std::string mEndTime;
	    std::string mSrcUrl;
	    std::string mSrcId;
	    std::string mSrcName;
	    int mMediaType;
	    bool isDownloading = false;
	    std::string teskId;
	} MediaResource;

	////参数中携带的时间段信息
	typedef struct{
		std::string endDate;
		std::string startDate;
		std::string startTime;
		std::string endTime;
		std::string content;
	} Schedule;

	typedef struct{
		std::string isChecked; //是否有多项
		std::string desc;        //参数描述
		std::string key;
		std::string name;        //参数名称
		std::string reg;         //正则表达式
		bool show;
		std::vector<Schedule> values;
	} Param;

	typedef struct{
		std::string seq;
		int cmd;
		std::string msg;
		Json::Value extend;
	} S_MqttMsg;

	typedef struct{
		int paramId;
		std::vector<Param> paramList;
	} Params;

	struct DataState{
		std::string state;
		int cargoWay;
		bool isResult;
		DataState(){
		}
		DataState(bool isResult){
			this->isResult = isResult;
		}
		DataState(std::string state, int cargoWay, bool isResult){
			this->state = state;
			this->cargoWay = cargoWay;
			this->isResult = isResult;
		}
	};

	typedef std::vector<MediaResource> MediaResourceList;

	void parsePlayList(std::string msg);
	void parsePlayListData(Json::Value& playList, MediaResourceList& videoList);
	bool parseUpgradeData(std::string json, std::string curVersion, std::string path, DownloadManager::DownloadCallbackFun callback);

	void parseParams(std::string msg, Params &params);
	void parseParamArr(Json::Value jsonArr, Params &params);
	void parseParam(Json::Value jsonObj, Param &param);
	S_MqttMsg parseMqttMsg(std::string msgStr);
	void do3812Task(Json::Value extend);
	std::string buildCmdResponse(Json::Value extend, DataState state);
	std::string buildCmdResponse(int cmd, std::string curCmd_Id, std::string state, bool isResult);
	string buildCmdResponse(int cmd, std::string curCmd_Id, int current, int total, std::string state, bool isResult);
	bool parseSN(std::string json);
	void initThreadDoMqttMsg();

	bool getMqttConnectStatus();

	std::string buildPingResponse(bool result, JsonParse::S_MqttMsg* mqttMsg);
	std::string buildParamUpdateResponse();
	std::string buildParamRebootResponse();
	std::string buildDeviceRegister();
}

#ifdef __cplusplus
extern "C" {
#endif

extern void paseMqttSubMsg(const char* msg);
extern void mqttConnectStatus(int status);

#ifdef __cplusplus
};
#endif


#endif /* JNI_MODEL_JSONPARSE_H_ */
