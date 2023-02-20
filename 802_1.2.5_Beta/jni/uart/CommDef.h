/*
 * CommDef.h
 *
 *  Created on: 2016-2-15
 *      Author: guoxs
 */

#ifndef _UART_COMMDEF_H_
#define _UART_COMMDEF_H_

#include <stdint.h>

#ifndef BYTE
typedef unsigned char	BYTE;
#endif
#ifndef UINT
typedef unsigned int	UINT;
#endif
#ifndef UINT16
typedef unsigned short  UINT16;
#endif

#ifndef MAKEWORD
#define MAKEWORD(low, high)		(((BYTE)(low)) | (((BYTE)(high)) << 8))
#endif

#ifndef LOBYTE
#define LOBYTE(l)           ((BYTE)(l))
#endif

#ifndef HIBYTE
#define HIBYTE(l)           ((BYTE)(l >> 8))
#endif

#ifndef TABLESIZE
#define TABLESIZE(table)    (sizeof(table)/sizeof(table[0]))
#endif

#define ARRLEN(data) 	(sizeof(data) / sizeof(data[0]))

// ��Ҫ��ӡЭ������ʱ�������º�
#define DEBUG_PRO_DATA

// ֧��checksumУ�飬�����º�
//#define PRO_SUPPORT_CHECK_SUM


/* SynchFrame CmdID  DataLen Data CheckSum (��ѡ) */
/*     2Byte  2Byte   1Byte	N Byte  1Byte */
// ��CheckSum�������С����: 2 + 2 + 1 + 1 = 6
// ��CheckSum�������С����: 2 + 2 + 1 = 5

#ifdef PRO_SUPPORT_CHECK_SUM
#define DATA_PACKAGE_MIN_LEN		6
#else
#define DATA_PACKAGE_MIN_LEN		5
#endif

// ͬ��֡ͷ
#define CMD_HEAD1	0xFF
#define CMD_HEAD2	0x55

#endif /* _UART_COMMDEF_H_ */
