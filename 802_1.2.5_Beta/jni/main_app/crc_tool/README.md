测试方法：
gcc -o test crc32_test.c usr_file.c crc32.c
2019.11.15
1.新增crc16_hc-iap校验

2019.11.14
1.新增crc16_modbus校验

2019.11.03
1.文件CRC32校验时分包大小请在头文件中宏定义FILE_SUB_SIZE设置

2019.10.30
目前支持如下功能：
1.整块数据CRC32校验
2.文件CRC分包校验
3.蜜连合成的CRC32分包校验
