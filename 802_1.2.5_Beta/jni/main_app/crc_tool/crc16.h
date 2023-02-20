#ifndef CRC16_H
#define CRC16_H

#include <stdio.h>
#include <stdint.h>
unsigned int crc16_modbus(unsigned char *buf, unsigned int length);
uint16_t crc16_hc_iap(uint8_t *p_data, int offset, uint32_t size);
uint16_t Cal_CRC16(const uint8_t *data, uint32_t size);

#endif //CRC16_H
