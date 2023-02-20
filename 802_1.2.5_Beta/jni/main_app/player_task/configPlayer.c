#include "configPlayer.h"
#include "../ibeelink_app/ibeelink_http.h"
#include "../main_app.h"
#include "../cJSON_file/read_json_file.h"

#define TMP_DATA_MAX_LEN 1024

static configParam sg_config = {"http://www.ibeelink.com",0,50,120,0,0};

static void configRAM2File(void)
{
    printf("configRAM2File[%s][%d][%d][%d][%d][%d]\n",sg_config.qr_link,sg_config.id,sg_config.qr_size,sg_config.vol,sg_config.QRCodeDisplay,sg_config.debugTime);
    save_pairs_file("qr_link",sg_config.qr_link,CONFIG_FILE_NAME);
    save_pairs_fileV2("id",&sg_config.id,INT_TYPE,CONFIG_FILE_NAME);
    save_pairs_fileV2("vol",&sg_config.vol,INT_TYPE,CONFIG_FILE_NAME);
    save_pairs_fileV2("qr_size",&sg_config.qr_size,INT_TYPE,CONFIG_FILE_NAME);
    save_pairs_fileV2("QRCodeDisplay",&sg_config.QRCodeDisplay,INT_TYPE,CONFIG_FILE_NAME);
    save_pairs_fileV2("debugTime",&sg_config.debugTime,INT_TYPE,CONFIG_FILE_NAME);
}

static void configFile2RAM(void)
{
    read_pairs_file("qr_link",sg_config.qr_link,CONFIG_FILE_NAME);
    read_pairs_fileV2("id",&sg_config.id,INT_TYPE,CONFIG_FILE_NAME);
    read_pairs_fileV2("vol",&sg_config.vol,INT_TYPE,CONFIG_FILE_NAME);
    read_pairs_fileV2("qr_size",&sg_config.qr_size,INT_TYPE,CONFIG_FILE_NAME);
    read_pairs_fileV2("QRCodeDisplay",&sg_config.QRCodeDisplay,INT_TYPE,CONFIG_FILE_NAME);
    read_pairs_fileV2("debugTime",&sg_config.debugTime,INT_TYPE,CONFIG_FILE_NAME);
    printf("configFile2RAM[%s][%d][%d][%d][%d]\n",sg_config.qr_link,sg_config.id,sg_config.qr_size,sg_config.vol,sg_config.QRCodeDisplay,sg_config.debugTime);
}

/*
设置屏幕方向，如果屏幕方向有改变，更新配置后重启
input：90竖屏，0横屏
1、读取全部文件，2、检索有效字段，3、拷贝有效字段后全部数据
4、读取有效值，5、对比有效值，6、修改有效值。
*/
/*
static void SetScreenDirection(int value)
{
    int tmp_value;
    int i;
    char tmp_data[TMP_DATA_MAX_LEN];
    char tmp_data_two[TMP_DATA_MAX_LEN];
    int read_len;
    char valid_data[3];
    int tmp_data_len,posion_one,posion_two,copy_len;
    //新优化代码
    char *ret_str = NULL;
    char *ret_char = NULL;
    char tmp_char;

    if(value > 0) {tmp_value = 90;}
    else {tmp_value = 0;}
    printf("[SetScreenDirection] tmp_value:%d\n",tmp_value);

    memset(tmp_data,TMP_DATA_MAX_LEN);
    memset(tmp_data_two,TMP_DATA_MAX_LEN);
    fp = fopen("/etc/EasyUI.cfg","r+");
	if(fp == NULL)
    {
		printf("open EasyUI.cfg fail\n");
		return;
	}
    read_len = fread(tmp_data, TMP_DATA_MAX_LEN, 1, fp);
    ret_str = strstr(tmp_data, "rotateScreen");
    if(NULL != ret_str)
    {
        ret_char = strchr(ret_str, ',');//将被检索信息以后的部分烤出来
        read_len = strlen(ret_char);
        memcpy(tmp_data_two, ret_char, read_len);

        //读取有效值
        memset(valid_data,3);
        tmp_char = *(ret_str + 14);

        if((tmp_char >= '0') && (tmp_char <= '9'))
        {
            valid_data[0] = tmp_char;
            tmp_char = ret_str + 15;
            if((tmp_char >= '0') && (tmp_char <= '9'))
            {
                valid_data[1] = tmp_char;
                valid_data[2] = '\n';
            }
            else
            {
                valid_data[1] = '\n';
            }
        }
        else
        {
            return;
        }

        copy_len = atoi(valid_data);


        posion_one = strchr(ret_str, ':');
        posion_two = strchr(tmp_data+posi_one, ',');
        if(posion_two > posion_one)
        {
            copy_len = posion_two - posion_one - 1;
            printf("[SetScreenDirection] copy_len:%d\n",copy_len);
        }
        else
        {
            printf("EasyUI.cfg err\n");
            return;
        }
    }


    fseek(fp,0,SEEK_SET);
    for(i = 0; i < 15; i++)
    {
        fseek(fp,0,SEEK_CUR);
        tmp_data_len = 0;
        
        fgets(tmp_data, TMP_DATA_MAX_LEN - 1, fp);
        tmp_data_len = strlen(tmp_data);
        if(tmp_data_len > 10)
        {
            if(NULL != strstr(tmp_data, "rotateScreen"))
            {
                memset(valid_data,8);
                posion_one = strchr(tmp_data, ':');
                posion_two = strchr(tmp_data, ',');
                if(posion_two > posion_one)
                {
                    copy_len = posion_two - posion_one - 1;
                    printf("[SetScreenDirection] copy_len:%d\n",copy_len);
                }
                else
                {
                    printf("EasyUI.cfg err\n");
                    return;
                }
                memcpy(valid_data, tmp_data, copy_len);
                copy_len = atoi(valid_data);
                printf("[SetScreenDirection] turn data:%d\n",copy_len);

                if(tmp_value != copy_len)
                {
                    sprintf(valid_data+posion_one,":%d,\n",tmp_value);
                    printf("[SetScreenDirection] valid_data:%s\n",valid_data);

                    //system("sync");
                    //system("reboot");
                }
            }
        }
    }
    

}
*/

