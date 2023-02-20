/*
 * SQ800Cmd.cpp
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#include "SQ800Cmd.h"

#include "baselib/Utils.h"
#include "utils/Log.h"

SQ800Cmd::SQ800Cmd(){

}

SQ800Cmd* SQ800Cmd::getInstance(){
	static SQ800Cmd mInstance;
	return &mInstance;
}

/**
 * 获取CRC32校验值
 *
 * @param data
 * @return
 */
int SQ800Cmd::crc16(BYTE* data, int dataLen, BYTE* outValue) {
	int crcInt = UTILS->CRC16_XMODEM_CUSTOM(data, dataLen - 2);
	BYTE crcByte[2] = {0};
	UTILS->IntToBytes(crcInt, crcByte, 2);
	memset(outValue, 0, dataLen*sizeof(BYTE));
	memcpy(outValue, data, dataLen * sizeof(BYTE));
	outValue[dataLen - 1] = crcByte[1];
	outValue[dataLen - 2] = crcByte[0];
	return dataLen;
}

bool SQ800Cmd::checkCRC(BYTE* rxByteArray, int len) {
	int crcInt = UTILS->CRC16_XMODEM_CUSTOM(rxByteArray, len - 2);
	LOGD("checkCRC=%d\n", crcInt);
	BYTE crcByte[2] = {0};
	UTILS->IntToBytes(crcInt, crcByte, 2);
	LOGD("checkCRC=%02x, %02x\n", crcByte[0], crcByte[1]);
	if(crcByte[0] != 0 && crcByte[1] != 0 && rxByteArray[len - 1] == crcByte[1] && rxByteArray[len - 2] == crcByte[0]){
		return true;
	}
	return false;
}

/**
 * 获取出票指令
 * @param addr 从站地址 0x00开始
 * @param len 票长
 * @param type 类型，英寸或毫米
 * @return
 */
int SQ800Cmd::getCutTicketCmd(int addr, int len, int type, BYTE* outCmd) {
	BYTE cmd[] = {0x1F, 0x0F, addr, 0x04, type, 0x01, len, 0x00, 0x00};
	crc16(cmd, 9, outCmd);
	return 9;
}

/**
 * 获取查询状态信息
 * @param addr 从站地址
 * @param typeState 如：SENSOR_STAT_PAPER_IN
 * @return
 */
int SQ800Cmd::getSensorStatusCmd(int addr, int typeState, BYTE* outCmd){
	BYTE cmd[] = {0x1F, 0x0F, addr, 0x01, typeState, 0x00, 0x00, 0x00};
	crc16(cmd, 8, outCmd);
	return 8;
}

int SQ800Cmd::getResetErrorCmd(int iCurDevAddr, int resetType, BYTE* outCmd) {
	BYTE cmd[] = {0x1F, 0x0F, iCurDevAddr, 0x05, resetType, 0x00, 0x00, 0x00};
	crc16(cmd, 8, outCmd);
	return 8;
}

