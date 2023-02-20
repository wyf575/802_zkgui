/*
 * DeviceParamsControl.h
 *
 *  Created on: 2020年12月24日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_DEVICEPARAMSCONTROL_H_
#define JNI_MODEL_DEVICEPARAMSCONTROL_H_

#include "JsonParse.h"

class DeviceParamsControl{
private:
	DeviceParamsControl();
public:
	const std::string VOL="vol";
	const std::string QRCODEDISPLAY="QRCodeDisplay";
	const std::string REBOOTTIME="rebootTime";
//	const std::string NOTICE="notice";
	const std::string VERSION="version";
	const std::string RUNTIME="runTime";
	const std::string ISSHOWLEFTBOTTOMLABEL="isShowLeftBottomLabel";
	const std::string QRLINK="qr_link";
	const std::string QRTIP="qrTip";
	const std::string BGIMG="bgImg";
	const std::string CLOSESERIAL="closeSerial";
	const std::string ISPORTRAIT="isPortrait";
	const std::string ISUPLOADLOG="isUploadLog";
	const std::string NEWIMEI="newImei";
	const std::string QR_SIZE="qr_size";
	const std::string QR_TIP_COLOR="qr_tip_color";
	const std::string QR_LOCATION="qr_location";

	static DeviceParamsControl* getInstance();
	void initDeviceParams(JsonParse::Params params);
	int getParamId();
	int getVolPercent();
	std::string getValueByTime(JsonParse::Param param);
	bool isDisplayQRCode();
	string displayCustomQRCode();
	string getQrSize();

	typedef struct{
		int paramId;
		JsonParse::Param mQrLocation;
		JsonParse::Param newImei;
		JsonParse::Param closeSerial;
		JsonParse::Param isPortrait;
		JsonParse::Param isUploadLog;
		JsonParse::Param mBgImg;
		JsonParse::Param vol;
		JsonParse::Param mQRCodeDisplay;
		JsonParse::Param rebootTime;
		JsonParse::Param versionDisplay;
		JsonParse::Param runTime;
		JsonParse::Param leftBottomLabel;
		JsonParse::Param mQrLink;
		JsonParse::Param mQrTip;
		JsonParse::Param mQrSize;
		JsonParse::Param mQrTipColor;
	} s_userParam;
};


#define DEVICEPARAMSCONTROL 	DeviceParamsControl::getInstance()
#endif /* JNI_MODEL_DEVICEPARAMSCONTROL_H_ */
