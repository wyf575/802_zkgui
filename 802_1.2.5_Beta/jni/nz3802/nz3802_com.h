//-----------------------------------------------------------------------------
// NZ3802_com.h
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

#ifndef __NZ3802_COM_H_
#define __NZ3802_COM_H_

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "NZ3802_cfg.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************		
*/

typedef enum
{
    SPI = 0,
    IIC,
    UART
}eComMode;
    
#define  NZ3802_IIC_ADDR  0x50        //I2C=1 EA=0
extern eComMode ComMode;              //default: SPI

//============================================================================
#define  BFL_JREG_EXT_REG_ENTRANCE    0x0F    //ext register entrance
//============================================================================
#define  BFL_JBIT_EXT_REG_WR_ADDR     0X40    //wrire address cycle
#define  BFL_JBIT_EXT_REG_RD_ADDR     0X80    //read address cycle
#define  BFL_JBIT_EXT_REG_WR_DATA     0XC0    //write data cycle
#define  BFL_JBIT_EXT_REG_RD_DATA     0X00    //read data cycle
//以下-----寄存器地址-----
#define  PAGE0           0x00    //Page register in page 0
#define  COMMAND         0x01    //Contains Command bits, PowerDown bit and bit to switch receiver off.
#define  COMMIEN         0x02    //Contains Communication interrupt enable bits andbit for Interrupt inversion.
#define  DIVIEN          0x03    //Contains RfOn, RfOff, CRC and Mode Interrupt enable and bit to switch Interrupt pin to PushPull mode.
#define  COMMIRQ         0x04    //Contains Communication interrupt request bits.
#define  DIVIRQ          0x05    //Contains RfOn, RfOff, CRC and Mode Interrupt request.
#define  REGERROR        0x06    //Contains Protocol, Parity, CRC, Collision, Buffer overflow, Temperature and RF error flags.
#define  STATUS1         0x07    //Contains status information about Lo- and HiAlert, RF-field on, Timer, Interrupt request and CRC status.
#define  STATUS2         0x08    //Contains information about internal states (Modemstate),Mifare states and possibility to switch Temperature sensor off.
#define  FIFODATA        0x09    //Gives access to FIFO. Writing to register increments theFIFO level (register 0x0A), reading decrements it.
#define  FIFOLEVEL       0x0A    //Contains the actual level of the FIFO.
#define  WATERLEVEL      0x0B    //Contains the Waterlevel value for the FIFO
#define  CONTROL         0x0C    //Contains information about last received bits, Initiator mode bit, bit to copy NFCID to FIFO and to Start and stopthe Timer unit.
#define  BITFRAMING      0x0D    //Contains information of last bits to send, to align received bits in FIFO and activate sending in Transceive*/
#define  COLL            0x0E    //Contains all necessary bits for Collission handling
#define  RFU0F           0x0F    //Currently not used.
    
#define  PAGE1           0x10    //Page register in page 1
#define  MODE            0x11    //Contains bits for auto wait on Rf, to detect SYNC byte in NFC mode and MSB first for CRC calculation
#define  TXMODE          0x12    //Contains Transmit Framing, Speed, CRC enable, bit for inverse mode and TXMix bit.
#define  RXMODE          0x13    //Contains Transmit Framing, Speed, CRC enable, bit for multiple receive and to filter errors.
#define  TXCONTROL       0x14    //Contains bits to activate and configure Tx1 and Tx2 and bit to activate 100% modulation.
#define  TXAUTO          0x15    //Contains bits to automatically switch on/off the Rf and to do the collission avoidance and the initial rf-on.
#define  TXSEL           0x16    //Contains SigoutSel, DriverSel and LoadModSel bits.
#define  RXSEL           0x17    //Contains UartSel and RxWait bits.
#define  RXTRESHOLD      0x18    //Contains MinLevel and CollLevel for detection.
#define  DEMOD           0x19    //Contains bits for time constants, hysteresis and IQ demodulator settings.
#define  FELICANFC       0x1A    //Contains bits for minimum FeliCa length received and for FeliCa syncronisation length.
#define  FELICANFC2      0x1B    //Contains bits for maximum FeliCa length received.
#define  MIFARE          0x1C    //Contains Miller settings, TxWait settings and MIFARE halted mode bit.
#define  MANUALRCV       0x1D    //Currently not used.
#define  TYPEBREG        0x1E    //Currently not used.
#define  SERIALSPEED     0x1F    //Contains speed settings for serila interface.
      
