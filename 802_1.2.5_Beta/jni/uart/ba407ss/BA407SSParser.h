/*
 * BA407SSParser.h
 *
 *  Created on: 2021年1月4日
 *      Author: Administrator
 */

#ifndef JNI_UART_BA407SSPARSER_H_
#define JNI_UART_BA407SSPARSER_H_

//#include "json/json.h"
//#include "uart/UartContext.h"
#include "model/JsonParse.h"
#include <map>
#include <queue>
#include <string>

using namespace std;

class BA407SSParser{

public:
	static BA407SSParser* getInstance();
	void parseMsg(Json::Value extend);
	bool replenishment(std::vector<char> &list);
	void doTaskforThread();

	typedef struct {
		int cmd;
		int digital;
		std::string msg;
		int count;
		bool mReplenish;
		std::string cmd_id;
	} BA407SSExtend;

	typedef struct{
		BA407SSExtend* mExtend;
		Json::Value mExtendObject;
	} BaseTask;

	typedef struct{
		std::string version;
		int controlType;
		int deviceType;
	} Board;

	void shipment(BaseTask mBaseTask, JsonParse::DataState& mDataState);
	map<char, Board> getAllChildAddr();
	string getMinDeviceVersion();
	void upgradeBA407SS();
	int notificationUpgrade(vector<char> needUpgradeList);
	void sendDataUpgradeToAll(string path);
	void getUpgradeFailedList(vector<char> successList, vector<char>& needUpgradeList);
	void upgradeOne(vector<char> needUpgradeList, string path, unsigned char* crcBytes);
	map<char, BA407SSParser::Board> getChildDeviceMap();
	bool sendDataUpgradeToOne(char addr, string path);
	int getUartInfo(int cmd, unsigned char* buffer, int bufferLen);

private:
	BA407SSParser();
	int getCSValue(const unsigned char *pData, int len);
	BA407SSExtend* getExtend(Json::Value extend);
	bool checkCargo(int digital);
	bool isFirst;
	int curSeq;
	int getSeq();
	queue<BaseTask> taskQueue;
	map<char, Board> childDeviceMap;
};

#define BA407SSPARSER 	BA407SSParser::getInstance()
#endif /* JNI_UART_BA407SSPARSER_H_ */
