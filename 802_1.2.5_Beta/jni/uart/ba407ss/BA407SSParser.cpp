/*
 * BA407SSParser.cpp
 *
 *  Created on: 2021年1月4日
 *      Author: Administrator
 */
#include "BA407SSParser.h"

#include "BA407SSCmd.h"
#include "model/ModelManager.h"
#include "baselib/Utils.h"
#include "../UartContext.h"
#include "utils/log.h"
#include <math.h>
#include "baselib/FileUtils.h"
#include "io/ioutil.h"

using namespace JsonParse;
using namespace std;

const unsigned char MSG_RESULT_HAVE_READ = 0x02;
const unsigned char MSG_RESULT_SUCCESS = 0x03;
const unsigned char MSG_RESULT_FAILED = 0x04;
const unsigned char MSG_RESULT_REFUSE = 0x05;
const unsigned char MSG_RESULT_CHECK_ERROR = 0x06;
const unsigned char MSG_RESULT_FORMAT_ERROR = 0x07;
const unsigned char MSG_RESULT_AUTHENTICATION_FAILED = 0x08;
const unsigned char CHILD_AGREEMENT_SHIPMENT = 0x01;
const unsigned char CHILD_AGREEMENT_GET_DEVICE_INFO = 0x06;//子协议：获取设备信息
const unsigned char CHILD_AGREEMENT_NOTIFICATION_UPGRADE = 0xA1;//子协议：通知升级

static bool isUpgradeFinish;

BA407SSParser::BA407SSParser(){
	isFirst = true;
	curSeq = 0;
	isUpgradeFinish = true;
}

BA407SSParser* BA407SSParser::getInstance(){
	static BA407SSParser mInstance;
	return &mInstance;
}

bool BA407SSParser::checkCargo(int digital) {
	char value = (char) ceil(digital / 16.0);
	if (childDeviceMap.find(value) != childDeviceMap.end()) {
		return true;
	} else {
		getAllChildAddr();
		char value = (char) ceil(digital / 16.0);
		if (childDeviceMap.find(value) != childDeviceMap.end()) {
			return true;
		} else {
			return false;
		}
	}
}

BA407SSParser::BA407SSExtend* BA407SSParser::getExtend(Json::Value extend){
	if(extend.isObject()){
		static BA407SSExtend mExtend;
		if(extend.isMember(CMD)){
			mExtend.cmd = extend[CMD].asInt();
		}
		if(extend.isMember(INPUT)){
			Json::Value input = extend[INPUT];
			if(input.isMember(DIGITAL)){
				mExtend.digital = input[DIGITAL].asInt();
			}else{
				mExtend.digital = 1;
			}
			if(input.isMember(MSG)){
				mExtend.msg = input[MSG].asString();
			}
			if(input.isMember(COUNT)){
				mExtend.count = input[COUNT].asInt();
			}else{
				mExtend.count = 1;
			}
			if(input.isMember(REPLENISH)){
				//mExtend.mReplenish = input[REPLENISH].asBool();
				mExtend.mReplenish = true;
			}else{
				mExtend.mReplenish = false;
			}
		}
		if(extend.isMember(CMD_ID)){
			mExtend.cmd_id = extend[CMD_ID].asString();
		}
		return &mExtend;
	}
	return NULL;
}

int parseData(BYTE* serialData, int len, std::vector<BYTE*>& list) {
	if (serialData) {
		for (int i = 0; i < len; ) {
			if (serialData[i] == 0xF0 && serialData[i + 7] == 0xF1) {
				static BYTE data[8]={0};
				for (int j = 0; j < 8; j++) {
					data[j] = serialData[i + j];
				}
				i = i + 8;
				list.push_back(data);

			} else if(serialData[i] == 0xF0 && serialData[i + 11] == 0xF1){
				static BYTE data[12]={0};
				for (int j = 0; j < 12; j++) {
					data[j] = serialData[i + j];
				}
				i = i + 12;
				list.push_back(data);
			}else{
				i++;
			}
		}
	}
	return list.size();
}

