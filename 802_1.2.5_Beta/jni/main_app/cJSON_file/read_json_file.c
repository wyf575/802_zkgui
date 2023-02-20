/**
 * 注意该文件提供的存取json文件的功能较弱 只能操作一层键值对
 * 需要后期不断完善 并保证依赖环境为linux及cJSON
*/

#include<stdio.h>
#include<unistd.h>
#include"cJSON.h"
#include"read_json_file.h"
#include "../linux_tool/busy_box.h"


/**
* @brief         : 文件操作+JSON解析键值对
* @param[in]	 : 输入键值对及存储的文件名 没有会新建
* @param[out]    : None 需要cJSON库中所依赖的库--m库
* @return        : 成功返回0 其他为错误
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.08.30 
* @date          : 2019.10.23 
*/
int save_pairs_fileV2(const char *key,void *value, json_pairs_type type,const char *file)
{

    FILE *fp = NULL;
    cJSON * json = NULL;
    char *json_data = NULL;
    char file_tmp[JSON_PAIRS_FILE_SIZE];
   
    if(access(file,F_OK) == 0){
        fp = fopen(file,"r+");
    }else{
        printf("Cannot Access File:%s, will creat it\n",file);
    }

    if(fp == NULL){
        json = cJSON_CreateObject();
    }else{
    	fgets(file_tmp, JSON_PAIRS_FILE_SIZE ,fp);
	    if(fgets(file_tmp, JSON_PAIRS_FILE_SIZE ,fp) != NULL){
	        return 1;
	    }
	    fclose(fp);
	    json = cJSON_Parse(file_tmp);
        if(NULL == json){
            json = cJSON_CreateObject();
        }
    }
    
    if(NULL != json){
        if(cJSON_HasObjectItem(json,key)){
            cJSON_DeleteItemFromObject(json,key);
        }
        switch (type)
        {
        case CHAR_TYPE:
            cJSON_AddStringToObject(json,key,(char*)value);
            printf("Save char:[%s][%s]-[%s]\n",key,(char*)value,file);
            break;
        case INT_TYPE:
            cJSON_AddNumberToObject(json,key,*(int *)value);
            printf("Save int :[%s][%d]-[%s]\n",key,*(int *)value,file);
            break;
        case DOUBLE_TYPE:
            cJSON_AddNumberToObject(json,key,*(double *)value);
            printf("Save int :[%s][%lf]-[%s]\n",key,*(double *)value,file);
            break;
        default:
            printf("warning: can not find your value type!\n");
            break;
        }
    	
    }
    json_data = cJSON_PrintUnformatted(json);

    fp = NULL;
    fp = fopen(file,"w");
    if(NULL != fp && NULL != json_data){
        sprintf(file_tmp,"%s\n",json_data);
        fputs(json_data,fp);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }else{
        printf("save_pairs failed[%d][%s]\n",fp,json_data);
    }
    free(json_data);
    cJSON_Delete(json);
    set_file_all_authority(file);
    return 0;
}


/**
* @brief         : 文件操作+JSON解析键值对
* @param[in]	 : 键和其所在的文件
* @param[out]    : 所需要读取的键值 
* @return        : 成功返回0 其他为错误
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.08.30 
* @date          : 2019.10.23 
*/
int read_pairs_fileV2(const char *key,void *value, json_pairs_type type,const char *file)
{
    char *file_tmp;
    FILE *fp = NULL;
    cJSON * json = NULL;
    cJSON *node = NULL;
    file_tmp = (char *)malloc(JSON_PAIRS_FILE_SIZE);
    memset(file_tmp,0,JSON_PAIRS_FILE_SIZE);
    if(access(file,F_OK) == 0){/*文件存在*/
        fp = fopen(file,"r");
    }else{
        printf("Cannot Access File:%s\n",file);
    }
    if(fp == NULL){/*读取不成功*/
        free(file_tmp);
        printf("Cannot Open File:%s\n",file);
        return READ_CANNOT_FILE;
    }
    
    if(fgets(file_tmp, JSON_PAIRS_FILE_SIZE ,fp) == NULL)//失败或读到文件结尾返回NULL
    {
        fclose(fp);
        printf("Cannot Read File:[%s][%s]\n",file,file_tmp);
        free(file_tmp);
        return READ_CANNOT_READ;
    }
    fclose(fp);
    json = cJSON_Parse(file_tmp);//将普通的json串解析成json对象
    if (NULL == json)
    {
        user_remove_file(file);
    }
    free(file_tmp);

    if(cJSON_HasObjectItem(json,key))//判断有没有这一项
	{
    	node = cJSON_GetObjectItem(json,key);
        switch (type)
        {
        case CHAR_TYPE:
            strcpy((char *)value,node->valuestring);
            printf("Read:[%s][%s]\n",key,(char *)value);
            break;
        case INT_TYPE:
            *(int *)value = node->valueint;
            printf("Read:[%s][%d]\n", key, *(int *)value);
            break;
        case DOUBLE_TYPE:
            *(double *)value = node->valuedouble;
            printf("Read:[%s][%lf]\n", key, *(double *)value);
            break;
        default:
            printf("warning: can not find your value type!\n");
            break;
        }
    	
    	cJSON_Delete(json);
        
    }else{
    	cJSON_Delete(json);
    	return READ_CANNOT_KEY;
    }
    return READ_KEY_SUCCESS;
}


