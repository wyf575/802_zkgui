/*
 * SQ800Cmd.h
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#ifndef JNI_UART_SQ800_SQ800CMD_H_
#define JNI_UART_SQ800_SQ800CMD_H_

#include "uart/CommDef.h"

#define TICKET_OUT_CAL_INCH 			1
#define TICKET_OUT_CAL_MILLIMETER  		2

#define RESETTYPE_KNIFE_ERR 	1
#define RESETTYPE_PAPER_JAM 	2

#define RESET_ERR_OK 		0
#define RESET_ERR_FAIL 		2

class SQ800Cmd{
private:
	SQ800Cmd();
public:
	static SQ800Cmd* getInstance();
	int crc16(BYTE* data, int dataLen, BYTE* outValue);
	bool checkCRC(BYTE* rxByteArray, int len);
	int getCutTicketCmd(int addr, int len, int type, BYTE* outCmd);
	int getSensorStatusCmd(int addr, int typeState, BYTE* outCmd);
	int getResetErrorCmd(int iCurDevAddr, int resetType, BYTE* outCmd);
};


#define SQ800CMD SQ800Cmd::getInstance()
#endif /* JNI_UART_SQ800_SQ800CMD_H_ */
