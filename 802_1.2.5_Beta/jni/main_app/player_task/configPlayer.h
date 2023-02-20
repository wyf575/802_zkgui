
#ifndef CONFIG_PLAYER_H
#define CONFIG_PLAYER_H

#include <stdbool.h>
#include "../cJSON_file/json_app.h"

typedef struct 
{
    char qr_link[256];
    int id;
    int vol;
    int qr_size;
    int QRCodeDisplay;
    int debugTime;
}configParam;

/**
 * @brief 解析参数组配置
 * @caller 控制中心
 * @param param 配置相关的JSON
 * @attention 解析后需要立即存储到文件
 * @attention 解析后需要立即更新RAM参数
 */
int parseConfigParams(cJSON *config);

/**
 * @brief Get the Set Debug Time object
 *  设置BA407SS需要的调试超时时间
 * @param setNum 0：不设置(只是获取) 其他值：设置超时时间
 * @return unsigned char 
 */
unsigned char getSetDebugTime(int setTimeS);

/**
 * @brief 获取配置组参数的值
 * 
 * @return configParam 返回变量值
 */
configParam getConfigParams(void);

#endif // !PLAYERCONFIG_H
