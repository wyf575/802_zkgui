#ifndef IBEELINK_HTTP_H
#define IBEELINK_HTTP_H

#include "main_app/http_client/ghttp_client.h"

/**
 * @brief 与蜜连服务器HTTP交互业务API
 * @date 2021.06.29
 * @author aiden.xiang@ibeelink.com
 */
//蜜连服务器地址URL
#define IBEELINK_SERVER "mqtt.ibeelink.com"
//蜜连上报异常数据URL
#define ALARM_URL "http://" IBEELINK_SERVER "/api/internal/device/report/abnormal"
//蜜连上报数据记录URL
#define DATA_URL "http://" IBEELINK_SERVER "/api/internal/device/upload/data"
//蜜连拉取DTU配置参数URL
#define CONFIG_URL "http://" IBEELINK_SERVER "/api/config/down-params"
//蜜连获取服务器时间戳URL
#define TIME_URL "http://" IBEELINK_SERVER "/time"
//蜜连上报注册信息URL
#define REGISTER_URL "http://" IBEELINK_SERVER "/api/internal/device/register/%s"
//蜜连获取内部升级版本信息URL
#define OTA_URL "http://" IBEELINK_SERVER "/api/version/upgrade/did"
//蜜连获取内部设备SN信息URL
#define SN_URL "http://" IBEELINK_SERVER "/api/device/exchange/sn?deviceId=%s"
//蜜连获取广告设备配置参数和广告任务URL
#define GET_AD_AND_CONFIG_URL "http://" IBEELINK_SERVER "/api/ad/play/list-param?deviceId=%s&paramType=1"
//蜜连上报广告任务执行结果URL
#define REPORT_AD_URL "http://" IBEELINK_SERVER "/api/ad/report"
//蜜连上报参数配置执行结果URL
#define REPORT_CONFIG_URL "http://" IBEELINK_SERVER "/api/config/success"
//蜜连生成推广二维码规则URL
#define POPULARIZE_URL "http://" IBEELINK_SERVER "/popularize?sn=%s"

/**
 * @brief 获取及处理配置和广告任务
 * @attention http底层接口内存问题
 * @attention 调用者不用关心具体如何处理
 * @attention 处理后的结果由playControl.c处理
 */
void handleConfigAndAD(char *url);

/**
 * @brief 回复获取配置后的处理结果
 * @attention 由于平台限制 只能回复成功
 * @param id 获取成功的id号
 */
void replyConfigResult(int id);

/**
 * @brief 回复广告任务处理结果
 * @attention 包括成功、失败、删除等JSON数组
 * @param jsonStr 三个JSON数组组成的JSON字串
 * @example {\"success\":[380,381,333],\"failed\":[380,381,333],\"delete\":[380,381,333]}
 */
void replyADResult(char *jsonStr);

/*
1、拉取广告后并确认加入到RAM链表
2、拉取广告后并确认存在且更新RAM链表任务时间
3、拉取广告后并确认拒绝加入到RAM链表
4、拉取广告后并确认移除播放列表并删除本地文件
5、播放广告时检测到过期并移除RAM删除本地文件
*/

/**
 * @brief 下载广告任务文件
 * 
 * @param downURL 下载地址
 * @param fileName 文件唯一名称
 */
void downloadTaskFile(char *downURL,char *fileName);


/**
 * @brief 蜜连平台HTTP注册接口
 * 
 */
void ibeelink_register(char*json_str);

#endif // !IBEELINK_HTTP_H
