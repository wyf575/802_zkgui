/*
 * SQ800Extend.cpp
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#include "SQ800Extend.h"

const std::string IS_INCH = "is_inch";
const std::string TICKET_SIZE = "ticket_size";
const std::string NUMBER = "number";
const std::string RESET_ERR = "reset_err";
const std::string SIZE = "size";

SQ800Extend::SQ800Extend(Json::Value extend) : Extend(extend){
	ticketSize = 0;
	mIsInch = 0;
	number = 0;
	resetErr = false;
	if(input.isObject()){
		if(input.isMember(IS_INCH)){
			mIsInch = input[IS_INCH].asBool();
		}
		if(input.isMember(TICKET_SIZE)){
			ticketSize = input[TICKET_SIZE].asInt();
		}
		if(input.isMember(NUMBER)){
			number = input[NUMBER].asInt();
		}
		if(input.isMember(RESET_ERR)){
			resetErr = input[IS_INCH].asBool();
		}
	}
}

int SQ800Extend::getTicketSize(){
	if(size != 0){
		return size;
	}
	return ticketSize;
}

bool SQ800Extend::isInch(){
	return mIsInch;
}

int SQ800Extend::getNumber(){
	return number;
}

int SQ800Extend::getResetErr(){
	return resetErr;
}