/**
* @brief         : 文件操作+JSON解析键值对
* @param[in]	 : 输入键值对及存储的文件名 没有会新建
* @param[out]    : None 需要cJSON库中所依赖的库--m库
* @return        : 成功返回0 其他为错误
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.08.30 
* @date          : 2021.07.21
*/
int save_pairs_fileV3(const char *key,void *value, json_pairs_type type,unsigned long valueLength,const char *file)
{
    FILE *fp = NULL;
    cJSON * json = NULL;
    char *json_data = NULL;
    char *file_tmp = (char *)malloc(valueLength);

    if(access(file,F_OK) == 0){
        fp = fopen(file,"r+");
    }else{
        printf("Cannot Access File:%s, will creat it\n",file);
    }

    if(fp == NULL){
        json = cJSON_CreateObject();
    }else{
    	fgets(file_tmp, valueLength ,fp);
	    if(fgets(file_tmp, valueLength ,fp) != NULL){
            free(file_tmp);
	        return 1;
	    }
	    fclose(fp);
	    json = cJSON_Parse(file_tmp);
        if(NULL == json){
            json = cJSON_CreateObject();
        }
    }
    
    if(NULL != json){
        if(cJSON_HasObjectItem(json,key)){
            cJSON_DeleteItemFromObject(json,key);
        }
        switch (type)
        {
        case CHAR_TYPE:
            cJSON_AddStringToObject(json,key,(char*)value);
            printf("Save char:[%s][%s]-[%s]\n",key,(char*)value,file);
            break;
        case INT_TYPE:
            cJSON_AddNumberToObject(json,key,*(int *)value);
            printf("Save int :[%s][%d]-[%s]\n",key,*(int *)value,file);
            break;
        case DOUBLE_TYPE:
            cJSON_AddNumberToObject(json,key,*(double *)value);
            printf("Save int :[%s][%lf]-[%s]\n",key,*(double *)value,file);
            break;
        default:
            printf("warning: can not find your value type!\n");
            break;
        }
    	
    }
    json_data = cJSON_PrintUnformatted(json);
    memset(file_tmp,0,valueLength);
    fp = NULL;
    fp = fopen(file,"w");
    if(NULL != fp && NULL != json_data){
        snprintf(file_tmp,valueLength,"%s\n",json_data);
        fputs(json_data,fp);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }else{
        printf("save_pairs failed[%d][%s]\n",fp,json_data);
    }
    free(json_data);
    free(file_tmp);
    cJSON_Delete(json);
    set_file_all_authority(file);
    return 0;
}

/**
* @brief         : 文件操作+JSON解析键值对
* @param[in]	 : 键和其所在的文件
* @param[out]    : 所需要读取的键值 
* @return        : 成功返回0 其他为错误
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.08.30 
* @date          : 2021.07.21
*/
int read_pairs_fileV3(const char *key,void *value, json_pairs_type type,long int valueLength,const char *file)
{
    FILE *fp = NULL;
    cJSON * json = NULL;
    cJSON *node = NULL;
    char *file_tmp = (char *)malloc(valueLength); 
    memset(file_tmp,0,valueLength);
    if(access(file,F_OK) == 0){/*文件存在*/
        fp = fopen(file,"r");
    }else{
        printf("Cannot Access File:%s\n",file);
    }
    if(fp == NULL){/*读取不成功*/
        free(file_tmp);
        printf("Cannot Open File:%s\n",file);
        return READ_CANNOT_FILE;
    }
    if(fgets(file_tmp, valueLength ,fp) == NULL)//失败或读到文件结尾返回NULL
    {
        fclose(fp);
        printf("Cannot Read File:[%s][%s]\n",file,file_tmp);
        free(file_tmp);
        return READ_CANNOT_READ;
    }
    fclose(fp);
    printf("YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY file_tmp[%s]\n",file_tmp);
    json = cJSON_Parse(file_tmp);//将普通的json串解析成json对象
    if (NULL == json)
    {
        user_remove_file(file);
    }
    free(file_tmp);

    if(cJSON_HasObjectItem(json,key))//判断有没有这一项
	{
    	node = cJSON_GetObjectItem(json,key);
        switch (type)
        {
        case CHAR_TYPE:
            strncpy((char *)value,node->valuestring,valueLength);
            printf("Read:[%s][%s]\n",key,(char *)value);
            break;
        case INT_TYPE:
            *(int *)value = node->valueint;
            printf("Read:[%s][%d]\n", key, *(int *)value);
            break;
        case DOUBLE_TYPE:
            *(double *)value = node->valuedouble;
            printf("Read:[%s][%lf]\n", key, *(double *)value);
            break;
        default:
            printf("warning: can not find your value type!\n");
            break;
        }
    	
    	cJSON_Delete(json);
        
    }else{
    	cJSON_Delete(json);
    	return READ_CANNOT_KEY;
    }
    return READ_KEY_SUCCESS;
}

/**
* @brief         : 文件操作+JSON解析键值对
* @param[in]	 : 键和其所在的文件
* @param[out]    : 所需要读取的键值 
* @return        : 成功返回0 其他为错误
* @author         : aiden.xiang@ibeelink.com
* @date          : 2019.08.30 
*/
int read_pairs_file(const char *key,char *value,const char *file){
    return read_pairs_fileV2(key, value, CHAR_TYPE, file);
}
int save_pairs_file(const char *key,char *value,const char *file){
    return save_pairs_fileV2(key, value, CHAR_TYPE, file);
}

