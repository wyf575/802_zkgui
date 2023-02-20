/*
 * spidev_manager.h
 *
 *  Created on: 2021年2月22日
 *      Author: Administrator
 */

#ifndef JNI_SPI_SPIDEV_MANAGER_H_
#define JNI_SPI_SPIDEV_MANAGER_H_

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "nz3802/type.h"


extern int SPI_Transfer(const u8 *TxBuf, u8 *RxBuf, int len);
extern int SPI_Write(u8 *TxBuf, int len);
extern int SPI_Read(u8 *RxBuf, int len);
extern int SPI_Open(void);
extern int SPI_Close(void);

#endif /* JNI_SPI_SPIDEV_MANAGER_H_ */
