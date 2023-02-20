#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

typedef enum{
    TYPE_NONE  = 0,
    GET_ICCID_STR,     /*用于获取模块信号值*/
    GET_GSM_LAC_CI,     /*用于获取基站区域码和基站编号*/
}recv_AT_type;

typedef struct 
{
    unsigned int urcStatus;
    unsigned int registerStat;
    unsigned long lacInfo;
    unsigned long ciInfo;
}creg_AT_info;

int UnGetCsqValue(void);
const char *UnGetDeviceId(void);
const char *UnGetMD5DeviceId(void);
const char *UnGetICCId(void);
const char *UnGetLBSInfo(void);
int UnGetCsqLevel(void);
const char *get_popularize_str(void);

//honestar karl.hong add 20210918
int UnIsDeviceWorking(void);
//honestar karl.hong add 20210918

#define NEED_DEVICE_SN 1

#if NEED_DEVICE_SN
const char *UnGetDeviceSN(void);
#endif 

#endif // !DEVICE_INFO_H