bool BA407SSParser::replenishment(std::vector<char> &list){

	bool busy = false;
	for (int addr = 1; addr <= 7; addr++) {
		int count = 0;
		while (count < 6) {

			if (!busy) {
				//发送补货指令
				unsigned char buffer[512] = {0};
				int len = BA407SSCMD->getReplenishmentCmd(addr, buffer);
				UARTCONTEXT->send485Msg(buffer, len);
			}

			unsigned char buffer[512] = {0};
			int len = getUartInfo(0x05, buffer, ARRLEN(buffer));

			if (buffer && buffer[1] == 0x05) {
				switch (buffer[5]) {
					case MSG_RESULT_HAVE_READ:
						LOGD("消息已读\n");
						busy = true;
						break;
					case MSG_RESULT_SUCCESS:
						LOGD("处理成功\n");
						list.push_back(addr);
						busy = false;
						break;
					case MSG_RESULT_FAILED:
						LOGD("处理失败\n");
						busy = false;
						break;
					case MSG_RESULT_REFUSE:
						LOGD("拒绝处理\n");
						busy = false;
						break;
					case MSG_RESULT_CHECK_ERROR:
						LOGD("校验错误\n");
						busy = false;
						break;
					case MSG_RESULT_FORMAT_ERROR:
						LOGD("格式错误\n");
						busy = false;
						break;
					case MSG_RESULT_AUTHENTICATION_FAILED:
						LOGD("鉴权失败\n");
						busy = false;
						break;
					default:
						LOGD("收到未知处理结果\n");
						busy = false;
						break;
				}
			}
			if (!busy) {
				break;
			}
			count++;
		}
	}
	if(list.size() > 0){
		return true;
	}
	return false;
}

int BA407SSParser::getSeq() {
	curSeq++;
	if (curSeq > 0xEF) {
		curSeq = 0x01;
	}
	LOGD("curSeq=%d\n", curSeq);
	return curSeq;
}

bool check407Data(BYTE* buffer, int bufferLen, int cmd, int indexCmd){
	if (bufferLen >= 8
			&& (buffer[0] == 0xF0)
			&& buffer[bufferLen - 1] == 0xF1
			&& buffer[indexCmd] == cmd) {
		return true;
	}
	return false;
}

int BA407SSParser::getUartInfo(int cmd, unsigned char* buffer, int bufferLen){
	return UARTCONTEXT->getUartDate(buffer, bufferLen, cmd, 1, check407Data);
}

void sendReceivedMsgToMCU(int motorNum, int tmpSeq) {
	unsigned char buffer[512] = {0};
	int count = 5;
	int len = 0;
	while (len < 8){
		len = BA407SSCMD->getReceiveDataCmd(tmpSeq, motorNum + 16, buffer);
		UARTCONTEXT->send485Msg(buffer, len);

		len = BA407SSPARSER->getUartInfo(CHILD_AGREEMENT_SHIPMENT, buffer, ARRLEN(buffer));

		count--;
		LOGD("sendReceivedMsgToMCU-----%d\n", count);
		if(count <= 0){
			break;
		}
	}
}

