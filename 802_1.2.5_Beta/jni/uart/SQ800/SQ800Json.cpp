/*
 * SQ800Json.cpp
 *
 *  Created on: 2021年4月8日
 *      Author: Administrator
 */

#include "SQ800Json.h"
#include "json/json.h"
#include "model/JsonParse.h"

//const std::string RESULT = "result";
using namespace JsonParse;

SQ800Json::SQ800Json(){

}

SQ800Json* SQ800Json::getInstance(){
	static SQ800Json mInstance;
	return &mInstance;
}

std::string SQ800Json::buildResetResponse(std::string resetType, bool result){
	Json::Value root;
	Json::Value extend;
	Json::Value json;

	json[::RESULT] = result;
	json[JsonParse::MSG] = resetType;
	extend.append(json);
	root[JsonParse::EXTEND] = extend;
	return root.toStyledString();
}

std::string SQ800Json::buildCmdExceptionResponse(std::string key, int status){
	Json::Value root;
	root[CMD] = CMD_3816;

	Json::Value json;
	json[KEY] = key;
	json[STATUS] = status;

	root[EXTEND] = json;
	return root.toStyledString();
}

std::string SQ800Json::buildCmdExceptionResponse(map<string, int> exceptionMap){
	Json::Value root;
	Json::Value extend;

	root[CMD] = CMD_3816;

	for(map<string, int>::iterator it = exceptionMap.begin();it != exceptionMap.end(); it++){
		Json::Value json;
		json[KEY] = it->first;
		json[STATUS] = it->second;
		extend.append(json);
	}

	root[EXTEND] = extend;
	return root.toStyledString();
}
