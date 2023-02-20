/*
 * SQ800Parser.h
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#ifndef JNI_UART_SQ800_SQ800PARSER_H_
#define JNI_UART_SQ800_SQ800PARSER_H_

#include "json/json.h"
#include <queue>
#include "SQ800Extend.h"
#include "model/JsonParse.h"

using namespace JsonParse;
using namespace std;

#define DEF_TICKET_OUT_LENGTH 200

class SQ800Parser{
private:
	SQ800Parser();
	queue<SQ800Extend> sq800taskQueue;
	bool isInitException;

public:
	static SQ800Parser* getInstance();
	void parseSQ8003812Msg(Json::Value extend);
	void doTaskforSQ800();
	int outTicket(int ticketSize, bool inch);
};

#define SQ800PARSER SQ800Parser::getInstance()
#endif /* JNI_UART_SQ800_SQ800PARSER_H_ */
