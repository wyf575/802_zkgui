/*
 * Module4GManager.cpp
 *
 *  Created on: 2021年3月6日
 *      Author: Administrator
 */

#include "../net/Module4GManager.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <vector>
#include "baselib/Utils.h"

static int uartId;
static bool isOpenUart;
static std::vector<std::string> atList = {"ATI\r\n", "AT+CGREG=2\r\n", "AT+CGREG?\r\n", "AT+CGSN\r\n", "AT+ICCID\r\n", "AT+CSQ\r\n"};
static int sig = 99;
static std::string imei = "";
static std::string iccid = "";
static std::string baseLoc = "";
static int netStatus = 0;

Module4GManager::Module4GManager(){
}

Module4GManager* Module4GManager::getInstance(){
	static Module4GManager mInstance;
	return &mInstance;
}
static void parseAT(std::string atString){
	std::vector<std::string> splitStrList;
	UTILS->SplitString(atString, splitStrList, "\r\n");
	for(std::string value : splitStrList){
		printf("parseAT - %s\n", value.c_str());
	}
	try{
		if(!splitStrList.empty() && splitStrList.size() >= 4){
			printf(" --- parseAT-----\n");
			std::string trimValue = splitStrList[0];
			UTILS->trimString(trimValue);
			printf(" --- parseAT-----%s\n", trimValue.c_str());
			if("AT+CSQ" == trimValue){
				sig = stoi(splitStrList[2].substr(6, 2));
				printf("sig - %d\n", sig);
			}else if("AT+CGSN" == trimValue){
				imei = splitStrList[2];
				printf("imei - %s\n", imei.c_str());
			}else if("AT+ICCID" == trimValue){
				iccid = splitStrList[2].substr(9);
				printf("iccid - %s\n", iccid.c_str());
			}else if("AT+CGREG?" == trimValue){
				std::vector<std::string> splitBaseLoc;
				UTILS->SplitString(splitStrList[2], splitBaseLoc, ",");
				if(splitBaseLoc.size() > 2){
					std::string lacStr = splitBaseLoc[2].substr(1, splitBaseLoc[2].length() - 2);
					int lac = UTILS->hex_to_decimal(lacStr.c_str(), lacStr.length());
					std::string cidStr = splitBaseLoc[3].substr(1, splitBaseLoc[3].length() - 2);
					int cid = UTILS->hex_to_decimal(cidStr.c_str(), cidStr.length());
					baseLoc = std::to_string(lac) + "." + std::to_string(cid);
					netStatus = atoi(splitBaseLoc[1].c_str());
					printf("----splitBaseLoc[1]---%s - %d\n", splitBaseLoc[1].c_str(), netStatus);
				}
				printf("baseLoc - %s\n", baseLoc.c_str());
			}
		}
	}catch (std::out_of_range &exc) {
		printf("out_of_range - %s - %d\n", exc.what(), __LINE__ );
	}
}

static void sendAT(std::string atStr){
	char readbuf[256];
	bool sendResult = MODULE4GMANAGER->sendUartData(atStr.c_str(), atStr.length());
	printf("sendAT %s - %d\n", atStr.c_str(), sendResult);
	if(sendResult){
		memset(readbuf, 0, sizeof(readbuf));
		for(int i = 0; i < 10; i++){
			int readNum = read(uartId, readbuf, sizeof(readbuf));
			if(readNum > 0){
				parseAT(readbuf);
				printf("loopReadUartData-%s\n", readbuf);
				break;
			}else{
				printf("loopReadUartData-未读到数据\n");
				usleep(50000);
				continue;
			}
		}
	}
}

static void *loopReadUartData(void){
	printf("--------------loopReadUartData-----------\n");
	bool openResult = false;
	while(!openResult){
		sleep(5);
		openResult = MODULE4GMANAGER->openUart("/dev/ttyUSB1", B115200);
		printf("openResult=%d uartId=%d\n", openResult, uartId);
	}

	for(std::string atStr : atList){
		sendAT(atStr);
	}

	return NULL;
}

bool Module4GManager::openUart(const char *pFileName, unsigned int baudRate){
		uartId = open(pFileName, O_RDWR|O_NOCTTY);

		if (uartId <= 0) {
			isOpenUart = false;
			return isOpenUart;
		} else {
			struct termios oldtio = { 0 };
			struct termios newtio = { 0 };
			tcgetattr(uartId, &oldtio);

			newtio.c_cflag = baudRate|CS8|CLOCAL|CREAD;
			newtio.c_iflag = 0;	// IGNPAR | ICRNL
			newtio.c_oflag = 0;
			newtio.c_lflag = 0;	// ICANON
			newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
			newtio.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */
			tcflush(uartId, TCIOFLUSH);
			tcsetattr(uartId, TCSANOW, &newtio);

			// 设置为非阻塞
			fcntl(uartId, F_SETFL, O_NONBLOCK);
			isOpenUart = true;
			return isOpenUart;
		}
}

bool Module4GManager::sendUartData(const unsigned char* mData, int len){
	if (!isOpenUart) {
		return false;
	}
	if (write(uartId, mData, len) != (int) len) {	// fail
		return false;
	}
	return true;
}

void Module4GManager::init(){
	pthread_t thread_read;
	pthread_create(&thread_read, NULL, (void*)loopReadUartData, NULL);
}

std::string Module4GManager::getIMEI(){
	return imei;
}

std::string Module4GManager::getICCID(){
	return iccid;
}

int Module4GManager::get4GSig(){
	if(isConnectedNet()){
		return sig;
	}else{
		return 99;
	}
}

std::string Module4GManager::getBaseLoc(){
	return baseLoc;
}

bool Module4GManager::isConnectedNet(){
	return netStatus == 1 || netStatus == 5;
}
