/*
 * Extend.cpp
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#include "Extend.h"
#include "model/JsonParse.h"

Extend::Extend(Json::Value extend){
	size = 0;
	if(extend.isObject()){
		if(extend.isMember(JsonParse::CMD)){
			cmd = extend[JsonParse::CMD].asInt();
		}
		if(extend.isMember(JsonParse::CMD_ID)){
			cmd_id = extend[JsonParse::CMD_ID].asString();
		}

		if(extend.isMember(JsonParse::INPUT)){
			input = extend[JsonParse::INPUT];
			if(input.isMember(JsonParse::DIGITAL)){
				digital = input[JsonParse::DIGITAL].asInt();
			}
			if(input.isMember(JsonParse::MSG)){
				msg = input[JsonParse::MSG].asString();
			}
			if(input.isMember(JsonParse::COUNT)){
				count = input[JsonParse::COUNT].asInt();
			}
			if(input.isMember(JsonParse::SIZE)){
				size = input[JsonParse::SIZE].asInt();
			}
			if(input.isMember(JsonParse::NICKNAME)){
				nickname = input[JsonParse::NICKNAME].asString();
			}
			if(input.isMember(JsonParse::REPLENISH)){
				mReplenish = true;
			}
		}
	}
}

int Extend::getCmd(){
	return cmd;
}

int Extend::getDigital(){
	return digital;
}

std::string Extend::getMsg(){
	return msg;
}

int Extend::getSize(){
	return size;
}

int Extend::getCount(){
	return count;
}

std::string Extend::getName(){
	return nickname;
}

bool Extend::isReplenish(){
	return mReplenish;
}

std::string Extend::getCmd_Id(){
	return cmd_id;
}
