/*
 * SQ800Extend.h
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#ifndef JNI_UART_SQ800_SQ800EXTEND_H_
#define JNI_UART_SQ800_SQ800EXTEND_H_

#include "json/json.h"
#include "model/Extend.h"

class SQ800Extend :public Extend{
private:
    int resetErr;
    int number;
    int ticketSize;
    bool mIsInch;

public:
	SQ800Extend(Json::Value extend);
	int getTicketSize();
	bool isInch();
	int getNumber();
	int getResetErr();
};



#endif /* JNI_UART_SQ800_SQ800EXTEND_H_ */
