/*
 * SQ800Json.h
 *
 *  Created on: 2021年4月8日
 *      Author: Administrator
 */

#ifndef JNI_UART_SQ800_SQ800JSON_H_
#define JNI_UART_SQ800_SQ800JSON_H_

#include <string>
#include<map>

using namespace std;

class SQ800Json{
private:
	SQ800Json();
public:
	static SQ800Json* getInstance();
	std::string buildResetResponse(std::string resetType, bool result);
	std::string buildCmdExceptionResponse(std::string key, int status);
	std::string buildCmdExceptionResponse(map<string, int> exceptionMap);
};


#define SQ800JSON 	SQ800Json::getInstance()
#endif /* JNI_UART_SQ800_SQ800JSON_H_ */
