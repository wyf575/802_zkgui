

#include "nz_string.h"

#include "errno.h"
#include "nz3802_cfg.h"
#include "nz3802_com.h"
#include "nz3802.h"
#include "mifare.h" 
//#include "TEST.h"

#if MIFARE_ONE_EN
/*
******************************************************************************
* LOCAL MACROS
******************************************************************************
*/
#define MIFARE_LOG_EN
#ifdef  MIFARE_LOG_EN
#define MIFARE_DBG      printf
#define MIFARE_DBG_EXT  printf
#else
#define	MIFARE_DBG(...)
#define MIFARE_DBG_EXT(...)
#endif

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static u8 MifareComTransceive(mifareTranSceiveBuffer_t *miCard)
{
    u8 recebyte = 0;
    u8 status;
    u8 irqEn   = 0x00;
    u8 waitFor = 0x00;
    u8 lastBits;
    u8 n;
    u16 i;
    
    switch (miCard->MfCommand)
    {
        case COMMAND_IDLE:
            irqEn   = 0x00;
            waitFor = 0x00;
            break;
        case COMMAND_AUTHENT:
            irqEn   = 0x12;
            waitFor = 0x10;
            break;           
        case COMMAND_TRANSCEIVE:
            irqEn   = 0x77;
            waitFor = 0x30;
            recebyte = 1;
            break;
        case COMMAND_TRANSMIT:
            irqEn   = 0x77;
            waitFor = 0x30;
            break;
        default:
            miCard->MfCommand = MI_UNKNOWN_COMMAND;
            break;
    }

    if (miCard->MfCommand != MI_UNKNOWN_COMMAND)
    {
        nzWriteReg(COMMIEN,irqEn|0x80);
        nzClearBitMask(COMMIRQ,0x80);
        nzWriteReg(COMMAND,COMMAND_IDLE);
        nzSetBitMask(FIFOLEVEL,0x80);


        for (i = 0; i < miCard->MfLength; i++)
        {
            nzWriteReg(FIFODATA, miCard->MfData[i]);
        }
        nzWriteReg(COMMAND, miCard->MfCommand);
        if (miCard->MfCommand == COMMAND_TRANSCEIVE)
        {   
            nzSetBitMask(BITFRAMING,0x80); 
        }
        i = 2000;
        do
        {
            n = nzReadReg(COMMIRQ);
            i--;
        }while ((i != 0) && !(n & 0x01) && !(n & waitFor));
        nzClearBitMask(BITFRAMING,0x80);
        status = MI_COM_ERR;
        if (i != 0)
        {
            if (!(nzReadReg(REGERROR) & 0x1B))
            {
                status = MI_OK;
                if(n & irqEn & 0x01)
                {
                    status = MI_NOTAGERR;
                }
                if (recebyte)
                {
                    n = nzReadReg(FIFOLEVEL);
                    lastBits = nzReadReg(CONTROL) & 0x07;
                    if (lastBits)
                    {
                        miCard->MfLength = (n - 1) * 8 + lastBits;
                    }
                    else
                    {
                        miCard->MfLength = n * 8;
                    }
                    if (n == 0)
                    {
                        n = 1;
                    }
                    for (i = 0; i < n; i++)
                    {
                        miCard->MfData[i] = nzReadReg(FIFODATA);
                    }
                }
            }
            //else if (nzReadReg(REGERROR) & 0x01)
            else if (nzReadReg(REGERROR) & 0x08)
            {
                status = MI_COLLERR;
                if (recebyte)
                {
                    n = nzReadReg(FIFOLEVEL);
                    lastBits = nzReadReg(CONTROL) & 0x07;
                    if (lastBits)
                    {
                        miCard->MfLength = (n - 1) * 8 + lastBits;
                    }
                    else
                    {
                        miCard->MfLength = n * 8;
                    }
                    if (n == 0)
                    {
                        n = 1;
                    }
                    for (i = 0; i < n; i++)
                    {
                        miCard->MfData[i + 1] = nzReadReg(FIFODATA);
                    }
                }
                miCard->MfData[0] = nzReadReg(COLL);
            }
        }
        else if (n & irqEn & 0x20)
        {
            status = MI_NOTAGERR;
        }
        else if (!(nzReadReg(REGERROR) & 0x17))
        {
            status = MI_ACCESSTIMEOUT;
        }
        else
        {
            status = MI_COM_ERR;
        }
        nzSetBitMask(CONTROL,0x80);          
        nzWriteReg(COMMAND,COMMAND_IDLE); 
    }
    return status;
}

