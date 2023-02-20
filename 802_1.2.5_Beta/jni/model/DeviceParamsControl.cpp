/*
 * DeviceParamsControl.cpp
 *
 *  Created on: 2020年12月24日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_DEVICEPARAMSCONTROL_CPP_
#define JNI_MODEL_DEVICEPARAMSCONTROL_CPP_

#include "DeviceParamsControl.h"
#include "JsonParse.h"
#include "utils/TimeHelper.h"
#include "utils/log.h"

static DeviceParamsControl::s_userParam curParam;

DeviceParamsControl::DeviceParamsControl(){

}

DeviceParamsControl* DeviceParamsControl::getInstance(){
	static DeviceParamsControl mInstance;
	return &mInstance;
}

void DeviceParamsControl::initDeviceParams(JsonParse::Params params){
	curParam.paramId = params.paramId;
	for(int i = 0; i < params.paramList.size(); i++){
		JsonParse::Param param = params.paramList[i];
		LOGD("DeviceParamsControl.key=%s\n", param.key.c_str());
		if(param.key == VOL){
			curParam.vol = param;
		}else if(param.key == QRCODEDISPLAY){
			curParam.mQRCodeDisplay = param;
		}else if(param.key == REBOOTTIME){
			curParam.rebootTime = param;
		}else if(param.key == VERSION){
			curParam.versionDisplay = param;
		}else if(param.key == RUNTIME){
			curParam.runTime = param;
		}else if(param.key == ISSHOWLEFTBOTTOMLABEL){
			curParam.leftBottomLabel = param;
		}else if(param.key == QRLINK){
			curParam.mQrLink = param;
		}else if(param.key == QRTIP){
			curParam.mQrTip = param;
		}else if(param.key == BGIMG){
			curParam.mBgImg = param;
		}else if(param.key == CLOSESERIAL){
			curParam.closeSerial = param;
		}else if(param.key == ISPORTRAIT){
			curParam.isPortrait = param;
		}else if(param.key == ISUPLOADLOG){
			curParam.isUploadLog = param;
		}else if(param.key == NEWIMEI){
			curParam.newImei = param;
		}else if(param.key == QR_SIZE){
			curParam.mQrSize = param;
		}else if(param.key == QR_TIP_COLOR){
			curParam.mQrTipColor = param;
		}else if(param.key == QR_LOCATION){
			curParam.mQrLocation = param;
		}
	}
}

int DeviceParamsControl::getParamId(){
	return curParam.paramId;
}

int DeviceParamsControl::getVolPercent(){
	if(curParam.vol.key.length() > 0){
		LOGD("getVolPercent key=%s\n", curParam.vol.key.c_str());
		std::string s_vol = getValueByTime(curParam.vol);
		LOGD("getVolPercent value=%s\n", s_vol.c_str());
		if(s_vol.length() > 0){
			return atoi(s_vol.c_str());
		}
	}
	return -1;
}

bool DeviceParamsControl::isDisplayQRCode(){
	if(curParam.mQRCodeDisplay.key.length() > 0){
		std::string isDisplay = getValueByTime(curParam.mQRCodeDisplay);
		if(isDisplay == "no"){
			return false;
		}
	}
	return true;
}

string DeviceParamsControl::displayCustomQRCode(){
	LOGD("displayCustomQRCode=%s\n", curParam.mQrLink.key.c_str());
	if(curParam.mQrLink.key.length() > 0){
		std::string qrLink = getValueByTime(curParam.mQrLink);
		return qrLink;
	}
	return "";
}

string DeviceParamsControl::getQrSize(){
	if(curParam.mQrSize.key.length() > 0){
		string qrSize = getValueByTime(curParam.mQrSize);
		return qrSize;
	}
	return "";
}

std::string DeviceParamsControl::getValueByTime(JsonParse::Param param){
	LOGD("getValueByTime key=%s size=%d\n", param.key.c_str(), param.values.size());
	struct tm *t = TimeHelper::getDateTime();

	int curDate = t->tm_mon * 100 + t->tm_mday;
	int curTime = t->tm_hour * 100 + t->tm_min;
	int result = 0;
	for(int i = 0; i < param.values.size(); i++){
		JsonParse::Schedule schedule = param.values[i];
		int intStartDate = atoi(schedule.startDate.c_str());
		int intEndDate = atoi(schedule.endDate.c_str());
		int intStartTime = atoi(schedule.startTime.c_str());
		int intEndTime = atoi(schedule.endTime.c_str());
		LOGD("getValueByTime intStartDate=%d  intEndDate=%d intStartTime=%d intEndTime=%d\n", intStartDate, intEndDate, intStartTime, intEndTime);
		// 1.仅没有开始日期 2.仅没有截止日期 3.在对应的日期时间内
		if (intStartDate <= curDate && ( intEndDate >= curDate || intEndDate < 1)) {
			LOGD("在规定的时间内\n");
			if (intStartTime <= curTime && intEndTime >= curTime) {
				result = 1;
				return schedule.content;
			}else if(intStartTime < 1 && intEndTime < 1){
				LOGD("没有时间限制\n");
				result = 2;
				return schedule.content;
			}else if(intStartTime < 1){
				LOGD("没有开始时间限制，限制了结束时间\n");
				if(curDate >= intStartTime){
					return schedule.content;
				}else{
					result = 3;
				}
			}else if(intEndTime < 1 ){
				LOGD("没有结束时间限制，限制了开始时间\n");
				if(curDate <= intEndTime){
					result = 4;
					return schedule.content;
				}else{
					result = 5;
				}
			}else{
				LOGD("有时间限制,但不在规定时间内\n");
				result = 6;
			}
		}
		//1.既没有开始日期也没有截止日期，即没有日期限制
		else if (intStartDate < 1 && intEndDate < 1) {
			//有时间限制
			LOGD("有时间限制\n");
			if (intStartTime > 0 && intEndTime > 0) {
				if (intStartTime <= curTime && intEndTime >= curTime) {
					result = 7;
					return schedule.content;
				}else {
					result = 8;
				}
			}
			//限制了结束时间
			else if (intStartTime < 1 && intEndTime > 0) {
				LOGD("限制了结束时间\n");
				if (intEndTime >= curTime) {
					result = 9;
					return schedule.content;
				}else{
					result = 10;
				}
			}
			//限制了开始时间
			else if (intStartTime > 0 && intEndTime < 1) {
				LOGD("限制了开始时间\n");
				if (intStartTime <= curTime) {
					result = 11;
					return schedule.content;
				}else{
					result = 12;
				}
			}
			//没有时间限制
			else if(intEndTime < 1 && intStartTime < 1){
				result = 13;
				LOGD("没有时间限制\n");
				return schedule.content;
			}else{//其他情况
				LOGD("其他情况\n");
				result = 14;
			}
		}
	}
	return NULL;
}


#endif /* JNI_MODEL_DEVICEPARAMSCONTROL_CPP_ */
