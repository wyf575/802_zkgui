/*
 * Extend.h
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_EXTEND_H_
#define JNI_MODEL_EXTEND_H_

#include "json/json.h"

class Extend{
public:
	Extend(Json::Value extend);
	int getCmd();
	std::string getCmd_Id();
	int getDigital();
	std::string getMsg();
	int getSize();
	int getCount();
	std::string getName();
	bool isReplenish();

protected:
	Json::Value input;
	int size;
	bool mReplenish;
	std::string nickname;
	int count;
	int cmd;
	std::string cmd_id;
	int digital;
	std::string msg;
};


#endif /* JNI_MODEL_EXTEND_H_ */
