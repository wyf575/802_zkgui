2019.10.21
本协议为sntp第三方客户端协议
1.提供更新系统时间为网络标准时间的接口update_sntp_time
2.提供字符串形式的十位时间戳get_system_time_stamp
3.注意：使用第一个接口时，需要系统管理员权限（涉及到系统时间操作）
测试方法：先在sntp_test.c中打开编译宏
然后使用: gcc -o test sntp_test.c sntp_client.c