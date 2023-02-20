#ifndef USER_UTIL_H
#define USER_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct 
{
    const char *dirPath;
    const char *startStr;
    const char *endStr;
    int total;
    int size;
    int recursive;
}fileParams;

int getFilesName(fileParams *params,char files[][256]);
int is_end_with(const char *str1, char *str2);

/**
 * @brief Get the Type Files Count object
 * 
 * @param dirPath 目录地址
 * @param startStr 开头字符串
 * @param endStr 结尾字符串
 * @date 2021.07.16 
 * @return int 符合要求的数量
 */
int getTypeFilesCount(char *dirPath,char *startStr,char *endStr);


#endif // !USER_UTIL_H
