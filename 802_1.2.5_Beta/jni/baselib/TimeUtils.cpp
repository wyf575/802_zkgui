/*
 * TimeUtils.cpp
 *
 *  Created on: 2021年1月4日
 *      Author: Administrator
 */

#include "TimeUtils.h"
#include "utils/TimeHelper.h"
#include "baselib/Utils.h"
#include <string>
#include <vector>

TimeUtils::TimeUtils(){
}

TimeUtils* TimeUtils::getInstance(){
	static TimeUtils mInstance;
	return &mInstance;
}

int TimeUtils::getDateNow(){
	struct tm *t = TimeHelper::getDateTime();
	return t->tm_year * 10000 + (t->tm_mon + 1)* 100 + t->tm_mday;
}

int TimeUtils::getTimeNow(){
	struct tm *t = TimeHelper::getDateTime();
	return t->tm_hour * 100 + t->tm_min;
}

//转换日期为int
int TimeUtils::convertDate(std::string date_y_m_d) {
	if(date_y_m_d.length() <= 0){
		return 0;
	}
	vector<string> dates;
	UTILS->SplitString(date_y_m_d, dates, "-");

	int iData = 0;
	if (dates.size() == 3) {
		iData = stoi(dates[0]);
		iData = iData * 100 + stoi(dates[1]);
		iData = iData * 100 + stoi(dates[2]);

	} else {
		if (date_y_m_d.length() == 8) {
			iData = stoi(date_y_m_d);
		}

	}
	return iData;
}

//转换时间为int
int TimeUtils::convertTime(string time_h_m) {
	if(time_h_m.length() <= 0){
		return 0;
	}
	vector<string> time;
	UTILS->SplitString(time_h_m, time, "-");
	int iTime = 0;
	if (time.size() == 2) {
		iTime = stoi(time[0]);
		iTime = iTime * 100 + stoi(time[1]);


	} else {
		if (time_h_m.length() == 4) {
			iTime = stoi(time_h_m);
		}
	}
	return iTime;

}