static void MifareCalulateCRC(u8 *pIn, u8 len, u8 *pOut)
{
    u8 i;
    u8 n; 
    
    nzClearBitMask(DIVIRQ, 0x04);
    nzWriteReg(COMMAND, COMMAND_IDLE);
    nzSetBitMask(FIFOLEVEL, 0x80);
    for (i = 0; i < len; i++)
    {
        nzWriteReg(FIFODATA, *(pIn + i));
    }
    nzWriteReg(COMMAND, COMMAND_CALCCRC);
    do
    {
        n = nzReadReg(STATUS1);
    }while (!(n & 0x20));
    
    pOut [0] = nzReadReg(CRCRESULT2);
    pOut [1] = nzReadReg(CRCRESULT1);
}

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
u8 MifareInitialize(void)
{
    NZ3802Init(CT_MI);
	  nzWriteReg(0x1c, 0x62);
    return MI_OK;
}
u8 MifareInitialize_3v(void)
{
	printf("-----------MifareInitialize_3v-----------\n");
    NZ3802Init(CT_MI);
	  nzWriteReg(0x1c, 0x72);
    return MI_OK;
}
u8 MifareRequest(mifareCommand_t cmd, mifareProximityCard_t *miCard)
{
	printf("--------------MifareRequest-----------------\n");
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    nzClearBitMask(STATUS2, 0x08);
    nzWriteReg(BITFRAMING, 0x07);
    nzSetBitMask(TXCONTROL, 0x03);

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 1;
    MfComData.MfData[0] = cmd;
    status = MifareComTransceive(&MfComData);
    if (!status)
    {
        if (MfComData.MfLength != 0x10)
        {
            status = MI_BITCOUNTERR;
        }
    }
    
    miCard->TagType[0]  = MfComData.MfData[0];
    miCard->TagType[1] =  MfComData.MfData[1];
    
    return status;
}