void BA407SSParser::shipment(BaseTask mBaseTask, JsonParse::DataState& mDataState){
	int tmpSeq = 0;
	bool isBusy = false;
	int count = 0;
	while (true) {
		if (count > 6) {
			if (isBusy) {
				mDataState.state = "timeout";
				mDataState.cargoWay = mBaseTask.mExtend->digital;
				mDataState.isResult = false;
				return;
			} else {
				mDataState.state = "Out_of_contact";
				mDataState.cargoWay = mBaseTask.mExtend->digital;
				mDataState.isResult = false;
				return;
			}
		}
		if (!isBusy) {
			tmpSeq = getSeq();
			unsigned char buffer[10] = {0};
			int len = BA407SSCMD->getShipmentCmd(tmpSeq, mBaseTask.mExtend->digital + 16, buffer);
			UARTCONTEXT->send485Msg(buffer, len);
		}
		unsigned char buffer[512] = {0};
		int len = getUartInfo(CHILD_AGREEMENT_SHIPMENT, buffer, ARRLEN(buffer));

		LOGD("shipment - getUartDate:");
		UTILS->printfCMD(buffer, len);

		std::vector<BYTE*> list;
		parseData(buffer, len, list);
		LOGD("shipment - getUartDate:%d tmpSeq:%d\n", list.size(), tmpSeq);
		for(BYTE* arr : list){
			LOGD("shipment - getUartDate:arr[1]=%d, arr[2]=%d, arr[5]=%d, arr=%d\n", arr[1], arr[2], arr[5], arr);
			UTILS->printfCMD(arr, 8);
			if (arr[2] == tmpSeq && arr[1] == CHILD_AGREEMENT_SHIPMENT) {
				switch (arr[5]) {
					case MSG_RESULT_HAVE_READ:
						LOGD("消息已读\n");
						isBusy = true;
						break;
					case MSG_RESULT_SUCCESS:
						LOGD("处理成功\n");
						sendReceivedMsgToMCU(mBaseTask.mExtend->digital, tmpSeq);
						mDataState.state = "true";
						mDataState.cargoWay = mBaseTask.mExtend->digital;
						mDataState.isResult = true;
						return;
					case MSG_RESULT_FAILED:
						LOGD("处理失败\n");
						sendReceivedMsgToMCU(mBaseTask.mExtend->digital, tmpSeq);
						mDataState.state = "failed";
						mDataState.cargoWay = mBaseTask.mExtend->digital;
						mDataState.isResult = false;
						return;
					case MSG_RESULT_REFUSE:
						LOGD("拒绝处理\n");
						isBusy = true;
						break;
					case MSG_RESULT_CHECK_ERROR:
						LOGD("校验错误\n");
						mDataState.state = "校验错误";
						mDataState.cargoWay = mBaseTask.mExtend->digital;
						mDataState.isResult = false;
						return;
					case MSG_RESULT_FORMAT_ERROR:
						LOGD("格式错误\n");
						mDataState.state = "格式错误";
						mDataState.cargoWay = mBaseTask.mExtend->digital;
						mDataState.isResult = false;
						return;
					case MSG_RESULT_AUTHENTICATION_FAILED:
						LOGD("鉴权失败\n");
						mDataState.state = "鉴权失败";
						mDataState.cargoWay = mBaseTask.mExtend->digital;
						mDataState.isResult = false;
						return;
					default:
						LOGD("收到未知处理结果\n");
						mDataState.state = "收到未知处理结果";
						mDataState.cargoWay = mBaseTask.mExtend->digital;
						mDataState.isResult = false;
						return;
				}
			}
		}
		count++;
	}
}

//解析3812消息
void BA407SSParser::parseMsg(Json::Value extend){
	LOGD("----------parseMsg--------\n");
	BA407SSExtend* mExtend = getExtend(extend);
	if(!mExtend){
		return;
	}
	MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(mExtend->cmd, mExtend->cmd_id, 0, 1, "wait", true));

	if(mExtend->mReplenish){
		std::vector<char> successList;
		bool result = replenishment(successList);
		MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(extend, DataState(result)));
		return;
	}

	if(taskQueue.size() > 16){
		MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(extend, DataState("overflow", mExtend->digital, false)));
	}else{
		if(checkCargo(mExtend->digital)){
			LOGD("----------parseMsg--------push\n");
			BaseTask task;
			task.mExtend = mExtend;
			task.mExtendObject = extend;
			taskQueue.push(task);
		}else{
			MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(extend, DataState("overstep", mExtend->digital, false)));
		}
	}
}

map<char, BA407SSParser::Board> BA407SSParser::getChildDeviceMap(){
	return childDeviceMap;
}

//查询所有从站信息
map<char, BA407SSParser::Board> BA407SSParser::getAllChildAddr() {
	childDeviceMap.clear();
	for (int addr = 1; addr <= 7; addr++) {
		unsigned char buffer[512] = {0};
		int len = BA407SSCMD->getPollingCmd(getSeq(), addr, buffer);
		UARTCONTEXT->send485Msg(buffer, len);

		len = getUartInfo(CHILD_AGREEMENT_GET_DEVICE_INFO, buffer, ARRLEN(buffer));
		UTILS->printfCMD(buffer, len);
		if (len > 8 && childDeviceMap.find(buffer[5]) == childDeviceMap.end() && buffer[1] == CHILD_AGREEMENT_GET_DEVICE_INFO) {
			char version[7];
			sprintf(version, "V%d.%d.%d", buffer[6], buffer[7], buffer[8]);
			int deviceType = buffer[10] & 0b00001111;
			int controlType = (buffer[10] >> 4) & 0b00001111;
			Board mBoard;
			mBoard.version = version;
			mBoard.deviceType = deviceType;
			mBoard.controlType = controlType;

			childDeviceMap.insert(map<char, Board>::value_type(buffer[5], mBoard));
			LOGD("从站:%d ,version:%s, deviceType=%d, controlType= %d,  array[10]=%d\n", buffer[5], version, deviceType, controlType, buffer[10]);
		}
	}
	LOGD("已查询到 %d 个从站\n", childDeviceMap.size());
	return childDeviceMap;
}

