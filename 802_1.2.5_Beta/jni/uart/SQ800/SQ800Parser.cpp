/*
 * SQ800Parser.cpp
 *
 *  Created on: 2021年4月1日
 *      Author: Administrator
 */

#include "SQ800Parser.h"
#include "SQ800Extend.h"
#include "SQ800Serial.h"
#include "SQ800Cmd.h"
#include "SQ800Json.h"
#include "utils/Log.h"
#include "model/ModelManager.h"
#include "net/DeviceSigManager.h"

SQ800Parser::SQ800Parser(){
	isInitException = false;
}

SQ800Parser* SQ800Parser::getInstance(){
	static SQ800Parser mInstance;
	return &mInstance;
}

int SQ800Parser::outTicket(int ticketSize, bool inch) {
	LOGD("outTicket-ticketSize=%d inch=%d\n", ticketSize, inch);
	int ticketOutState = SQ800SERIAL->ticketOut(ticketSize == 0 ? DEF_TICKET_OUT_LENGTH : ticketSize,
			inch ? TICKET_OUT_CAL_INCH : TICKET_OUT_CAL_MILLIMETER);

	for(int i = 0; i < 3; i++){
		LOGD("ticketOutState=%d\n", ticketOutState);
		if (ticketOutState == RSLT_OUT_TICKET_KNIFE_ERR) {
			bool result = SQ800SERIAL->resetKnife();
			MODELMANAGER->sendMsgToMqttService(SQ800JSON->buildResetResponse("resetKnife", result));
			if(i == 2){
				SQ800SERIAL->knifeExceptionTrue();
			}
		} else if (ticketOutState == RSLT_OUT_TICKET_PAPERJAM) {
			bool result = SQ800SERIAL->resetPaperJam();
			MODELMANAGER->sendMsgToMqttService(SQ800JSON->buildResetResponse("resetPaperJam", result));
			if(i == 2){
				SQ800SERIAL->paperjamExceptionTrue();
			}
		}else if(ticketOutState == -1){
			LOGD("---------------\n");
		}else{
			break;
		}
		ticketOutState = SQ800SERIAL->ticketOut(ticketSize == 0 ? DEF_TICKET_OUT_LENGTH : ticketSize,
				inch ? TICKET_OUT_CAL_INCH : TICKET_OUT_CAL_MILLIMETER);
	}
	LOGD("ticketOutState=%d\n", ticketOutState);
	return ticketOutState;
}

void SQ800Parser::parseSQ8003812Msg(Json::Value extend){
	SQ800Extend mSQ800Extend(extend);
	MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(mSQ800Extend.getCmd(), mSQ800Extend.getCmd_Id(), 0, 1, "wait", true));
	if(sq800taskQueue.size() > 16){
		MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(extend, DataState("overflow", mSQ800Extend.getDigital(), false)));
	}else{
		sq800taskQueue.push(mSQ800Extend);
	}
}

void SQ800Parser::doTaskforSQ800(){
	if(!isInitException){
		if(JsonParse::getMqttConnectStatus()){
			isInitException = true;
			SQ800SERIAL->initException();
		}
	}
	if(!sq800taskQueue.empty() && !SQ800SERIAL->getIsUpdateException()){
		SQ800SERIAL->setBusyState(true);
		SQ800Extend mSQ800Extend = sq800taskQueue.front();
		sq800taskQueue.pop();

		for (int index = 1; index <= mSQ800Extend.getCount(); index++) {
			if(mSQ800Extend.getDigital() == 1){// 1货道口罩机
				sleep(1);
				int status = outTicket(mSQ800Extend.getTicketSize(), mSQ800Extend.isInch());
				bool result = false;
				std::string state = "DATA_FORMAT_ERROR";
				switch (status) {
					case RSLT_OUT_TICKET_SUCC:
						result = true;
						LOGD("出货成功\n");
						state = "true";
						break;
					case RSLT_OUT_TICKET_NOPAPER:
						state = "HAVE_NOTHING";
						break;
					case RSLT_OUT_TICKET_NOT_TAKEN:
						state = "BLACK_FLAG_ERROR";
						break;
					case RSLT_OUT_TICKET_KNIFE_ERR:
						state = "CUT_LOCAL_EEROR";
						break;
					case RSLT_OUT_TICKET_PAPERJAM:
						state = "STUCK_THING_EEROR";
						break;
					default:
						break;
				}
				MODELMANAGER->sendMsgToMqttService(JsonParse::buildCmdResponse(mSQ800Extend.getCmd(), mSQ800Extend.getCmd_Id(), index, mSQ800Extend.getCount(), state, result));
				if(status != RSLT_OUT_TICKET_SUCC){
					LOGD("出货失败\n");
				}
				sleep(1);
				SQ800SERIAL->updateException();
			}
		}
		SQ800SERIAL->setBusyState(false);
	}
}
