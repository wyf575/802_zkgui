#ifndef CRC32_H
#define CRC32_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#define FILE_SUB_SIZE 512
typedef struct
{
    unsigned char crc32[4];
    unsigned long crc;
} CRC32_CTX;

void CRC32_Init(CRC32_CTX *ctx);
void CRC32_Update(CRC32_CTX *ctx, const unsigned char *data, size_t len);
void CRC32_Final(CRC32_CTX *ctx);
int crc32_calculate(void *pBuf, unsigned int pBufSize,CRC32_CTX *ctx);



#endif // !CRC32_H#define 
