//-----------------------------------------------------------------------------
// errno.h
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


#ifndef __ERRNO_H_
#define __ERRNO_H_

/*!
 * Error codes to be used within the application.
 * They are represented by an u8
 */
#define ERR_NONE           0 /*!< no error occured */
#define ERR_INIT           1       
#define ERR_CMD_TIMEOUT    2   
#define ERR_KEY            3
#define ERR_ACCESS         4
#define ERR_FIFO_OFL       5
#define ERR_CRC            6
#define ERR_FRAMING        7
#define ERR_PARITY         8
#define ERR_COLL           9
#define ERR_UID            10
#define ERR_PARA           11
#define ERR_DATA           12
#define ERR_FDT            13
#define ERR_TIMEOUT        14 
#define ERR_TRANSMIT       15 
#define ERR_OVERLOAD       16
#define ERR_PROTOCOL       17
#define ERR_ONOFFFIELD     18
#define ERR_EOT_IND        19
#define ERR_EOT_IND_30MS   20
#define ERR_NOTFOUND       21
#define ERR_GENERAL        99


/*!MIFARE CARD ONLY
 * Error codes to be used within the application.
 * They are represented by an u8
 */
#define MI_OK                    0
#define MI_NOTAGERR              1
#define MI_CRCERR                2
#define MI_AUTHERR               3
#define MI_PARITYERR             4
#define MI_COLLERR               5
#define MI_CODEERR               6

#define MI_BITCOUNTERR           10
#define MI_BYTECOUNTERR          11
#define MI_RESETERR              12
#define MI_NOTAUTHERR            13
#define MI_WRITEERR              14
#define MI_READERR               15
#define MI_INCRERR               16
#define MI_DECRERR               17

#define MI_OVERFLOWERR           20
#define MI_FRAMINGERR            21
#define MI_ACCESSTIMEOUT         22
#define MI_PARAMEPERR            23
#define MI_UNKNOWN_COMMAND       24
#define MI_COM_ERR               25
#define MI_MAX_RETRIED           26
#define MI_WR                    27

#endif /* ERRNO_H */