//获取从站最小版本号
string BA407SSParser::getMinDeviceVersion() {
	if (childDeviceMap.empty()) {
		getAllChildAddr();
	}
	if (!childDeviceMap.empty()) {
		map<char, Board>::iterator iter;
		Board* mBoard = NULL;
		for (iter = childDeviceMap.begin();iter != childDeviceMap.end(); iter++){
			//cout << iter->first << "-" << iter->second << endl;
			if(mBoard == NULL){
				mBoard = &iter->second;
			}else{
				if(mBoard->version.compare(iter->second.version) < 0){
					mBoard = &iter->second;
				}
			}
		}
		return mBoard->version;
	}
	return "";
}

int BA407SSParser::notificationUpgrade(vector<char> needUpgradeList){
	int count = 0;
	for(char addr : needUpgradeList){
		BYTE outData[8]={0};
		int len = BA407SSCMD->getNotificationCmd(addr, outData);
		UARTCONTEXT->send485Msg(outData, len);

		unsigned char buffer[512] = {0};
		len = getUartInfo(CHILD_AGREEMENT_NOTIFICATION_UPGRADE, buffer, ARRLEN(buffer));
		if (len > 6 && buffer[5] == (char) 0xB2 && addr == buffer[4]) {
			count++;
		}
	}
	return count;
}

void BA407SSParser::doTaskforThread(){
	//LOGD("----------doTaskforThread-----------\n");

	if(isFirst){
		isFirst = false;
		sleep(1);
		getAllChildAddr();
		upgradeBA407SS();
	}
	if(isUpgradeFinish && !taskQueue.empty()){
		BaseTask mBaseTask = taskQueue.front();
		taskQueue.pop();

		DataState mDataState;
		shipment(mBaseTask, mDataState);
		MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(mBaseTask.mExtendObject, mDataState));
	}
}

void BA407SSParser::sendDataUpgradeToAll(string path){
	ioutil::Reader r;
	int subpackageNum = 0;
	if (r.Open(path.c_str())) {
		char buf[512] = {0};
		while (true) {
			int n = r.Read(buf, sizeof(buf));
			if (n > 0) {
				//有读到内容,输出每一个字节
//				for (int i = 0; i < n; ++i) {
//					LOGD("%02x", buf[i]);
//				}
				subpackageNum++;
				 BYTE packageSize[2] = {0};
				 UTILS->IntToBytes(n, packageSize, 2);
				 int len = 14 + n;
				 BYTE outData[len] = {0};
				 len = BA407SSCMD->getSendUpgradeDataCmd(true, 0xFF, subpackageNum, packageSize, buf, n, outData);
				 UARTCONTEXT->send485Msg(outData, len);
				 Thread::sleep(1000);
			} else if (n == 0) {
				LOGD("读取文件结束\n");
				break;
			} else {
				LOGD("出错\n");
				break;
			}
	  }
	  r.Close();
	}

}

bool sendPkgCrc(unsigned char* bytes, char addr){
	for (int i = 0; i < 3; i++) {
		BYTE cmd[11] = {0};
		BA407SSCMD->getAllPkgCrcCmd(addr, bytes, cmd);
		UARTCONTEXT->send485Msg(cmd, 11);
		unsigned char buffer[512] = {0};
		int len = BA407SSPARSER->getUartInfo(0xA1, buffer, ARRLEN(buffer));
		if (len > 6 && buffer[5] == (char) 0xB8 && addr == buffer[4]) {
			return true;
		} else {
			continue;
		}
	}
	return false;
}

void sendAllPkgCrc(unsigned char* bytes, vector<char> * list){
	if(list && list->size() > 0){
		LOGD("sendAllPkgCrc\n");
		for(int i = 0; i < list->size(); i++){
			LOGD("sendAllPkgCrc1\n");
			sendPkgCrc(bytes, (*list)[i]);
			LOGD("sendAllPkgCrc2\n");
		}
	}else{
		for(int i = 1; i <= 7; i++){
			sendPkgCrc(bytes, i);
		}
	}

}