#define  PAGE2           0x20    //Page register in page 2
#define  CRCRESULT1      0x21    //Contains MSByte of CRC Result.
#define  CRCRESULT2      0x22    //Contains LSByte of CRC Result.
#define  GSNLOADMOD      0x23    //Contains the conductance and the modulation settings for the N-MOS transistor only for load modulation (See difference to BFL_JREG_GSN!).
#define  MODWIDTH        0x24    //Contains modulation width setting.
#define  TXBITPHASE      0x25    //Contains TxBitphase settings and receive clock change.
#define  RFCFG           0x26    //Contains sensitivity of Rf Level detector, the receiver gain factor and the RfLevelAmp.
#define  GSN             0x27    //Contains the conductance and the modulation settings for the N-MOS transistor during active modulation (no load modulation setting!).
#define  CWGSP           0x28    //Contains the conductance for the P-Mos transistor.
#define  MODGSP          0x29    //Contains the modulation index for the PMos transistor.
#define  TMODE           0x2A    //Contains all settings for the timer and the highest 4 bits of the prescaler.
#define  TPRESCALER      0x2B    //Contais the lowest byte of the prescaler.
#define  TRELOADHI       0x2C    //Contains the high byte of the reload value.
#define  TRELOADLO       0x2D    //Contains the low byte of the reload value.
#define  TCOUNTERVALHI   0x2E    //Contains the high byte of the counter value.
#define  TCOUNTERVALLO   0x2F    //Contains the low byte of the counter value.
       
#define  PAGE3           0x30    //Page register in page 3
#define  TESTSEL1        0x31    //Test register
#define  TESTSEL2        0x32    //Test register
#define  TESTPINEN       0x33    //Test register
#define  TESTPINVALUE    0x34    //Test register
#define  TESTBUS         0x35    //Test register
#define  AUTOTEST        0x36    //Test register
#define  VERSION         0x37    //Contains the product number and the version .
#define  ANALOGTEST      0x38    //Test register
#define  TESTDAC1        0x39    //Test register
#define  TESTDAC2        0x3A    //Test register
#define  TESTADC         0x3B    //Test register
#define  ANALOGUETEST1   0x3C    //Test register
#define  ANALOGUETEST0   0x3D    //Test register
#define  ANALOGUETPD_A   0x3E    //Test register
#define  ANALOGUETPD_B   0x3F    //Test register
    
    
/* /////////////////////////////////////////////////////////////////////////////
 * Possible commands
 * ////////////////////////////////////////////////////////////////////////// */
#define  COMMAND_IDLE          0x00
#define  COMMAND_CONFIG        0x01
#define  COMMAND_RANDOMIDS     0x02
#define  COMMAND_CALCCRC       0x03 //Remark:If data is already in the FIFO when the command is activated, this data is transmitted immediately. It is possible to
                           //write data to the FIFO while the Transmit command is active. Thus it is possible to transmit an unlimited number of bytes in one
                           //stream by writting them to the FIFO in time.
#define  COMMAND_TRANSMIT      0x04
#define  COMMAND_NOCMDCHANGE   0x07 /*This command does not change the actual commant of
                           the PN51x and can only be written. Remark:This command is used for WakeUp procedure
                           of PN51x to not change the current state. */
#define  COMMAND_RECEIVE       0x08 /*Activate Receiver Circuitry. Before the receiver actually starts, the state machine
                           waits until the time configured in the register RxWait has passed.
                           Remark: It is possible to read any received data from the FIFO while the Receive command
                           is active. Thus it is possible to receive an unlimited number of bytes by reading them
                           from the FIFO in time.*/
