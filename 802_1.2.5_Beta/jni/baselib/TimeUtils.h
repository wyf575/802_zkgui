/*
 * TimeUtils.h
 *
 *  Created on: 2021年1月4日
 *      Author: Administrator
 */

#ifndef JNI_BASELIB_TIMEUTILS_H_
#define JNI_BASELIB_TIMEUTILS_H_

#include <string>
using namespace std;

class TimeUtils{

public:
	static TimeUtils* getInstance();
	int getDateNow();
	int getTimeNow();
	int convertTime(string time_h_m);
	int convertDate(string date_y_m_d);

private:
	TimeUtils();

};


#define TIMEUTILS  TimeUtils::getInstance()
#endif /* JNI_BASELIB_TIMEUTILS_H_ */
