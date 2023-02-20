/*
 * SQ800Serial.h
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#ifndef JNI_UART_SQ800_SQ800SERIAL_H_
#define JNI_UART_SQ800_SQ800SERIAL_H_

#define RSLT_OUT_TICKET_SUCC 	0
#define RSLT_OUT_TICKET_NOPAPER 	1
#define RSLT_OUT_TICKET_NOT_TAKEN 	2
#define RSLT_OUT_TICKET_KNIFE_ERR 	3
#define RSLT_OUT_TICKET_PAPERJAM 	4

class SQ800Serial{
private:
	int iCurDevAddr;
	SQ800Serial();
	bool resetErr(int type);

	int motorTimeoutCount;

	bool isFirstQuery;

	bool paperjamException;
	bool knifeException;
	bool noPaperException;
	bool motorTimeoutException;

	bool isUpdateException;

public:
	static SQ800Serial* getInstance();
	int ticketOut(int iLength, int type);
	bool resetKnife();
	bool resetPaperJam();
	void knifeExceptionTrue();
	void paperjamExceptionTrue();
	void noPaperExceptionTrue();
	void updateException();
	void initException();
	void setBusyState(bool busy);
	bool getIsUpdateException();
	bool isBusy;
};


#define SQ800SERIAL SQ800Serial::getInstance()
#endif /* JNI_UART_SQ800_SQ800SERIAL_H_ */
