/*
 * Sing7Parser.h
 *
 *  Created on: 2021年4月2日
 *      Author: Administrator
 */

#ifndef JNI_UART_SING7_SING7PARSER_H_
#define JNI_UART_SING7_SING7PARSER_H_

#include "json/json.h"
#include <queue>
#include "uart/CommDef.h"
#include "model/Extend.h"

using namespace std;

class Sing7Parser{
private:
	Sing7Parser();
	queue<Extend> sing7taskQueue;
	bool isWorking;
	std::string curCmd_Id;
public:
	static Sing7Parser* getInstance();
	void parseSING73812Msg(Json::Value extend);
	void doTaskforSING7();
	int sing7ParseProtocol(const BYTE *pData, UINT len);
};


#define SING7PARSER 	Sing7Parser::getInstance()
#endif /* JNI_UART_SING7_SING7PARSER_H_ */