u8 MifareAnticoll(u8 bcnt, mifareProximityCard_t *miCard)
{
    u8 status;
    u8 i;
    u8 ucBits;
    u8 ucBytes;
    u8 snr_check = 0;
    u8 ucCollPosition = 0;
    u8 ucTemp;
    u8 ucSNR[5] = {0, 0, 0, 0 , 0};
    mifareTranSceiveBuffer_t MfComData;

    nzClearBitMask(STATUS2, 0x08);
    nzWriteReg(BITFRAMING, 0x00);
    nzClearBitMask(COLL, 0x80);
    
    do
    {
        ucBits = (ucCollPosition) % 8;
        if (ucBits != 0)
        {
            ucBytes = ucCollPosition / 8 + 1;
            nzWriteReg(BITFRAMING, (ucBits << 4) + ucBits);
        }
        else
        {
            ucBytes = ucCollPosition / 8;
        }

        MfComData.MfCommand = COMMAND_TRANSCEIVE;
        MfComData.MfData[0] = MiCMD_ANTICOLL1;
        MfComData.MfData[1] = 0x20 + ((ucCollPosition / 8) << 4) + (ucBits & 0x0F);
        for (i = 0; i < ucBytes; i++)
        {
            MfComData.MfData[i + 2] = ucSNR[i];
        }
        MfComData.MfLength = ucBytes + 2;

        MIFARE_DBG("Bit=%d,",ucBits);
        MIFARE_DBG("Byte=%d,",ucBytes);
        MIFARE_DBG("Coll=%d,",ucCollPosition);
        MIFARE_DBG("len=%d\r\n",MfComData.MfLength);
        MIFARE_DBG_EXT(MfComData.MfData,MfComData.MfLength,"T");
        status = MifareComTransceive(&MfComData);
        MIFARE_DBG_EXT(MfComData.MfData,MfComData.MfLength,"R");
        
        ucTemp = ucSNR[(ucCollPosition / 8)];
        if (status == MI_COLLERR)
        {
            for (i = 0; i < 5 - (ucCollPosition / 8); i++)
            {
                ucSNR[i + (ucCollPosition / 8)] = MfComData.MfData[i + 1];
            }
            ucSNR[(ucCollPosition / 8)] |= ucTemp;
            ucCollPosition = MfComData.MfData[0];
        }
        else if (status == MI_OK)
        {
            miCard->UidLength = (MfComData.MfLength / 8) - 1;
            for (i = 0; i < (MfComData.MfLength / 8); i++)
            {
                ucSNR[miCard->UidLength - i] = MfComData.MfData[MfComData.MfLength / 8 - i - 1];
            }
            ucSNR[(ucCollPosition / 8)] |= ucTemp;
        }
    }
    while (status == MI_COLLERR);

    if (status == MI_OK)
    {
        for (i = 0; i < bcnt; i++)
        {
            miCard->Uid[i] = ucSNR[i];
            snr_check ^= ucSNR[i];
        }
        //if (snr_check != ucSNR[i])
        if ((snr_check == 0) || (snr_check != ucSNR[i]))
        {
            status = MI_COM_ERR;
        }
    }
    
    nzSetBitMask(COLL, 0x80);
    return status;
}

u8 MifareSelect(mifareProximityCard_t *miCard)
{
    u8 i;
    u8 status;
    u8 snr_check = 0;
    mifareTranSceiveBuffer_t MfComData;

    nzClearBitMask(STATUS2, 0x08);
    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 9;
    MfComData.MfData[0] = MiCMD_ANTICOLL1;
    MfComData.MfData[1] = 0x70;
    for (i = 0; i < miCard->UidLength; i++)
    {
        snr_check ^= miCard->Uid[i];
        MfComData.MfData[i + 2] = miCard->Uid[i];
    }
    MfComData.MfData[miCard->UidLength + 2] = snr_check;
    MifareCalulateCRC(MfComData.MfData, 7, &MfComData.MfData[7]);
    status = MifareComTransceive(&MfComData);
    if (status == MI_OK)
    {
        if (MfComData.MfLength != 0x18)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            miCard->Sak = MfComData.MfData[0];
        }
    }
    return status;
}


u8 MifareAuthentication(mifareCommand_t cmd, mifareProximityCard_t *miCard, u8 block, u8 *pKey)
{
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_AUTHENT;
    MfComData.MfLength  = 12;
    MfComData.MfData[0] = cmd;
    MfComData.MfData[1] = block;
    mem_copy(&MfComData.MfData[2], pKey, 6);
    mem_copy(&MfComData.MfData[8], miCard->Uid, miCard->UidLength);

    status = MifareComTransceive(&MfComData);
    if (status == MI_OK)
    {
        if (nzReadReg(STATUS2) & 0x08)
        {
            status = MI_OK;
        }
        else
        {
            status = MI_AUTHERR;
        }
    }
    return status;
}

u8 MifareRead(u8 block, u8 *pData, u8 *pLen)
{
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 4;
    MfComData.MfData[0] = MiCMD_READ;
    MfComData.MfData[1] = block;
    MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);
    status = MifareComTransceive(&MfComData);
    if (status == MI_OK)
    {
        if (MfComData.MfLength != 0x90)
        {
            status = MI_READERR;//MI_BITCOUNTERR;
        }
        else
        {
            *pLen = (MfComData.MfLength / 8) - 2;
            mem_copy(pData, &MfComData.MfData[0], 16);
        }
    }
    
    return status;
}

