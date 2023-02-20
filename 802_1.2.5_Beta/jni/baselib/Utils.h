/*
 * Utils.h
 *
 *  Created on: 2021年1月6日
 *      Author: Administrator
 */

#ifndef JNI_BASELIB_UTILS_H_
#define JNI_BASELIB_UTILS_H_

#include <vector>
#include <cstring>
#include <strstream>
#include <openssl/md5.h>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>

using namespace std;

#define BUFSIZE     1024*4

class Utils{
private:
	Utils(){

	}

public:
	static Utils* getInstance(){
		static Utils mInstance;
		return &mInstance;
	}

	void printfCMD(unsigned char* cmd, int len){
		for(int i = 0; i < len; i++){
			printf("%02X", cmd[i]);
		}
		printf("\n");
	}

	void trimString(std::string & str )
	{
	    int s = str.find_first_not_of(" ");
	    int e = str.find_last_not_of(" ");
	    str = str.substr(s,e-s+1);
	    return;
	}

	void SplitString(const string& s, vector<string>& v, const string& c)
	{
		 string::size_type pos1, pos2;
		 pos2 = s.find(c);
		 pos1 = 0;
		 while(string::npos != pos2)
		 {
			 v.push_back(s.substr(pos1, pos2-pos1));

			 pos1 = pos2 + c.size();
			 pos2 = s.find(c, pos1);
		 }
		 if(pos1 != s.length())
			 v.push_back(s.substr(pos1));
	}

	string lltoString(long long t)
	{
	    std::string result;
	    std::strstream ss;
	    ss <<  t;
	    ss >> result;
	    return result;
	}

	string MD5(const string& src )
	{
	    MD5_CTX ctx;

	    string md5_string;
	    unsigned char md[16] = { 0 };
	    char tmp[33] = { 0 };

	    MD5_Init( &ctx );
	    MD5_Update( &ctx, src.c_str(), src.size() );
	    MD5_Final( md, &ctx );

	    for( int i = 0; i < 16; ++i )
	    {
	        memset( tmp, 0x00, sizeof( tmp ) );
	        sprintf( tmp, "%02X", md[i] );
	        md5_string += tmp;
	    }
	    return md5_string;
	}

	void IntToBytes(unsigned int num, unsigned char* bytes, int size)
	{
	    for (int i = 0; i < size; i++)
	    {
	        int offset = i * 8;
	        bytes[size - i - 1] = (num >> offset) & 0xFF;
	    }
	}

	std::string bytesToHexString(const char* bytes, const int length)
	{
		if (bytes == NULL) {
			return "";
		}
		std::string buff;
		const int len = length;
		for (int j = 0; j < len; j++) {
			/*if ((bytes[j] & 0xff) < 16) {
				buff.append("0");
			}*/
			int high = bytes[j]/16, low = bytes[j]%16;
			buff += (high<10) ? ('0' + high) : ('a' + high - 10);
			buff += (low<10) ? ('0' + low) : ('a' + low - 10);
		}
		return buff;
	}

	const unsigned int crc_table[256] =
	{
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
		0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
		0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
		0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
		0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
		0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
		0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
		0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
		0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
		0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
		0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
		0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
		0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
		0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
		0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
		0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
		0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
		0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
		0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
		0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
		0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
		0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
		0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
		0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
		0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
		0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
		0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
		0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
		0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
		0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
		0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
		0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
		0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
		0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
		0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
		0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
		0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
		0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
		0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
		0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
		0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
		0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
		0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
		0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
		0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
		0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
		0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};

	unsigned int getcrc(unsigned char *c, unsigned int len)
	{
		unsigned int crc = 0xFFFFFFFF;
//		unsigned char     *e = c + len;
//		while (c < e) {
//			crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_table[ (crc^ *c) & 0xFF ];
//			++c;
//		}
		unsigned int i;
		for (i = 0; i < len; i++) {
			crc = crc_table[(crc ^ c[i]) & 0xff] ^ (crc >> 8);
		}
		return( crc^0xFFFFFFFF );
	}


	unsigned int crc32(unsigned int crc,unsigned char *buffer, unsigned int size)
	{
	    unsigned int i;
	    for (i = 0; i < size; i++) {
	        crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	    }
	    return crc ;
	}

	int calc_img_crc(const char *in_file, unsigned int *img_crc)
	{
	    int fd;
	    int nread;
	    int ret;
	    unsigned char buf[BUFSIZE];
	    /*第一次传入的值需要固定,如果发送端使用该值计算crc校验码,
	    **那么接收端也同样需要使用该值进行计算*/
	    unsigned int crc = 0xffffffff;

	    fd = open(in_file, O_RDONLY);
	    if (fd < 0) {
	        printf("%d:open %s.\n", __LINE__, strerror(errno));
	        return -1;
	    }

	    while ((nread = read(fd, buf, BUFSIZE)) > 0) {
	        crc = crc32(crc, buf, nread);
	    }
	    *img_crc = crc^0xFFFFFFFF;

	    close(fd);

	    if (nread < 0) {
	        printf("%d:read %s.\n", __LINE__, strerror(errno));
	        return -1;
	    }
	    return 0;
	}

	int hex_char_value(char c)
	{
	    if(c >= '0' && c <= '9')
	        return c - '0';
	    else if(c >= 'a' && c <= 'f')
	        return (c - 'a' + 10);
	    else if(c >= 'A' && c <= 'F')
	        return (c - 'A' + 10);
	    return 0;
	}
	int hex_to_decimal(const char* szHex, int len)
	{
	    int result = 0;
	    for(int i = 0; i < len; i++)
	    {
	        result += (int)pow((float)16, (int)len-i-1) * hex_char_value(szHex[i]);
	    }
	    return result;
	}

	int CRC16_XMODEM_CUSTOM(unsigned char* buffer, int len) {
		int wCRCin = 0x0000; // initial value 65535
		int wCPoly = 0x8005; // 0001 0000 0010 0001 (0, 5, 12)
		for (int j = 0; j < len; j++) {
			for (int i = 0; i < 8; i++) {
				bool bit = ((buffer[j] >> (7 - i) & 1) == 1);
				bool c15 = ((wCRCin >> 15 & 1) == 1);
				wCRCin <<= 1;
				if (c15 ^ bit) {
					wCRCin ^= wCPoly;
				}
			}
		}
		wCRCin &= 0xffff;
		return wCRCin ^= 0x0000;
	}

};


#define UTILS  	Utils::getInstance()
#endif /* JNI_BASELIB_UTILS_H_ */
