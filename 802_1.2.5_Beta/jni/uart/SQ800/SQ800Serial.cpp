/*
 * SQ800Serial.cpp
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#include "SQ800Serial.h"
#include "SQ800Cmd.h"
#include "uart/CommDef.h"
#include "../UartContext.h"
#include "Utils/Log.h"
#include "storage/StoragePreferences.h"
#include "model/ModelManager.h"
#include "SQ800Json.h"
#include <map>
//#include <mutex>

#define  PAPERJAM_EXCEPTION			"paperjamException"
#define  KNIFE_EXCEPTION			"knifeException"
#define  NOPAPER_EXCEPTION			"noPaperException"
#define  MOTORTIMEOUT_EXCEPTION		"motorTimeoutException"

#define  NOTHING		"Nothing"
#define  CUTERROR		"CutError"
#define  STUCKING		"Stucking"
#define  MOTOR			"MOTOR"

#define  EXCEPTION_OK			1
#define  EXCEPTION_CANCEL		2

#define  MOTOR_TIMEOUT_COUNT		180 //180 * 10s


SQ800Serial::SQ800Serial(){
	iCurDevAddr = 0;
	isBusy = false;
	isFirstQuery = true;
	motorTimeoutCount = 0;
	isUpdateException = false;

	paperjamException = StoragePreferences::getBool(PAPERJAM_EXCEPTION, false);
	knifeException = StoragePreferences::getBool(KNIFE_EXCEPTION, false);
	noPaperException = StoragePreferences::getBool(NOPAPER_EXCEPTION, false);
	motorTimeoutException = StoragePreferences::getBool(MOTORTIMEOUT_EXCEPTION, false);
}

SQ800Serial* SQ800Serial::getInstance(){
	static SQ800Serial mInstance;
	return &mInstance;
}

bool checkSQ800UartData(BYTE* buffer, int bufferLen, int cmd, int indexCmd){
	LOGD("checkSQ800UartData-indexCmd=%d, cmd=%d\n", indexCmd, cmd);
	if(bufferLen >= 8
			&& buffer[0] == 0x1F
			&& buffer[1] == 0x0F
			&& (buffer[indexCmd] == cmd || buffer[indexCmd] == 0)
			&& SQ800CMD->checkCRC(buffer, bufferLen)){
		return true;
	}
	return false;
}

int SQ800Serial::ticketOut(int iLength, int type){
	LOGD("ticketOut.iLength=%d, type=%d\n", iLength, type);
	BYTE outCmd[10]={0};
	int len = SQ800CMD->getCutTicketCmd(iCurDevAddr, iLength, type, outCmd);
	UARTCONTEXT->send485Msg(outCmd, len);
	BYTE buffer[512]={0};
	len = UARTCONTEXT->getUartDate(buffer, 512, outCmd[3], 3, checkSQ800UartData);
	if(len > 6){
		if(buffer[3] == 0){
			return -1;
		}
		if(buffer[6] == RSLT_OUT_TICKET_NOPAPER){
			noPaperExceptionTrue();
		}
		return buffer[6];
	}
	return -1;
}

bool SQ800Serial::resetErr(int type){
	BYTE outCmd[10]={0};
	int len = SQ800CMD->getResetErrorCmd(iCurDevAddr, type, outCmd);
	UARTCONTEXT->send485Msg(outCmd, len);
	BYTE buffer[512]={0};
	len = UARTCONTEXT->getUartDate(buffer, 512, outCmd[3], 3, checkSQ800UartData);
	if(len > 6){
		switch (buffer[6]) {
			case RESET_ERR_OK:
				LOGD("重置刀头成功\n");
				return true;
			case RESET_ERR_FAIL:
				LOGD("重置刀头失败\n");
				break;
			default:
				break;
		}
	}
	return false;
}

bool SQ800Serial::resetKnife(){
	return resetErr(RESETTYPE_KNIFE_ERR);
}

bool SQ800Serial::resetPaperJam(){
	return resetErr(RESETTYPE_PAPER_JAM);
}

void SQ800Serial::knifeExceptionTrue(){
	StoragePreferences::putBool(KNIFE_EXCEPTION, true);
	MODELMANAGER->sendMsgToMqttService(SQ800JSON->buildCmdExceptionResponse(CUTERROR, EXCEPTION_OK));
	knifeException = true;
}

void SQ800Serial::paperjamExceptionTrue(){
	MODELMANAGER->sendMsgToMqttService(SQ800JSON->buildCmdExceptionResponse(STUCKING, EXCEPTION_OK));
	StoragePreferences::putBool(PAPERJAM_EXCEPTION, true);
	paperjamException = true;
}

void SQ800Serial::noPaperExceptionTrue(){
	MODELMANAGER->sendMsgToMqttService(SQ800JSON->buildCmdExceptionResponse(NOTHING, EXCEPTION_OK));
	StoragePreferences::putBool(NOPAPER_EXCEPTION, true);
	noPaperException = true;
}

void SQ800Serial::updateException(){
	isUpdateException = true;
	BYTE outCmd[10]={0};
	int len = SQ800CMD->getSensorStatusCmd(iCurDevAddr, 5, outCmd);
	UARTCONTEXT->send485Msg(outCmd, len);
	BYTE buffer[512]={0};
	len = UARTCONTEXT->getUartDate(buffer, 512, outCmd[3], 3, checkSQ800UartData);
	LOGD("updateException=%d\n", len);
	bool isSendException = false;
	if(len < 8){
		return;
	}
	map<string, int> exceptionMap;
	exceptionMap[NOTHING] = EXCEPTION_CANCEL;
	exceptionMap[STUCKING] = EXCEPTION_CANCEL;
	exceptionMap[CUTERROR] = EXCEPTION_CANCEL;
	exceptionMap[MOTOR] = EXCEPTION_CANCEL;

	if(len > 6){
		int state = buffer[6];
		switch (state) {
			case 0://正常状态
				LOGD("正常状态\n");
				if (noPaperException || knifeException || paperjamException) {
					isSendException = true;
				}
				if (noPaperException) {
					noPaperException = false;
					StoragePreferences::putBool(NOPAPER_EXCEPTION, false);
				}
				if (knifeException) {
					knifeException = false;
					StoragePreferences::putBool(KNIFE_EXCEPTION, false);
				}
				if (paperjamException) {
					paperjamException = false;
					StoragePreferences::putBool(PAPERJAM_EXCEPTION, false);
				}
				break;
			case 1://入纸口无票
			case 2://中间传感器无遮挡
				LOGD("无票-%d\n", noPaperException);
				exceptionMap[NOTHING] = EXCEPTION_OK;
				if(!noPaperException){
					isSendException = true;
					StoragePreferences::putBool(NOPAPER_EXCEPTION, true);
					noPaperException = true;
				}
				break;
			case 3://切刀位置出错
				LOGD("切刀位置出错\n");
				exceptionMap[CUTERROR] = EXCEPTION_OK;
				if(!knifeException){
					isSendException = true;
					StoragePreferences::putBool(KNIFE_EXCEPTION, true);
					knifeException = true;
				}
				break;
			case 4://处于卡纸状态
				LOGD("处于卡纸状态\n");
				exceptionMap[STUCKING] = EXCEPTION_OK;
				if(!paperjamException){
					isSendException = true;
					StoragePreferences::putBool(PAPERJAM_EXCEPTION, true);
					paperjamException = true;
				}
				break;
			default:
				break;
		}

		motorTimeoutCount = 0;
		if(motorTimeoutException){
			motorTimeoutException = false;
			isSendException = true;
			StoragePreferences::putBool(MOTORTIMEOUT_EXCEPTION, false);
		}
	}else{
		motorTimeoutCount++;
		if(motorTimeoutCount >= MOTOR_TIMEOUT_COUNT && !motorTimeoutException){
			StoragePreferences::putBool(MOTORTIMEOUT_EXCEPTION, true);
			exceptionMap[MOTOR] = EXCEPTION_OK;
			isSendException = true;
			motorTimeoutException = true;
		}
	}

	if(isSendException || isFirstQuery){
		MODELMANAGER->sendMsgToMqttService(SQ800JSON->buildCmdExceptionResponse(exceptionMap));
		if(isFirstQuery){
			isFirstQuery = false;
		}
	}
	isUpdateException = false;
}

void* taskUpdateException(){
	while(1){
		if(!SQ800SERIAL->isBusy){
			sleep(1);
			SQ800SERIAL->updateException();
		}
		sleep(10);
	}
}

void SQ800Serial::initException(){

	pthread_t cut_exception_thread;
	int ret = pthread_create(&cut_exception_thread, NULL, (void*)taskUpdateException, NULL);
}

void SQ800Serial::setBusyState(bool busy){
	isBusy = busy;
}

bool SQ800Serial::getIsUpdateException(){
	return isUpdateException;
}
