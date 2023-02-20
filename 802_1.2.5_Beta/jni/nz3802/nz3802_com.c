//-----------------------------------------------------------------------------
// NZ3802_com.c
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

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
//#include "stm32fx_io_action.h"
//#include "my_io_defs.h"
#include "NZ3802_com.h"

eComMode ComMode = SPI;

/*
******************************************************************************
* LOCAL DEFINES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/

/**
 * @brief WriteRawRC
 *        write value at the specified address
 * @param address
 * @param value
 */
static void WriteRawRC(u8 address, u8 value)
{
    if(ComMode == SPI)
    {
//        SOCKET_RF_SPI_CS_EN();//使能片选
//        SPISendRecvByte((address<<1) & 0x7E);
//        SPISendRecvByte(value);
//        SOCKET_RF_SPI_CS_DIS();
    	printf("---------WriteRawRC.address=0x%02X value=0x%02X\n", address, value);
    	u8 TxBuf[] = {((address<<1) & 0x7E)};
    	u8 result[]={0xFF};
    	SPI_Transfer(TxBuf, result, 1);
//    	usleep(1000);
    	u8 txbuf[] ={value};
    	SPI_Transfer(txbuf, result, 1);
    }
}
/**
 * @brief ReadRawRC
 *        read value from the specified address
 * @param address
 * @param
 */
static u8 ReadRawRC(u8 address)
{
	printf("------------ReadRawRC--------------\n");
    u8 rx_dat[] = {0xFF};
	
    if(ComMode == SPI)
    {
//        SOCKET_RF_SPI_CS_EN();
//        SPISendRecvByte(((address<<1) & 0x7E) | 0x80);
//        rx_dat = SPISendRecvByte(0xFF);
//        SOCKET_RF_SPI_CS_DIS();

    	u8 TxBuf[] = {(((address<<1) & 0x7E) | 0x80)};
    	u8 value[] = {0xFF};
    	SPI_Transfer(TxBuf, rx_dat, 1);
    	SPI_Transfer(value, rx_dat, 1);
    	printf("--------ReadRawRC.address=0x%02X--0x%02X\n",address, rx_dat[0]);
    	return rx_dat[0];
    }
    return rx_dat[0];
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/**
 * @brief G E N E R I C    W R I T E
 *        write value at the specified address
 * @param address-哪个寄存器
 * @param value-要写的参数
 */
void nzWriteReg(u8 reg, u8 value)
{
    WriteRawRC(reg, value);
}

/**
 * @brief G E N E R I C    R E A D
 *        read value from the specified address
 * @param address
 * @param
 */
u8 nzReadReg(u8 reg)
{
    return ReadRawRC(reg);
}

/**
 * @brief S E T   A   B I T   M A S K
 *
 * @param address
 * @param mask
 
 */
void nzSetBitMask(u8 reg,u8 mask)
{
	u8 tmp;
	tmp = nzReadReg(reg);
	nzWriteReg(reg,(tmp | mask));
}

/**
 * @brief C L E A R   A   B I T   M A S K
 *
 * @param address
 * @param mask
 例:0x08 8
 */
void nzClearBitMask(u8 reg,u8 mask)
{
	u8 tmp;
	tmp = nzReadReg(reg);//先读一哈
	nzWriteReg(reg,(tmp & (~mask)));
}

/**
 * @brief SET EXT REG DATA
 *
 * @param extRegAddr
 * @param extRegData
 */
void nzSetRegExt(u8 extRegAddr,u8 extRegData)
{
    u8 addr,regdata;

    addr = BFL_JREG_EXT_REG_ENTRANCE;
    regdata = BFL_JBIT_EXT_REG_WR_ADDR + extRegAddr;
    nzWriteReg(addr,regdata);

    addr = BFL_JREG_EXT_REG_ENTRANCE;
    regdata = BFL_JBIT_EXT_REG_WR_DATA + extRegData;
    nzWriteReg(addr,regdata);
}

/**
 * @brief Clear Fifo 
 *
 * @param
 * @param
 */
void nzClearFifo(void)
{
    while(nzReadReg(FIFOLEVEL & 0x7f) != 0)
    {
        nzSetBitMask(FIFOLEVEL,0x00|BFL_JBIT_FLUSHBUFFER);
    }
}
/**
 * @brief Clear Flag
 *
 * @param
 * @param
 */
void nzClearFlag(void)
{
    nzWriteReg(COMMIRQ, 0x7f);
    nzWriteReg(DIVIRQ, 0x7f);
}

/**
 * @brief Start Cmd
 *
 * @param cmd
 * @param 
 */
void nzStartCmd(u8 cmd)
{
    nzWriteReg(COMMAND, cmd);
}

/**
* @brief Stop Cmd
*
* @param 
* @param 
*/
void nzStopCmd(void)
{
    nzWriteReg(COMMAND, 0x00);//命令启动和停止，连接，active
}

/**
* @brief Set CRC enable or disable
*
* @param bEN
* @param 
*/
void nzSetCRC(bool bEN)
{
    if(bEN) 
    {
        nzSetBitMask(TXMODE, BFL_JBIT_CRCEN);
        nzSetBitMask(RXMODE, BFL_JBIT_CRCEN);
    }
    else  
    {
        nzClearBitMask(TXMODE, BFL_JBIT_CRCEN);
        nzClearBitMask(RXMODE, BFL_JBIT_CRCEN);
    }
}
/**
* @brief Set PARITY enable or disable
*
* @param bEN
* @param 
*/
void nzSetPARITY(bool bEN)
{
    ;
}

/**
* @brief check errors flag if Set or Not
*
* @param 
* @param 
*/
bool nzFlagOK(void) 
{
    if((nzReadReg(REGERROR)&(BFL_JBIT_CRCERR|BFL_JBIT_PROTERR/*|BFL_JBIT_COLLERR|BFL_JBIT_PARITYERR*/))==0)
    {
        return TRUE;
    }
    return FALSE;
}

/**
* @brief check crc flag if Set or Not
*
* @param 
* @param 
*/
bool nzCrcOK(void)
{
    if((nzReadReg(REGERROR)&(BFL_JBIT_CRCERR))==0)
    {
        return TRUE;
    }
    return FALSE;
}