static void parseValueItem(cJSON *item,char *key)
{
    if (strstr(key,"qr_link"))
    {
        cJSON *value = cJSON_GetObjectItem(item,"value");
        cJSON *valueItem = cJSON_GetArrayItem(value, 0);
        parse_json_table(valueItem,J_String,"content",sg_config.qr_link);
    }else if (strstr(key,"vol"))
    {
        cJSON *value = cJSON_GetObjectItem(item,"value");
        cJSON *valueItem = cJSON_GetArrayItem(value, 0);
        parse_json_table(valueItem,J_Int,"content",&sg_config.vol);
    }else if (strstr(key,"qr_size"))//二维码大小
    {
        cJSON *value = cJSON_GetObjectItem(item,"value");
        cJSON *valueItem = cJSON_GetArrayItem(value, 0);
        parse_json_table(valueItem,J_Int,"content",&sg_config.qr_size);
    }else if (strstr(key,"QRCodeDisplay"))
    {
        cJSON *value = cJSON_GetObjectItem(item,"value");
        cJSON *valueItem = cJSON_GetArrayItem(value, 0);
        parse_json_table(valueItem,J_Int,"content",&sg_config.QRCodeDisplay);
    }else if (strstr(key,"debugTime"))
    {
        cJSON *value = cJSON_GetObjectItem(item,"value");
        cJSON *valueItem = cJSON_GetArrayItem(value, 0);
        parse_json_table(valueItem,J_Int,"content",&sg_config.debugTime);
        getSetDebugTime(sg_config.debugTime);
    }
    else if(strstr(key,"ScreenDirection"))//横竖屏配置
    {
        int screen_direction;
        cJSON *value = cJSON_GetObjectItem(item,"value");
        cJSON *valueItem = cJSON_GetArrayItem(value, 0);
        parse_json_table(valueItem,J_Int,"ScreenDirection",&screen_direction);
        //SetScreenDirection(screen_direction);
    }else
    {
        // printf("参数组键名[%s]未匹配\n",key);
    }
}


/**
 * @brief 解析参数组配置
 * @caller 控制中心
 * @param param 配置相关的JSON
 * @attention 解析后需要立即存储到文件
 * @attention 解析后需要立即更新RAM参数
 * @return int 参数组ID号
 */
int parseConfigParams(cJSON *config)
{
    cJSON *params = cJSON_GetObjectItem(config,"params");
    parse_json_table(config,J_Int,"id",&sg_config.id);
    //1.逐个解析各参数组中的参数值
    for (size_t i = 0; i < cJSON_GetArraySize(params); i++)
    {
        char key[50] = {0};
        cJSON *item = cJSON_GetArrayItem(params, i);
        parse_json_table(item,J_String,"key",key);
        parseValueItem(item,key);
    }
    //2.因为网络更新不是频繁操作的行为 这里直接存储到文件即可
    configRAM2File();
    return sg_config.id;
}

/**
 * @brief Get the Set Debug Time object
 *  设置BA407SS需要的调试超时时间
 * @param setNum 0：不设置(只是获取) 其他值：设置超时时间
 * @return unsigned char 
 */
unsigned char getSetDebugTime(int setTimeS)
{
    static unsigned char TimeS = 0;
    if (setTimeS>0)
    {
        TimeS = setTimeS;
    }
    printf("getSetDebugTime  setTimeS[%d] TimeS[%d]\n",setTimeS,TimeS);
    return TimeS;
}

/**
 * @brief 获取配置组参数的值
 * 
 * @return configParam 返回变量值
 */
configParam getConfigParams(void)
{
    static bool firstCall = true;
    if (true == firstCall)
    {
        firstCall = false;
        configFile2RAM();
    }
    return sg_config;
}