//查询升级结果
void queryUpgradeResult(vector<char>& successList) {
	for (char addr = 1; addr <= 7; addr++) {
		for (int i = 0; i < 2; i++) {
			BYTE cmd[8]={0};
			BA407SSCMD->getUpgradeResult(addr, cmd);
			UARTCONTEXT->send485Msg(cmd, 8);

			unsigned char buffer[512] = {0};
			int len = BA407SSPARSER->getUartInfo(0xA1, buffer, ARRLEN(buffer));
			if (len > 7 && buffer[5] == (char) 0xB6 && addr == buffer[4] && buffer[6] == 0) {
				successList.push_back(addr);
				LOGD("%d号站升级成功\n", addr);
				break;
			} else {
				continue;
			}
		}
	}
}

void BA407SSParser::getUpgradeFailedList(vector<char> successList, vector<char>& needUpgradeList){
	LOGD("getUpgradeFailedList %d-%d\n",successList.size(), needUpgradeList.size());
	if(successList.size() == childDeviceMap.size()){
		return;
	}
	if(childDeviceMap.size() > 0){
		for(auto &it : childDeviceMap){
			bool isSuccessed = false;
			for(int i = 0; i < successList.size(); i++){
				if(successList[i] == it.first){
					isSuccessed = true;
					break;
				}
			}
			if(!isSuccessed){
				needUpgradeList.push_back(it.first);
			}
		}
	}
}

bool BA407SSParser::sendDataUpgradeToOne(char addr, string path){

	ioutil::Reader r;
	int subpackageNum = 0;
	if (r.Open(path.c_str())) {
		char buf[512] = {0};
		while (true) {
			LOGD("sendDataUpgradeToOne start\n");
			Thread::sleep(1000);
			int n = r.Read(buf, sizeof(buf));
			if (n > 0) {
//				 //有读到内容,输出每一个字节
//				for (int i = 0; i < n; ++i) {
//					LOGD("%02x", buf[i]);
//				}
				subpackageNum++;
				BYTE packageSize[2]={0};
				UTILS->IntToBytes(n, packageSize, 2);
				int sendLen = 14 + n;

				 for(int i = 0; i < 3; i++){
					 BYTE outData[sendLen] = {0};
					 BA407SSCMD->getSendUpgradeDataCmd(false, addr, subpackageNum, packageSize, buf, n, outData);
					 UARTCONTEXT->send485Msg(outData, sendLen);
					 unsigned char buffer[512] = {0};
					 int len = getUartInfo(0xA1, buffer, ARRLEN(buffer));
					 if (len > 7 && addr == buffer[4] && (char) 0xB4 == buffer[5] && 0x00 == buffer[6]) {
						 LOGD("发送成功！\n");
						 break;
					 } else if(i == 2){
						 if(len > 7){
							 switch (buffer[6]) {
								 case 0:
									 LOGD("成功\n");
									 break;
								 case 1:
									 LOGD("校验错误\n");
									 r.Close();
									 return false;
								 case 3:
									 LOGD("往flash写入数据失败\n");
									 r.Close();
									 return false;
								 case 4:
									 LOGD("数据超长\n");
									 r.Close();
									 return false;
								 default:
									 LOGD("格式错误\n");
									 r.Close();
									 return false;
							 }
						 }else{
							 LOGD("未读到串口数据\n");
							 r.Close();
							 return false;
						 }
					 }
				 }
			} else if (n == 0) {
				LOGD("读取文件结束\n");
				break;
			} else {
				LOGD("出错\n");
				r.Close();
				return false;
			}
	  }
	  r.Close();
	}
	LOGD("结束-\n");
	return true;
}