u8 MifareWrite(u8 block, u8 *pData)
{
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 4;
    MfComData.MfData[0] = MiCMD_WRITE;
    MfComData.MfData[1] = block;

    MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);

    status = MifareComTransceive(&MfComData);
    if (status != MI_NOTAGERR)
    {
        if(MfComData.MfLength != 4)
        {
            status = MI_WRITEERR;//MI_BITCOUNTERR;
        }
        else
        {
            MfComData.MfData[0] &= 0x0F;
            switch (MfComData.MfData[0])
            {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
    }
    if (status == MI_OK)
    {
        MfComData.MfCommand = COMMAND_TRANSCEIVE;
        MfComData.MfLength  = 18;
        mem_copy(&MfComData.MfData[0], pData, 16);
        MifareCalulateCRC(MfComData.MfData, 16, &MfComData.MfData[16]);
        status = MifareComTransceive(&MfComData);
        if (status != MI_NOTAGERR)
        {
            MfComData.MfData[0] &= 0x0F;
            switch(MfComData.MfData[0])
            {
                case 0x00:
                    status = MI_WRITEERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
    }
    
    return status;
}

#if 0
u8 MifareInitPurse(u8 block, u8 *InitAmount)
{
    u8 status;
    u8 Data[16];
    Data[0]  = *(InitAmount + 0);
    Data[1]  = *(InitAmount + 1);
    Data[2]  = *(InitAmount + 2);
    Data[3]  = *(InitAmount + 3);
    Data[4]  = ~*(InitAmount + 0);
    Data[5]  = ~*(InitAmount + 1);
    Data[6]  = ~*(InitAmount + 2);
    Data[7]  = ~*(InitAmount + 3);
    Data[8]  = *(InitAmount + 0);
    Data[9]  = *(InitAmount + 1);
    Data[10] = *(InitAmount + 2);
    Data[11] = *(InitAmount + 3);
    Data[12] = block;
    Data[13] = ~block;
    Data[14] = block;
    Data[15] = ~block;
    status = MifareWrite(block, Data);
    return status;
}

u8 MifareReadPurse(u8 block, u8 *Amount)
{
    u8 status;
    u8 Data[16];
    u8 Len;
    status = MifareRead(block, Data, &Len);
    *(Amount + 0) = Data[0];
    *(Amount + 1) = Data[1];
    *(Amount + 2) = Data[2];
    *(Amount + 3) = Data[3];
    return status;
}

u8 MifareIncrement(u8 block, u8 *Amount)
{
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 4;
    MfComData.MfData[0] = MiCMD_INCREMENT;
    MfComData.MfData[1] = block;

    MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);
    status = MifareComTransceive(&MfComData);
    if (status != MI_NOTAGERR)
    {
        if (MfComData.MfLength != 4)
        {
            status = MI_INCRERR;//MI_BITCOUNTERR;
        }
        else
        {
            MfComData.MfData[0] &= 0x0F;
            switch (MfComData.MfData[0])
            {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                case 0x01:
                    status = MI_CRCERR;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
    }
    if (status == MI_OK)
    {
        MfComData.MfCommand = COMMAND_TRANSMIT;//COMMAND_TRANSCEIVE;
        MfComData.MfLength  = 6;
        mem_copy(&MfComData.MfData[0], Amount, 4);
        MifareCalulateCRC(MfComData.MfData, 4, &MfComData.MfData[4]);
        status = MifareComTransceive(&MfComData);
        if (status == MI_OK)
        {
            /*
            if (MfComData.MfLength != 4)
            {
                status = MI_INCRERR;//MI_BITCOUNTERR;
            }
            else
            {
                status = MI_OK;
            }
            */
            status = MI_OK;
        }
        else if(status == MI_NOTAGERR)
        {
            status = MI_OK;
        }
        else
        {
            status = MI_COM_ERR;
        }
    }

    if (status == MI_OK)
    {
        MfComData.MfCommand = COMMAND_TRANSCEIVE;
        MfComData.MfLength  = 4;
        MfComData.MfData[0] = MiCMD_TRANSFER;
        MfComData.MfData[1] = block;
        MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);
        status = MifareComTransceive(&MfComData);
        if (status != MI_NOTAGERR)
        {
            if (MfComData.MfLength != 4)
            {
                status = MI_INCRERR;//MI_BITCOUNTERR;
            }
            else
            {
                MfComData.MfData[0] &= 0x0F;
                switch(MfComData.MfData[0])
                {
                    case 0x00:
                        status = MI_NOTAUTHERR;
                        break;
                    case 0x0a:
                        status = MI_OK;
                        break;
                    case 0x01:
                        status = MI_CRCERR;
                        break;
                    default:
                        status = MI_CODEERR;
                        break;
                }
            }
        }
    }

    return status;
}

u8 MifareDecrement(u8 block, u8 *Amount)
{
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 4;
    MfComData.MfData[0] = MiCMD_DECREMENT;
    MfComData.MfData[1] = block;

    MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);

    status = MifareComTransceive(&MfComData);
    if (status != MI_NOTAGERR)
    {
        if (MfComData.MfLength != 4)
        {
            status = MI_DECRERR;//MI_BITCOUNTERR;
        }
        else
        {
            MfComData.MfData[0] &= 0x0F;
            switch (MfComData.MfData[0])
            {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                case 0x01:
                    status = MI_CRCERR;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
    }
    if (status == MI_OK)
    {
        MfComData.MfCommand = COMMAND_TRANSMIT;//COMMAND_TRANSCEIVE;
        MfComData.MfLength  = 6;
        mem_copy(&MfComData.MfData[0], Amount, 4);
        MifareCalulateCRC(MfComData.MfData, 4, &MfComData.MfData[4]);

        status = MifareComTransceive(&MfComData);
        if (status == MI_OK)
        {
            /*
            if (MfComData.MfLength != 4)
            {
                status = MI_DECRERR;//MI_BITCOUNTERR;
            }
            else
            {
                status = MI_OK;
            }
            */
            status = MI_OK;
        }
        else if(status == MI_NOTAGERR)
        {
            status = MI_OK;
        }
        else
        {
            status = MI_COM_ERR;
        }
    }

    if (status == MI_OK)
    {
        MfComData.MfCommand = COMMAND_TRANSCEIVE;
        MfComData.MfLength  = 4;
        MfComData.MfData[0] = MiCMD_TRANSFER;
        MfComData.MfData[1] = block;
        MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);

        status = MifareComTransceive(&MfComData);
        if (status != MI_NOTAGERR)
        {
            if (MfComData.MfLength != 4)
            {
                status = MI_DECRERR;//MI_BITCOUNTERR;
            }
            else
            {
                MfComData.MfData[0] &= 0x0F;
                switch(MfComData.MfData[0])
                {
                    case 0x00:
                        status = MI_NOTAUTHERR;
                        break;
                    case 0x0a:
                        status = MI_OK;
                        break;
                    case 0x01:
                        status = MI_CRCERR;
                        break;
                    default:
                        status = MI_CODEERR;
                        break;
                }
            }
        }
    }

    return status;
}

u8 MifareRestore(u8 block)
{
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 4;
    MfComData.MfData[0] = MiCMD_RESTORE;
    MfComData.MfData[1] = block;
    MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);

    status = MifareComTransceive(&MfComData);
    if (status != MI_NOTAGERR)
    {
        if (MfComData.MfLength != 4)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            MfComData.MfData[0] &= 0x0F;
            switch(MfComData.MfData[0])
            {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                case 0x01:
                    status = MI_CRCERR;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
    }
    if (status == MI_OK)
    {
        MfComData.MfCommand = COMMAND_TRANSCEIVE;
        MfComData.MfLength  = 6;
        MfComData.MfData[0] = 0;
        MfComData.MfData[1] = 0;
        MfComData.MfData[2] = 0;
        MfComData.MfData[3] = 0;
        MifareCalulateCRC(MfComData.MfData, 4, &MfComData.MfData[4]);
        status = MifareComTransceive(&MfComData);
        if (status == MI_NOTAGERR)
        {
            status = MI_OK;
        }
    }
    return status;
}

u8 MifareTransfer(u8 block)
{
    u8 status;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 4;
    MfComData.MfData[0] = MiCMD_TRANSFER;
    MfComData.MfData[1] = block;
    MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);
    status = MifareComTransceive(&MfComData);
    if (status != MI_NOTAGERR)
    {
        if (MfComData.MfLength != 4)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            MfComData.MfData[0] &= 0x0F;
            switch (MfComData.MfData[0])
            {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                case 0x01:
                    status = MI_CRCERR;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
    }
    return status;
}

u8 MifareHalt(void)
{
    u8 status = MI_OK;
    mifareTranSceiveBuffer_t MfComData;

    MfComData.MfCommand = COMMAND_TRANSCEIVE;
    MfComData.MfLength  = 4;
    MfComData.MfData[0] = MiCMD_HALT;
    MfComData.MfData[1] = 0;
    MifareCalulateCRC(MfComData.MfData, 2, &MfComData.MfData[2]);
    status = MifareComTransceive(&MfComData);
    if (status)
    {
        if (status == MI_NOTAGERR || status == MI_ACCESSTIMEOUT)
            status = MI_OK;
    }
    return status;
}
#endif

//-----------------------------------------------------------------------------
//Input:      value:0-5.5V耐压，1-3.3V耐压
// Mifare One 卡测试用例接口函数:
// 1,配置Mifareg工作模式
// 2,关场5ms
// 3,开场10ms
// 4,寻卡
// 5,防冲撞
// 6,選卡激活
// 7,验证A密钥 【FF FF FF FF FF FF】
// 8,写块4
// 9,读块4
u8 MifarePresent(u8 value)
{
    const u8 KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};
    static mifareProximityCard_t miCard;
     
    u8 ReadDat[16];
    u8 WriteDat[16]={0};
    u8 length;
    u8 block;
    u8 status;
    
    MIFARE_DBG("\r\n\r\n******Mifare One Card Present*******************\r\n");
		if(value==1)
		{
				MifareInitialize_3v();
		}
		else
		{
				MifareInitialize();
		}
    
    status = NZ3802ActivateField(FALSE);
    if(status!=ERR_NONE)
    {
        SuccessRate.numFieldOffFail++;
        MIFARE_DBG(">Field OFF\r\n FAIL.\r\n");
        return status;
    }
    usleep(15000);
    do {
    	sleep(1);
		status = NZ3802ActivateField(TRUE);
		MIFARE_DBG("-------NZ3802ActivateField --\n");
	} while (status!=ERR_NONE);
//    status = NZ3802ActivateField(TRUE);
//    if(status!=ERR_NONE)
//    {
//        SuccessRate.numFieldOnFail++;
//        MIFARE_DBG(">Field ON\r\n FAIL.\r\n");
//        return status;
//    }
    usleep(15000);
    SuccessRate.Totality++;
    block = 4;  //操作块4
    //寻卡
    status = MifareRequest(MiCMD_REQALL,&miCard);
    if(status==MI_OK)
    {
        MIFARE_DBG(">Request_OK.\r\n"); 
        MIFARE_DBG(" TagType=0x%02x%02x -> ",miCard.TagType[0],miCard.TagType[1]);
        /*TagType��0x4400 = ultra_light
                   0x0400 = Mifare_One(S50)
                   0x0200 = Mifare_One(S70)
                   0x4403 = Mifare_DESFire
                   0x0800 = Mifare_Pro
                   0x0403 = Mifare_ProX
                   0x0033 = SHC1102
        */
        if(miCard.TagType[0]==0x44 && miCard.TagType[1]==0x00)
        {
            MIFARE_DBG(" ultra_light\r\n");
        }
        else if(miCard.TagType[0]==0x04 && miCard.TagType[1]==0x00)
        {
            MIFARE_DBG(" Mifare_One(S50)\r\n");
        }
        else if(miCard.TagType[0]==0x02 && miCard.TagType[1]==0x00)
        {
            MIFARE_DBG(" Mifare_One(S70)\r\n");
        }
        else if(miCard.TagType[0]==0x44 && miCard.TagType[1]==0x03)
        {
            MIFARE_DBG(" Mifare_DESFire\r\n");
        }
        else if(miCard.TagType[0]==0x08 && miCard.TagType[1]==0x00)
        {
            MIFARE_DBG(" Mifare_Pro\r\n");
        }
        else if(miCard.TagType[0]==0x04 && miCard.TagType[1]==0x03)
        {
            MIFARE_DBG(" Mifare_ProX\r\n");
        }
        else if(miCard.TagType[0]==0x00 && miCard.TagType[1]==0x33)
        {
            MIFARE_DBG(" SHC1102\r\n");
        }
        else
        {
            MIFARE_DBG(" ???\r\n");
        }
    }
    else
    {
        MIFARE_DBG(">Request_FAIL.\r\n");    
        MIFARE_DBG(" NO_CARD!!![%d]\r\n",status);
    }

    //防冲撞
    if(status==MI_OK)
    {
        status = MifareAnticoll(4,&miCard);
        if(status==MI_OK)
        {
            MIFARE_DBG(">Anticoll_OK.\r\n");      	
            MIFARE_DBG_EXT(miCard.Uid,miCard.UidLength," CardID:");
        }
        else
        {
            MIFARE_DBG(">Anticoll_FAIL!!![%d]\r\n",status);   
        }
    }

    //选卡
    if(status==MI_OK)
    {
        status = MifareSelect(&miCard);
        if(status==MI_OK)
        {
            SuccessRate.numL3OK++;
            MIFARE_DBG(">Select_OK.\r\n");
            MIFARE_DBG(" SAK=0x%02x\r\n",miCard.Sak);
        }
        else
        {
            MIFARE_DBG(">Select_FAIL!!![%d]\r\n",status); 
        }
    }

    //验证
    if(status==MI_OK)
    {
        status = MifareAuthentication(MiCMD_AUTHENT1A,&miCard,block,(u8*)KEY);
        if(status==MI_OK)
        {
            MIFARE_DBG(">AuthState_OK.\r\n");
        }
        else
        {
            MIFARE_DBG(">AuthState_FAIL!!![%d]\r\n",status);  
        }
    }

    //写块
    if(status==MI_OK)
    {
        memset(WriteDat, 0xaa, 16);
        status = MifareWrite(block, WriteDat);
        if(status==MI_OK)
        {
            MIFARE_DBG(">Write_Block[%d]_OK.\r\n",block);
            MIFARE_DBG_EXT(WriteDat,16," Content:");
            
        }
        else
        {
            MIFARE_DBG(">Write_Block[%d]_FAIL!!![%d]\r\n",block,status); 
        }
    }
    
    //读块
    if(status==MI_OK)
    {
        memset(ReadDat, 0x00, 16);
        status = MifareRead(block, ReadDat,&length);
        if(status==MI_OK)
        {
            SuccessRate.numL4OK++;
            MIFARE_DBG(">Read_Block[%d]_OK.\r\n",block);
            MIFARE_DBG_EXT(ReadDat,length," Content:");//content of block
        }
        else
        {
            MIFARE_DBG(">Read_Block[%d]_FAIL!!![%d]\r\n",block,status); 
        }
    }

    MIFARE_DBG("<<RFONFail=%d,RFOFFFail=%d,L3Pass=%d,L4Pass=%d,Totality=%d\r\n",
        SuccessRate.numFieldOnFail,SuccessRate.numFieldOffFail,
        SuccessRate.numL3OK,SuccessRate.numL4OK,SuccessRate.Totality);
    
    return status;
}
#endif

