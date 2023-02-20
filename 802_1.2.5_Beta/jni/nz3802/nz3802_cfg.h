//-----------------------------------------------------------------------------
// NZ3802_cfg.h
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


#ifndef __NZ3802_CFG_H_
#define __NZ3802_CFG_H_

#include "type.h"
//#include "my_io_defs.h"
//#include "drv_spi.h"
//#include "drv_tim.h"
//#include "drv_spi.h"
//#include "drv_iic.h"
//#include "drv_uart.h"
#include "spi/spidev_manager.h"

/*
******************************************************************************
* SPI 
******************************************************************************
*/
//#define SPISendRecvByte      SPI1TxRxByte       // �ӿ� u8 function(u8)
//#define SPISendRecvByte      SPI_Transfer
#define SPI_CS_Enable        SOCKET_RF_SPI_CS_EN 
#define SPI_CS_Disable       SOCKET_RF_SPI_CS_DIS



/*
******************************************************************************
* HW RST 
******************************************************************************
*/
//#define RF_RST_Enable        SOCKET_RF_RST_EN
//#define RF_RST_Disable       SOCKET_RF_RST_DIS

/*
******************************************************************************
* DELAY 
******************************************************************************
*/
//#define SleepMS              HwSleepMs         // �ӿ� void HwSleepMs(u16)
//#define SleepUs              HwSleepUs         // �ӿ� void HwSleepUs(u16)

//#define SleepMS              sleep
#define SleepUs              usleep
/*
******************************************************************************
* FIELD RETRY 
******************************************************************************
*/
#define FIELD_ONOFF_RETRY_EN  0                //开关场重试: 1使能，0禁止
/*
******************************************************************************
* DEMO 
******************************************************************************
*/
#define ISO14443B_EN          1                //ʹ��TYPEB ����
#define ISO14443A_EN          1                //ʹ��TYPEA ����
#define MIFARE_ONE_EN         1                //ʹ��MIFARE ONE����

#endif /* __SYS_CFG_H_ */



