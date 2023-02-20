/*
 * Sing7Parser.cpp
 *
 *  Created on: 2021年4月2日
 *      Author: Administrator
 */


#include "Sing7Parser.h"
#include "model/JsonParse.h"
#include "Sing7Cmd.h"
#include "../UartContext.h"
#include "Utils/Log.h"
#include "model/ModelManager.h"
Sing7Parser::Sing7Parser(){

}

Sing7Parser* Sing7Parser::getInstance(){
	static Sing7Parser mInstance;
	return &mInstance;
}

void Sing7Parser::parseSING73812Msg(Json::Value extend){
	LOGD("parseSING73812Msg start\n");
	Extend mExtend(extend);
	MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(1000, mExtend.getCmd_Id(), "wait", true));
	sing7taskQueue.push(mExtend);
	LOGD("parseSING73812Msg end\n");
}

void sing7shipment(int index){
	LOGD("sing7shipment start\n");
	BYTE outCmd[20] = {0};
	int len = SING7CMD->getShipmentCmd(0xA1, index, outCmd);
	LOGD("sing7shipment 1\n");
	UARTCONTEXT->send(outCmd, len);
}

void Sing7Parser::doTaskforSING7(){
	if(!sing7taskQueue.empty()){
		Extend mExtend = sing7taskQueue.front();
		sing7taskQueue.pop();

		sing7shipment(mExtend.getDigital());
		curCmd_Id = mExtend.getCmd_Id();
		isWorking = true;
		int time = 0;
		while(1){
			if(isWorking){
				time++;
				sleep(1);
				if(time > 15){
					MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(1000, curCmd_Id,"timeout", false));
					break;
				}
			}else{
				break;
			}
		}
		sleep(2);
	}
}

/**
 * 功能：解析协议
 * 参数：pData 协议数据，len 数据长度
 * 返回值：实际解析协议的长度
 */
int Sing7Parser::sing7ParseProtocol(const BYTE *pData, UINT len) {
	LOGD("sing7ParseProtocol=%s len=%d\n", pData, len);

	UINT remainLen = len;	// 剩余数据长度
	UINT frameLen = 0;	// 帧长度

	/**
	 * 以下部分需要根据协议格式进行相应的修改，解析出每一帧的数据
	 */
	while (remainLen >= 5) {
		// 找到一帧数据的数据头
		while ((remainLen >= 2) && ((pData[0] != 0x68) && (pData[0] != 0x42) && (pData[0] != 0x45))) {
			pData++;
			remainLen--;
			continue;
		}
		LOGD("remainLen=%d - pData[0]=%02x\n", remainLen, pData[0]);
		if (remainLen < 5) {
			break;
		}

		if(pData[0] == 0x68){//心跳数据
			frameLen = 5;
		}else if(pData[0] == 0x45 && pData[1] == 0x45){//故障
			frameLen = 10;
		}else if(pData[0] == 0x42 && (pData[1] == 0x42 || pData[1] == 0x32 || pData[1] == 0x31)){//出货回复
			if(pData[4] == 0x46 && pData[5] == 0x45){
				frameLen = 6;
			}else if(pData[8] == 0x46 && pData[9] == 0x45){
				frameLen = 10;
			}else{
				frameLen = 26;
			}
		}
		LOGD("frameLen=%d remainLen=%d\n", frameLen, remainLen);
		if (frameLen > remainLen) {
			// 数据内容不全
			break;
		}

		// 解析一帧数据
		if(pData[0] == 0x68 && pData[frameLen - 1] == 0x74){//心跳
			LOGD("心跳\n");
		}else if(pData[0] == 0x42
				&& ((pData[1] == 0x42) || pData[1] == 0x31 || pData[1] == 0x32)
				&& pData[frameLen - 1] == 0x45 && pData[frameLen - 2] == 0x46){//收到出货数据
			if(frameLen == 6){
				if(pData[3] == 0x31){
					LOGD("电机空闲\n");
				}else if(pData[3] == 0x39){
					LOGD("设备出货中，稍后重发送\n");
					MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(1000, curCmd_Id, "running", false));
					isWorking = false;
				}
			}else if(frameLen == 26){
				if(pData[5] == 0x30){//出货成功
					LOGD("出货成功\n");
					MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(1000, curCmd_Id, "true", true));
				}else{//失败
					LOGD("失败\n");
					MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(1000, curCmd_Id,"false", false));
				}
				isWorking = false;
			}
		}else if(pData[0] == 0x45 && pData[1] == 0x45 && pData[frameLen - 1] == 0x44 && pData[frameLen - 2] == 0x44){//货道机故障
			LOGD("货道机故障\n");
			MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(1000, curCmd_Id, "MOTOR", false));
			isWorking = false;
		}

		pData += frameLen;
		remainLen -= frameLen;
	}

	return len - remainLen;

}