void BA407SSParser::upgradeOne(vector<char> needUpgradeList, string path, unsigned char* crcBytes){
    for (int i = 0; i < needUpgradeList.size(); i++) {
        LOGD("升级从站  - %d\n", needUpgradeList[i]);
        LOGD("通知从站进入升级状态\n");
        bool isUpgradeState = false;
        for (int i = 0; i < 3; i++) {
        	BYTE outData[8]={0};
			int len = BA407SSCMD->getNotificationCmd(needUpgradeList[i], outData);
			UARTCONTEXT->send485Msg(outData, len);

			unsigned char buffer[512] = {0};
			len = getUartInfo(CHILD_AGREEMENT_NOTIFICATION_UPGRADE, buffer, ARRLEN(buffer));

            if (len > 6 && buffer[5] == (char) 0xB2 && needUpgradeList[i] == buffer[4]) {
                isUpgradeState = true;
                break;
            }
        }
        if (!isUpgradeState) {
            continue;
        }
        LOGD("下发分包\n");
        bool result = sendDataUpgradeToOne((char)needUpgradeList[i], path);
        LOGD("下发分包%d\n", result);
        if (!result) {
            continue;
        }
        LOGD("发送整包CRC\n");
        bool bo = sendPkgCrc(crcBytes, (char)needUpgradeList[i]);
        if (!bo) {
            continue;
        }
        LOGD("查询升级结果\n");
        for (int j = 0; j < 3; j++) {
        	BYTE cmd[8] = {0};
        	BA407SSCMD->getUpgradeResult(needUpgradeList[i], cmd);
            UARTCONTEXT->send485Msg(cmd, 8);

            unsigned char buffer[512] = {0};
            int len = getUartInfo(0xA1, buffer, ARRLEN(buffer));

            if (len > 7 && buffer[5] == (char) 0xB6 && needUpgradeList[i] == buffer[4] && buffer[6] == 0) {
                break;
            } else {
                continue;
            }
        }

    }
}

void * upgradeFileDownloadCallback(bool result, string path, string version){
	if(result){
		LOGD("升级文件下载成功！\n");
		map<char, BA407SSParser::Board> childDeviceMap = BA407SSPARSER->getChildDeviceMap();
		if(!childDeviceMap.empty()){
			vector<char> needUpgradeList;
			for(auto &it : childDeviceMap){
				LOGD("upgradeFileDownloadResult: %s - %s\n", version.c_str(), it.second.version.c_str());
				if(version.compare(it.second.version) > 0){
					needUpgradeList.push_back(it.first);
				}
			}

			unsigned int img_crc;
			LOGD("path.c_str()=%s\n", path.c_str());
			UTILS->calc_img_crc(path.c_str(), &img_crc);
			LOGD("fileCRC:%u  -  ", img_crc);
			unsigned char crcBytes[4] = {0};
			UTILS->IntToBytes(img_crc, crcBytes, 4);
			UTILS->printfCMD(crcBytes, 4);
			LOGD("needUpgradeList.size()=%d - %d\n", needUpgradeList.size(), childDeviceMap.size());
			isUpgradeFinish = false;
			if(needUpgradeList.size() == childDeviceMap.size()){
				//整体升级
				LOGD("\n通知从站进入升级状态\n");
				int count = BA407SSPARSER->notificationUpgrade(needUpgradeList);
				LOGD("count=%d\n", count);
				if(count <= 0){
					LOGD("没有从站进入升级状态\n");
					isUpgradeFinish = true;
					return NULL;
				}
				Thread::sleep(3000);
				LOGD("\n给所有从站发送升级数据\n");
				BA407SSPARSER->sendDataUpgradeToAll(path);
				LOGD("\n主站下发整包crc\n");
				sendAllPkgCrc(crcBytes, NULL);
				LOGD("\n查询升级结果\n");
				vector<char> successList;
				queryUpgradeResult(successList);
				needUpgradeList.clear();
				BA407SSPARSER->getUpgradeFailedList(successList, needUpgradeList);
			}
			LOGD("需要单个从站升级个数：%d\n", needUpgradeList.size());
			if(needUpgradeList.size() > 0){
				BA407SSPARSER->upgradeOne(needUpgradeList, path, crcBytes);
			}
			isUpgradeFinish = true;
			BA407SSPARSER->upgradeBA407SS();
		}else{
			LOGD("未挂载从站\n");
		}

	}else{
		LOGD("升级文件下载失败！\n");
	}
	return NULL;
}

void BA407SSParser::upgradeBA407SS(){
	LOGD("---upgradeBA407SS--\n");
	if(!childDeviceMap.empty()){
		static ModelManager::s_upgrade_info sUpgradeInfo;
		sUpgradeInfo.callback = upgradeFileDownloadCallback;
		sUpgradeInfo.type = "mcu";
		sUpgradeInfo.version = getMinDeviceVersion();
		if(sUpgradeInfo.version.size() > 0){
			sUpgradeInfo.path = FILEUTILS->getMCUUpgardePath();
			MODELMANAGER->startUpgrade(&sUpgradeInfo);
		}else{
			LOGD("未查到从站\n");
		}
	}
}


