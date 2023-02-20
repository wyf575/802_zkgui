#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

/**
* @brief         : 生成CRC32模型所需的多项式
* @param[in]	 : 全局变量 CRC数组
* @param[out]    : None
* @return        : None
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.28
*/
static unsigned long crc32_tbl[256];
static void crc32_table_create(void)
{
    unsigned int c;
    unsigned int i, j;
    for (i = 0; i < 256; i++)
    {
        c = (unsigned int)i;
        for (j = 0; j < 8; j++)
        {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc32_tbl[i] = c;
    }
}

void CRC32_Init(CRC32_CTX *ctx)
{
    crc32_table_create();
    ctx->crc = 0xFFFFFFFFL;
}

void CRC32_Update(CRC32_CTX *ctx, const unsigned char *data, size_t len)
{
    size_t i = 0;
    for (i = 0; i < len; i++)
    {
        ctx->crc = (ctx->crc >> 8) ^ crc32_tbl[(ctx->crc & 0xFF) ^ *data++];
    }
}

void CRC32_Final(CRC32_CTX *ctx)
{
    ctx->crc ^= 0xFFFFFFFFUL;
    ctx->crc32[0] = (ctx->crc & 0xFF000000UL) >> 24;
    ctx->crc32[1] = (ctx->crc & 0x00FF0000UL) >> 16;
    ctx->crc32[2] = (ctx->crc & 0x0000FF00UL) >> 8;
    ctx->crc32[3] = (ctx->crc & 0x000000FFUL);
}
/**
* @brief         : 计算并生成CRC32数据
* @param[in]	 : 需要计算的数据及长度
* @param[out]    : None
* @return        : 计算结果unsigned int型
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.10.28
*/
int crc32_calculate(void *pBuf, unsigned int pBufSize,CRC32_CTX *ctx){
    CRC32_Init(ctx);
    CRC32_Update(ctx, (const unsigned char*)pBuf, pBufSize);
    CRC32_Final(ctx);
    return 0;
}



