/*
 * Sing7Cmd.h
 *
 *  Created on: 2021年4月2日
 *      Author: Administrator
 */

#ifndef JNI_UART_SING7_SING7CMD_H_
#define JNI_UART_SING7_SING7CMD_H_

#include "uart/CommDef.h"

class Sing7Cmd{
private:
	Sing7Cmd();
	int orderId;
public:
	static Sing7Cmd* getInstance();
	int getShipmentCmd(BYTE type, BYTE num, BYTE* outCmd);
};

#define SING7CMD 	Sing7Cmd::getInstance()
#endif /* JNI_UART_SING7_SING7CMD_H_ */
