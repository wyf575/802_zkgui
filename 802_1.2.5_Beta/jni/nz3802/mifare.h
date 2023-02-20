

#ifndef __ISO_MIFARE_H_
#define __ISO_MIFARE_H_

#include "nz3802_cfg.h"

/*! 
 * PCD command set.
 */
typedef enum
{
    MiCMD_REQIDL     = 0x26,     //寻天线区内未进入休眠状态
    MiCMD_REQALL     = 0x52,     //寻天线区内全部卡
    MiCMD_ANTICOLL1  = 0x93,      //防冲撞
    MiCMD_ANTICOLL2  = 0x95,      //防冲撞
    MiCMD_AUTHENT1A  = 0x60,      //验证A密钥
    MiCMD_AUTHENT1B  = 0x61,      //验证B密钥
    MiCMD_READ       = 0x30,      //读块
    MiCMD_WRITE      = 0xA0,      //写块
    MiCMD_DECREMENT  = 0xC0,      //扣款
    MiCMD_INCREMENT  = 0xC1,      //充值
    MiCMD_RESTORE    = 0xC2,      //调块数据到缓冲区
    MiCMD_TRANSFER   = 0xB0,      //保存缓冲区中数据
    MiCMD_HALT       = 0x50,      //休眠
    MiCMD_RESET      = 0xE0,      //复位
}mifareCommand_t;


#define MIFARE_MAX_UID_LENGTH     10
#define MIFARE_TAG_TYPE_LENGTH     2

/*!< 
 * struct representing an MIFARE CARD as returned by
 * #MifareRequest.
 */
typedef struct  //_mifareProximityCard_t
{
    u8 Uid[MIFARE_MAX_UID_LENGTH]; /*<! UID of the MIFARE CARD */
    u8 UidLength;     /*!< UID length */
    u8 TagType[MIFARE_TAG_TYPE_LENGTH];/*!< Tag Type */
    u8 Sak;           /*!< SAK byte */
}mifareProximityCard_t;


/*!< 
 * struct mifare TranScive Buffer
 */
typedef struct //_mifareTranSceiveBuffer_t
{
    u8  MfCommand;
    u16 MfLength;
    u8  MfData[64];
}mifareTranSceiveBuffer_t;

extern u8 MifareInitialize(void);
extern u8 MifareInitialize_3v(void);
//寻卡
extern u8 MifareRequest(mifareCommand_t cmd, mifareProximityCard_t *MiCard);
//防冲撞
extern u8 MifareAnticoll(u8 bcnt, mifareProximityCard_t *MiCard);
//选定一张卡
extern u8 MifareSelect(mifareProximityCard_t *miCard);
//验证卡密钥
extern u8 MifareAuthentication(mifareCommand_t cmd, mifareProximityCard_t *MiCard, u8 block, u8 *pKey);
//读mifare_one卡上一块(block)数据(16字节)
extern u8 MifareRead(u8 block, u8 *pData, u8 *pLen);
//写入MifareOne卡一块数据
extern u8 MifareWrite(u8 block, u8 *pData);
//初始化为钱包
extern u8 MifareInitPurse(u8 block, u8 *InitAmount);
//读取钱包值
extern u8 MifareReadPurse(u8 block, u8 *Amount);
//充值
extern u8 MifareIncrement(u8 block, u8 *Amount);
//扣款
extern u8 MifareDecrement(u8 block, u8 *Amount);
//将mifare_one卡一块数据回传
extern u8 MifareRestore(u8 block);
//将卡缓冲区中数据传入制定的块
extern u8 MifareTransfer(u8 block);
//进入HALT状态
extern u8 MifareHalt(void);


extern u8 MifarePresent(u8 value);

#endif

