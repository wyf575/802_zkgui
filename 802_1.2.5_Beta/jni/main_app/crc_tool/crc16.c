#include "crc16.h"
/**
* @brief         : CRC16-Modbus校验方式
* @param[in]	 : 全局变量 CRC数组
* @param[out]    : None
* @return        : None
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.11.14
*/
unsigned int crc16_modbus(unsigned char *buf, unsigned int length)
{
    unsigned int i;
    unsigned int j;
    unsigned int c;
    unsigned int crc = 0xFFFF;
    for (i = 0; i < length; i++)
    {
        c = *(buf + i) & 0x00FF;
        crc ^= c;
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    crc = (crc >> 8) + (crc << 8);
    return (crc>>8);
}


/**
* @brief         : CRC16-HC-IAP校验方式
* @param[in]	 : 全局变量 CRC数组
* @param[out]    : None
* @return        : None
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.11.14
*/
uint16_t crc16_hc_iap(uint8_t * p_data, int offset, uint32_t size)
{
    uint8_t u8Cnt;
    uint16_t u16CrcResult = 0xA28C;
    uint32_t u32Offset = (uint32_t)offset;
    while (size != 0)
    {
        u16CrcResult ^= p_data[u32Offset++];
        for (u8Cnt = 0; u8Cnt < 8; u8Cnt++)
        {
            if ((u16CrcResult & 0x1) == 0x1)
            {
                u16CrcResult >>= 1;
                u16CrcResult ^= 0x8408;
            }
            else
            {
                u16CrcResult >>= 1;
            }
        }
        size--;
    }
    u16CrcResult = (uint16_t)(~u16CrcResult);
    return u16CrcResult;
}
/**
  * @brief  Update CRC16 for input byte
  * @param  CRC input value 
  * @param  input byte
   * @retval None
  */
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
	uint32_t crc = crcIn;
	uint32_t in = byte | 0x100;
	do
	{
		crc <<= 1;
		in <<= 1;
		if (in & 0x100)
			++crc;
		if (crc & 0x10000)
			crc ^= 0x1021;
	} while (!(in & 0x10000));
	return crc & 0xffffu;
}
/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
   * @retval None
  */
uint16_t Cal_CRC16(const uint8_t *data, uint32_t size)
{
	uint32_t crc = 0;
	const uint8_t *dataEnd = data + size;
	while (data < dataEnd)
	{
		crc = UpdateCRC16(crc, *data++);
	}

	crc = UpdateCRC16(crc, 0);
	crc = UpdateCRC16(crc, 0);
	return crc & 0xffffu;
}
