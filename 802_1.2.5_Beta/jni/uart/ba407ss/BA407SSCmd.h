/*
 * BA407SSCMD.h
 *
 *  Created on: 2021年1月5日
 *      Author: Administrator
 */

#ifndef JNI_UART_BA407SS_BA407SSCMD_H_
#define JNI_UART_BA407SS_BA407SSCMD_H_

#include "uart/CommDef.h"
#include <string>
#include "baselib/Utils.h"
#include "utils/log.h"

class BA407SSCmd{

private:
	BA407SSCmd(){

	}
public:
	static BA407SSCmd* getInstance(){
		static BA407SSCmd mInstance;
		return &mInstance;
	}

	/**
	 * 获取CS校验值
	 * @param data
	 * @return
	 */
	BYTE getCSValue(BYTE* pData, int len){
		BYTE csResult = 0;
		for (int i = 1; i < len - 1; i++){
			csResult += pData[i];
		}
		csResult -= pData[3];
		csResult = ~csResult & 0x7F;
		return csResult;
	}

	/**
	 * 核对CS校验值
	 * @param byteMsg
	 * @return
	 */
	bool checkCS(BYTE* pData, int len) {
		BYTE csValue = getCSValue(pData, len);
		return ((char)csValue == pData[3]);
	}

	/**
	 * 获取主站轮询从站指令
	 * @param addr
	 * @return
	 */
	int getPollingCmd(int seq, int addr, BYTE* outData){
		BYTE mData[] = {0xF0, 0x06, seq, 0x00, addr, 0x01, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 获取通知进入boot开始升级指令
	 * @param addr
	 * @return
	 */
	int getNotificationCmd(int addr, BYTE* outData){
		BYTE mData[] = {0xF0, 0xA1, 0xFF, 0x00, addr, 0xB1, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 获取广告升级数据指令
	 * @param isAllChildDevice 是否广播所有子设备升级
	 * @param addr 子设备地址（给指定从站升级时有效）
	 * @param subpackageNum 分包编号
	 * @param packageLen 包长
	 * @param upgradeData 升级数据
	 * @return
	 */
	int getSendUpgradeDataCmd(bool isAllChildDevice, int addr, int subpackageNum, char* packageLen, char* upgradeData, int bufferLen, BYTE* outData){
		LOGD("getSendUpgradeDataCmd=%d - %d - ", subpackageNum, bufferLen);
		UTILS->printfCMD(upgradeData, bufferLen);
		int len = 14 + bufferLen;
		BYTE mData[len];
		mData[0] = (char) 0xF0;
		mData[1] = (char) 0xB1;
		mData[2] = (char) 0xFF;
		mData[3] = (char) 0x00;
		if (isAllChildDevice){
			mData[4] = (char) 0xFF;
			mData[5] = (char) 0xBB;
		}else{
			mData[4] = (char) addr;
			mData[5] = (char) 0xB3;
		}
		mData[6] = (char) subpackageNum;
		mData[7] = packageLen[0];
		mData[8] = packageLen[1];
		for(int i = 9; i < 9 + bufferLen; i++){
			mData[i] = upgradeData[(i - 9)];
		}
		unsigned int crcInt = UTILS->getcrc(upgradeData, bufferLen);
		LOGD("crcInt = %u\n", crcInt);
		BYTE crc32Value[4] = {0};
		UTILS->IntToBytes(crcInt, crc32Value, 4);
		mData[len - 5] = crc32Value[0];
		mData[len - 4] = crc32Value[1];
		mData[len - 3] = crc32Value[2];
		mData[len - 2] = crc32Value[3];
		mData[len - 1] = (char) 0xF1;

		mData[3] = getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 轮询升级结果
	 * @param addr 从站地址
	 * @return
	 */
	int getUpgradeResult(int addr, BYTE* outData){
		BYTE mData[] = {0xF0, 0xA1, 0xFF, 0x00, addr, 0xB5, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 主站下发整包crc
	 * @param addr
	 * @param crc
	 * @return
	 */
	int getAllPkgCrcCmd(int addr, char* crc, BYTE* outData){
		BYTE mData[] = {0xF0, 0xA1, 0xFF, 0x00, addr, 0xB7, crc[0], crc[1], crc[2], crc[3], 0xF1};
		int len = ARRLEN(mData);
		mData[3] = (char) getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 获取出货指令
	 * @param seq 唯一标示
	 * @param motorNum 电机号
	 * @return
	 */
	int getShipmentCmd(int seq, int motorNum, BYTE* outData){
		BYTE mData[] = {0xF0, 0x01, seq, 0x00, motorNum, 0x01, 0x00, 0xF1};
		int len = ARRLEN(mData);
		UTILS->printfCMD(mData, len);
		mData[3] = (BYTE) getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 获取补货指令
	 * @param addr 从站地址
	 * @return
	 */
	int getReplenishmentCmd(int addr, BYTE* outData){
		BYTE mData[] = {0xF0, 0x05, 0x00, 0x00, addr, 0x01, 0x00, 0xF1};
		int len = ARRLEN(mData);

		mData[3] = getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 获取回复收到数据指令
	 * @param seq
	 * @param motorNum
	 * @return
	 */
	int getReceiveDataCmd(int seq, int motorNum, BYTE* outData){
		BYTE mData[] = {0xF0, 0x01, seq, 0x00, motorNum, 0x09, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = (char) getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 获取打开LED灯指令
	 * @param seq 唯一标示
	 * @param addr 从站地址
	 * @return
	 */
	int getOpenLEDCmd(int seq, int addr, BYTE* outData){
		BYTE mData[] = {0xF0, 0x07, seq, 0x00, addr, 0x01, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = (char) getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 开柜指令
	 * @param seq
	 * @param addr
	 * @return
	 */
	int getOpenDoorCmd(int seq, int addr, BYTE* outData){
		BYTE mData[] = { 0xF0, 0x08, seq, 0x00, addr, 0x01, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = (char) getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 查询从站出货结果指令
	 * @param seq
	 * @param addr
	 * @return
	 */
	int getQueryCoinResultCmd(int seq, int addr, BYTE* outData) {
		BYTE mData[] = {0xF0, 0x09, seq, 0x00, addr, 0x01, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = (char) getCSValue(mData,len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}

	/**
	 * 回复收到投币结果
	 * @param seq
	 * @param addr
	 * @return
	 */
	int getReversionCoinResultCmd(int seq, int addr, BYTE* outData){
		BYTE mData[] = {0xF0, 0x09, seq, 0x00, addr, 0x09, 0x00, 0xF1};
		int len = ARRLEN(mData);
		mData[3] = (char) getCSValue(mData, len);
		memset(outData, 0, len*sizeof(BYTE));
		memcpy(outData, mData, len * sizeof(BYTE));
		return len;
	}
};

#define BA407SSCMD 	BA407SSCmd::getInstance()
#endif /* JNI_UART_BA407SS_BA407SSCMD_H_ */
