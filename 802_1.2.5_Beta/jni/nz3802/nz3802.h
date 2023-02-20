//-----------------------------------------------------------------------------
// NZ3802.h
//-----------------------------------------------------------------------------
// Copyright 2017 nationz Ltd, Inc.
// http://www.nationz.com
//
// Program Description:
//
// driver definitions for the nfc reader.
//
//
//      PROJECT:   NZ3802 firmware
//      $Revision: $
//      LANGUAGE:  ANSI C
//
//
// Release 1.0
//    -Initial Revision (NZ)
//    -14 Feb 2017
//    -Latest release before new firmware coding standard
//
#ifndef __NZ_RC5XX_H_
#define __NZ_RC5XX_H_

#include "NZ3802_cfg.h"

typedef struct
{
	u32 numFieldOnFail;//��ʧ�ܼ���
    u32 numFieldOffFail;//�ر�ʧ�ܼ���
    u32 numL3OK;
    u32 numL4OK;
    u32 Totality;
}tsSuccessRate;//�ɹ���ͳ��

typedef enum
{
	CT_A = 0,	// TypeAģʽ
	CT_B,		// TypeBģʽ
	CT_MI		// MIFAREģʽ
}teCardType;

typedef enum
{
	TA_REQA,
	TA_WUPA,
	TA_HLTA,
	TA_ANT,
	TA_SELECT,
	TA_RATS,
	TA_PPS,
	TA_IBLOCK,
	TA_RSBLOCK,
	TA_REQB,
	TB_WUPB,
	TB_ATTRIB,
	TB_HLTB,
	TB_IBLOCK,
	TB_xBLOCK, //�������֤
	CMD_TOTAL
}eCmd;

extern u8  FWI;
extern u16 FSD;
extern u16 FSC;
extern u8  CID;         
extern u8  NAD;
extern u8  BlockNum;
extern const u8 FSCTab[];
extern const u8 UartSpeedTab[];


extern u8  reg29h_MODGSP;
extern u8  reg18h_RXTRESHOLD;
extern u8  reg24h_MODWIDTH;
extern u8  reg27h_GSN;
extern u8  reg28h_CWGSP;
extern u8  reg26h_RFCFG;
extern u8  reg16h_TXSEL;
extern u8  reg38h_ANALOGTEST;
extern u16 TxRxSpeed;
extern u8  PPSEn;
extern u32 UartBaudRate;

extern tsSuccessRate SuccessRate;

extern u32   power(u8 n);
extern void* mem_copy(void * dest,const void *src, u16 count);

extern void  NZ3802SetTimer(u32 fc);
extern void  NZ3802SetTimer2(u8 fwi);
extern u8    NZ3802HwReset(void);
extern u8    NZ3802SoftReset(void);
extern u8    NZ3802SoftPwrDown(bool bEnable);
extern u8    NZ3802ActivateField(bool activateField);
extern u8    NZ3802Init(teCardType);

extern u8    NZ3802Transceive(eCmd command, 
                    const u8 *request, u8 requestLength, u8 txalign, 
                          u8 *response, u8 *responseLength, u8 rxalign);
extern u8    NZ3802IBLOCK(const u8 *inf, u16 infLength, 
                          u8 *response, u16 *responseLength);

extern void NZ3802Test();
#endif