#define  COMMAND_TRANSCEIVE    0x0C /*This Command has two modes:\n
                           If Initiator is 1: Transmits data from FIFO to
                           the card and after that automatically activates
                           the receiver. Before the receiver actually
                           starts,the state machine waits until the
                           time configured in the register RxWait has
                           passed.*/
                           //Remark:This command is the combination of Transmit and Receive.
                           //If Initiator is 0: Wait until data received,writes them into the FIFO abd switches afterwards
                           //to Transmit Mode. Data are only send if Bit StartSend is set.
                           //Remark: This command is the combination of Transmit and Receive.
#define  COMMAND_AUTOCOLL      0x0D /*Activates automatic anticollision in Target mode. Data from Config command is used.
                           Remark: </strong>Activate CRC before,(Mifare does it's own settings for CRC)*/
#define  COMMAND_AUTHENT       0x0E /*Perform the card authentication using the Crypto1 algorithm.*/
#define  COMMAND_SOFTRESET     0x0F /*Runs the Reset- and Initialisation Phase
                           Remark:This command can be activated by software, but only by a Power-On or Hard Reset*/
    
/*  Defintion for special transceive command, which uses only timeout to terminate!
 *  This is especially used for the FeliCa Polling command because there only a
 *  timeout is valid to terminate for a slow interface!*/
#define  COMMAND_TRANSCEIVE_TO 0x8C
    
/* /////////////////////////////////////////////////////////////////////////////
 * Bit Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*name  Bit definitions of Page 0 */
/* Command Register                          */
#define  BFL_JBIT_RCVOFF             0x20   /*Switches the receiver on/off. */
#define  BFL_JBIT_POWERDOWN          0x10   /*Switches PN51x to Power Down mode. */
    
/* CommIEn Register                          */
#define  BFL_JBIT_IRQINV             0x80   /*Inverts the output of IRQ Pin. */
    
/* DivIEn Register                           */
#define  BFL_JBIT_IRQPUSHPULL        0x80   /*Sets the IRQ pin to Push Pull mode. */
    
/* CommIEn and CommIrq Register              */
#define  BFL_JBIT_TXI                0x40   /*Bit position for Transmit Interrupt Enable/Request. */
#define  BFL_JBIT_RXI                0x20   /*Bit position for Receive Interrupt Enable/Request. */
#define  BFL_JBIT_IDLEI              0x10   /*Bit position for Idle Interrupt Enable/Request. */
#define  BFL_JBIT_HIALERTI           0x08   /*Bit position for HiAlert Interrupt Enable/Request. */
#define  BFL_JBIT_LOALERTI           0x04   /*Bit position for LoAlert Interrupt Enable/Request. */
#define  BFL_JBIT_ERRI               0x02   /*Bit position for Error Interrupt Enable/Request. */
#define  BFL_JBIT_TIMERI             0x01   /*Bit position for Timer Interrupt Enable/Request. */
    
/* DivIEn and DivIrq Register                */
#define  BFL_JBIT_MODEI              0x08   /*Bit position for Mode Interrupt Enable/Request. */
#define  BFL_JBIT_CRCI               0x04   /*Bit position for CRC Interrupt Enable/Request. */
    
/* CommIrq and DivIrq Register               */
#define  BFL_JBIT_SET                0x80   /*Bit position to set/clear dedicated IRQ bits. */
    
/* Error Register                            */
#define  BFL_JBIT_WRERR              0x40   /*Bit position for Write Access Error. */
#define  BFL_JBIT_TEMPERR            0x40   /*Bit position for Temerature Error. */
#define  BFL_JBIT_BUFFEROVFL         0x10   /*Bit position for Buffer Overflow Error. */
#define  BFL_JBIT_COLLERR            0x08   /*Bit position for Collision Error. */
#define  BFL_JBIT_CRCERR             0x04   /*Bit position for CRC Error. */
#define  BFL_JBIT_PARITYERR          0x02   /*Bit position for Parity Error. */
#define  BFL_JBIT_PROTERR            0x01   /*Bit position for Protocol Error. */
    
/* Status 1 Register                         */
#define  BFL_JBIT_CRCOK              0x40   /*Bit position for status CRC OK. */
#define  BFL_JBIT_CRCREADY           0x20   /*Bit position for status CRC Ready. */
#define  BFL_JBIT_IRQ                0x10   /*Bit position for status IRQ is active. */
#define  BFL_JBIT_TRUNNUNG           0x08   /*Bit position for status Timer is running. */
#define  BFL_JBIT_HIALERT            0x02   /*Bit position for status HiAlert. */
#define  BFL_JBIT_LOALERT            0x01   /*Bit position for status LoAlert. */
    
/* Status 2 Register                         */
#define  BFL_JBIT_TEMPSENSOFF        0x80   /*Bit position to switch Temperture sensors on/off. */
#define  BFL_JBIT_I2CFORCEHS         0x40   /*Bit position to forece High speed mode for I2C Interface. */
#define  BFL_JBIT_CRYPTO1ON          0x08   /*Bit position for reader status Crypto is on. */
    
/* FIFOLevel Register                        */
#define  BFL_JBIT_FLUSHBUFFER        0x80   /*Clears FIFO buffer if set to 1 */
    
/* Control Register                          */
#define  BFL_JBIT_TSTOPNOW           0x80   /*Stops timer if set to 1. */
#define  BFL_JBIT_TSTARTNOW          0x40   /*Starts timer if set to 1. */
    
/* BitFraming Register                       */
#define  BFL_JBIT_STARTSEND          0x80   /*Starts transmission in transceive command if set to 1. */
    
/* BitFraming Register                       */
#define  BFL_JBIT_VALUESAFTERCOLL    0x80   /*Activates mode to keep data after collision. */
    
/*name PN51x Bit definitions of Page 1
 *  Below there are useful bit definition of the PN51x register set of Page 1.*/
/* Mode Register                             */
#define  BFL_JBIT_MSBFIRST           0x80   /*Sets CRC coprocessor with MSB first. */
#define  BFL_JBIT_TXWAITRF           0x20   /*Tx waits until Rf is enabled until transmit is startet, else
                                                    transmit is started immideately. */
#define  BFL_JBIT_POLSIGIN           0x08   /*Inverts polarity of SiginActIrq, if bit is set to 1 IRQ occures
                                                    when Sigin line is 0. */
    
/* TxMode Register                           */
#define  BFL_JBIT_INVMOD             0x08   /*Activates inverted transmission mode. */
    
/* RxMode Register                           */
#define  BFL_JBIT_RXNOERR            0x08   /*If 1, receiver does not receive less than 4 bits. */
#define  BFL_JBIT_RXMULTIPLE         0x04   /*Activates reception mode for multiple responses. */
    
/* Definitions for Tx and Rx                 */
#define  BFL_JBIT_106KBPS            0x00   /*Activates speed of 106kbps. */
#define  BFL_JBIT_212KBPS            0x10   /*Activates speed of 212kbps. */
#define  BFL_JBIT_424KBPS            0x20   /*Activates speed of 424kbps. */
#define  BFL_JBIT_848KBPS            0x30   /*Activates speed of 848kbps. */
    
    
#define  BFL_JBIT_TYPEA              0x00   /*Activates TYPEA communication mode. */
#define  BFL_JBIT_TYPEB              0x03   /*Activates TYPEB communication mode. */
    
#define  BFL_JBIT_CRCEN              0x80   /*Activates transmit or receive CRC. */
    
/* TxControl Register                        */
#define  BFL_JBIT_INVTX2ON           0x80   /*Inverts the Tx2 output if drivers are switched on. */
#define  BFL_JBIT_INVTX1ON           0x40   /*Inverts the Tx1 output if drivers are switched on. */
#define  BFL_JBIT_INVTX2OFF          0x20   /*Inverts the Tx2 output if drivers are switched off. */
#define  BFL_JBIT_INVTX1OFF          0x10   /*Inverts the Tx1 output if drivers are switched off. */
#define  BFL_JBIT_TX2CW              0x08   /*Does not modulate the Tx2 output, only constant wave. */
#define  BFL_JBIT_TX2RFEN            0x02   /*Switches the driver for Tx2 pin on. */
#define  BFL_JBIT_TX1RFEN            0x01   /*Switches the driver for Tx1 pin on. */
    
/* TxAuto Register                           */
#define  BFL_JBIT_FORCE100ASK        0x40   /*Activates 100%ASK mode independent of driver settings. */
/* Demod Register                            */
#define  BFL_JBIT_FIXIQ              0x20   /*If set to 1 and the lower bit of AddIQ is set to 0, the receiving is fixed to I channel.
                                                    If set to 1 and the lower bit of AddIQ is set to 1, the receiving is fixed to Q channel. */
/* RFU 0x1D Register                         */
#define  BFL_JBIT_PARITYDISABLE      0x10   /*Disables the parity generation and sending independent from the mode. */
    
/*! \name PN51x Bit definitions of Page 2
 *  Below there are useful bit definition of the PN51x register set.
 */
/* TMode Register                           (2A) */
#define  BFL_JBIT_TAUTO              0x80   /*Sets the Timer start/stop conditions to Auto mode. */
#define  BFL_JBIT_TAUTORESTART       0x10   /*Restarts the timer automatically after finished
                                                    counting down to 0. */
/*@}*/

/*! \name PN51x Bit definitions of Page 3
 *  Below there are useful bit definition of the PN51x register set.
 */
/* AutoTest Register                        (36) */
#define  BFL_JBIT_AMPRCV             0x40   /* */
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bitmask Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name PN51x Bitmask definitions
 *  Below there are some useful mask defintions for the PN51x. All specified
 *  bits are set to 1.
 */
/* Command register                 (0x01)*/
#define  BFL_JMASK_COMMAND           0x0F   /*Bitmask for Command bits in Register BFL_JREG_COMMAND. */
#define  BFL_JMASK_COMMAND_INV       0xF0   /*Inverted bitmask of BFL_JMASK_COMMAND. */
    
/* Waterlevel register              (0x0B)*/
#define  BFL_JMASK_WATERLEVEL        0x3F   /*Bitmask for Waterlevel bits in register BFL_JREG_WATERLEVEL. */
    
/* Control register                 (0x0C)*/
#define  BFL_JMASK_RXBITS            0x07   /*Bitmask for RxLast bits in register BFL_JREG_CONTROL. */
    
/* Mode register                    (0x11)*/
#define  BFL_JMASK_CRCPRESET         0x03   /*Bitmask for CRCPreset bits in register BFL_JREG_MODE. */
    
/* TxMode register                  (0x12, 0x13)*/
#define  BFL_JMASK_SPEED             0x70   /*Bitmask for Tx/RxSpeed bits in register BFL_JREG_TXMODE and BFL_JREG_RXMODE. */
#define  BFL_JMASK_FRAMING           0x03   /*Bitmask for Tx/RxFraming bits in register BFL_JREG_TXMODE and BFL_JREG_RXMODE. */
    
/* TxSel register                   (0x16)*/
#define  BFL_JMASK_LOADMODSEL        0xC0   /*Bitmask for LoadModSel bits in register BFL_JREG_TXSEL. */
#define  BFL_JMASK_DRIVERSEL         0x30   /*Bitmask for DriverSel bits in register BFL_JREG_TXSEL. */
#define  BFL_JMASK_SIGOUTSEL         0x0F   /*Bitmask for SigoutSel bits in register BFL_JREG_TXSEL. */
    
/* RxSel register                   (0x17)*/
#define  BFL_JMASK_UARTSEL           0xC0   /*Bitmask for UartSel bits in register BFL_JREG_RXSEL. */
#define  BFL_JMASK_RXWAIT            0x3F   /*Bitmask for RxWait bits in register BFL_JREG_RXSEL. */
    
/* RxThreshold register             (0x18)*/
#define  BFL_JMASK_MINLEVEL          0xF0   /*Bitmask for MinLevel bits in register BFL_JREG_RXTHRESHOLD. */
#define  BFL_JMASK_COLLEVEL          0x07   /*Bitmask for CollLevel bits in register BFL_JREG_RXTHRESHOLD. */
    
/* Demod register                   (0x19)*/
#define  BFL_JMASK_ADDIQ             0xC0   /*Bitmask for ADDIQ bits in register BFL_JREG_DEMOD. */
#define  BFL_JMASK_TAURCV            0x0C   /*Bitmask for TauRcv bits in register BFL_JREG_DEMOD. */
#define  BFL_JMASK_TAUSYNC           0x03   /*Bitmask for TauSync bits in register BFL_JREG_DEMOD. */
    
/* FeliCa / FeliCa2 register        (0x1A, 0x1B)*/
#define  BFL_JMASK_FELICASYNCLEN     0xC0   /*Bitmask for FeliCaSyncLen bits in registers BFL_JREG_FELICANFC. */
#define  BFL_JMASK_FELICALEN         0x3F   /*Bitmask for FeliCaLenMin and FeliCaLenMax in
                                                    registers BFL_JREG_FELICANFC and BFL_JREG_FELICANFC2. */
/* Mifare register                  (0x1C)*/
#define  BFL_JMASK_SENSMILLER        0xE0   /*Bitmask for SensMiller bits in register BFL_JREG_MIFARE. */
#define  BFL_JMASK_TAUMILLER         0x18   /*Bitmask for TauMiller bits in register BFL_JREG_MIFARE. */
#define  BFL_JMASK_TXWAIT            0x03   /*Bitmask for TxWait bits in register BFL_JREG_MIFARE. */
    
/* Manual Rcv register              (0x1D)*/
#define  BFL_JMASK_HPCF				0x03   /*Bitmask for HPCF filter adjustments. */
    
/* TxBitPhase register              (0x25)*/
#define  BFL_JMASK_TXBITPHASE        0x7F   /*Bitmask for TxBitPhase bits in register BFL_JREG_TXBITPHASE. */
    
/* RFCfg register                   (0x26)*/
#define  BFL_JMASK_RXGAIN            0x70   /*Bitmask for RxGain bits in register BFL_JREG_RFCFG. */
#define  BFL_JMASK_RFLEVEL           0x0F   /*Bitmask for RfLevel bits in register BFL_JREG_RFCFG. */
    
/* GsN register                     (0x27)*/
#define  BFL_JMASK_CWGSN             0xF0   /*Bitmask for CWGsN bits in register BFL_JREG_GSN. */
#define  BFL_JMASK_MODGSN            0x0F   /*Bitmask for ModGsN bits in register BFL_JREG_GSN. */

/* CWGsP register                   (0x28)*/
#define  BFL_JMASK_CWGSP             0x3F   /*Bitmask for CWGsP bits in register BFL_JREG_CWGSP. */
    
/* ModGsP register                  (0x29)*/
#define  BFL_JMASK_MODGSP            0x3F   /*Bitmask for ModGsP bits in register BFL_JREG_MODGSP. */
    
/* TMode register                   (0x2A)*/
#define  BFL_JMASK_TGATED            0x60   /*Bitmask for TGated bits in register BFL_JREG_TMODE. */
#define  BFL_JMASK_TPRESCALER_HI     0x0F   /*Bitmask for TPrescalerHi bits in register BFL_JREG_TMODE. */

/*
******************************************************************************
* GLOBAL FUNCTION
******************************************************************************		
*/
extern void nzWriteReg(u8 reg, u8 value);
extern void nzWriteReg(u8 reg, u8* value, int len);
extern u8   nzReadReg(u8 reg);
extern void nzSetBitMask(u8 reg,u8 mask);
extern void nzClearBitMask(u8 reg,u8 mask);
extern void nzSetRegExt(u8 extRegAddr,u8 extRegData);
extern void nzClearFifo(void);
extern void nzClearFlag(void);
extern void nzStartCmd(u8 cmd);
extern void nzStopCmd(void);
extern void nzSetCRC(bool bEN);
extern void nzSetPARITY(bool bEN);
extern bool nzFlagOK(void);
extern bool nzCrcOK(void);

#endif /* __NZ3802_COM_H_ */

