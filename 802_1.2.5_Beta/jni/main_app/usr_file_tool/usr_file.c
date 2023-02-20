#include "usr_file.h"

/**
* @brief         : 写入buf到文件
* @param[in]	 : 文件路径 数据 大小 方式
* @param[out]    : None 
* @return        : 返回0正确 其他错误
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.29
*/
int write_data_to_file(const char filename[], const void *buf, const int size, usr_file_opt method)
{
    FILE *fp = NULL;
    switch (method)
    {
    case CLEAN_WRITE:
        fp = fopen(filename, "w");
        break;
    case APPEND_WRITE:
        fp = fopen(filename, "a");
        break;
    default:
        break;
    }
    CHECK_NULL(fp);
    fwrite(buf, size, 1, fp);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    return 0;
}


/**
* @brief         : 读取文件指定内容
* @param[in]	 : 文件路径 内存 大小 偏移
* @param[out]    : None 
* @return        : 返回0正确 其他错误
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.29
*/
int read_data_from_file(const char filename[], void *buf, const int size, int offset)
{
    FILE *fp = NULL;
    fp = fopen(filename, "r");
    CHECK_NULL(fp);
    fseek(fp, size * offset, SEEK_SET);
    fread(buf, size, 1, fp);
    fclose(fp);
    return 0;
}

/**
 * @brief  根据文件描述符读取文件内容
 * 只要文件不关闭 光标则不会重置
 * @param fp 
 * @param buf 
 * @param size 
 * @return int 
 * @author aiden.xiang@ibeelink.com
 * @date 2021.07.05
 */
int read_data_by_fp(FILE *fp, void *buf, const int size)
{
    CHECK_NULL(fp);
    fread(buf, size, 1, fp);
    return 0;
}

/**
* @brief         : 读取文件字节大小
* @param[in]	 : 文件路径
* @param[out]    : None 
* @return        : 返回大于0正确 其他错误
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.29
*/
long read_file_size_byte(const char *filename)
{
    FILE *pFile = NULL;
    long size = 0;
    pFile = fopen(filename, "rb");
    if (pFile == NULL)
        perror("Error opening file");
    else
    {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fclose(pFile);
        return size;
    }
    return 0;
}

