#ifndef READ_JSON_FILE_H
#define READ_JSON_FILE_H
#include "cJSON.h"
#define JSON_PAIRS_FILE_SIZE  1024*10
/*文件 读取键值对 所定义的状态*/
#define READ_KEY_SUCCESS 0
#define READ_CANNOT_FILE 1
#define READ_CANNOT_READ 2
#define READ_CANNOT_KEY  3

typedef enum 
{
    CHAR_TYPE = 0,
    INT_TYPE,
    DOUBLE_TYPE,
}json_pairs_type;

int read_pairs_file(const char *key, char *value, const char *file);
int save_pairs_file(const char *key, char *value, const char *file);
int save_pairs_fileV2(const char *key, void *value, json_pairs_type type, const char *file);
int read_pairs_fileV2(const char *key, void *value, json_pairs_type type, const char *file);
int save_pairs_fileV3(const char *key,void *value, json_pairs_type type,unsigned long valueLength,const char *file);
int read_pairs_fileV3(const char *key,void *value, json_pairs_type type,long int valueLength,const char *file);


#endif // READ_JSON_FILE_H


