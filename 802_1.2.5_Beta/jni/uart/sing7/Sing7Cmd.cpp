/*
 * Sing7Cmd.cpp
 *
 *  Created on: 2021年4月2日
 *      Author: Administrator
 */


#include "Sing7Cmd.h"
#include <cstring>

Sing7Cmd::Sing7Cmd(){

}

Sing7Cmd* Sing7Cmd::getInstance(){
	static Sing7Cmd mInstance;
	return &mInstance;
}

int Sing7Cmd::getShipmentCmd(BYTE type, BYTE num, BYTE* outCmd){
	orderId++;
	if(orderId > 0xFF){
		orderId = 0;
	}
	BYTE cmd[] = {type, 0x01, num, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, orderId, 0xFE};
	int len = ARRLEN(cmd);
	memset(outCmd, 0, len*sizeof(BYTE));
	memcpy(outCmd, cmd, len * sizeof(BYTE));
	return len;
}

