#ifndef USR_FILE_H
#define USR_FILE_H
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define CHECK_NULL(x)                       \
    {                                       \
        if (NULL == x)                      \
        {                                   \
            printf("this point is NULL\n"); \
            return -1;                      \
        }                                   \
    }
    
typedef enum
{
    CLEAN_WRITE = 0,    /*先清除内容 再写*/
    APPEND_WRITE,       /*不清除内容 追加*/
    ONLY_READ,          /*只读文件*/
} usr_file_opt;

int write_data_to_file(const char filename[], const void *buf, const int size, usr_file_opt method);
int read_data_from_file(const char filename[], void *buf, const int size, int offset);
long read_file_size_byte(const char *filename);
int read_data_by_fp(FILE *fp, void *buf, const int size);

#endif