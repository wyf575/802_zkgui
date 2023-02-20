/**
* @file      : debug_printf.h  
* @brief     : 供外部使用的调试信息输出 无源文件
* @author    : aiden.xiang@ibeelink.com
* @date      : 2019.09.05
* @version   : A001 后续版本会增加颜色及等级
* @copyright : ibeelink                                                              
*/

#ifndef DEBUG_PRINTF_H
#define DEBUG_PRINTF_H
#include "main_app/main_app.h"

extern "C"
{
#if 0
/*设置输出前景色*/
#define PRINT_FONT_BLA printf("\033[30m"); //黑色
#define PRINT_FONT_RED printf("\033[31m"); //红色
#define PRINT_FONT_GRE printf("\033[32m"); //绿色
#define PRINT_FONT_YEL printf("\033[33m"); //黄色
#define PRINT_FONT_BLU printf("\033[34m"); //蓝色
#define PRINT_FONT_PUR printf("\033[35m"); //紫色
#define PRINT_FONT_CYA printf("\033[36m"); //青色
#define PRINT_FONT_WHI printf("\033[37m"); //白色
/*设置输出背景色*/
#define PRINT_BACK_BLA printf("\033[40m"); //黑色
#define PRINT_BACK_RED printf("\033[41m"); //红色
#define PRINT_BACK_GRE printf("\033[42m"); //绿色
#define PRINT_BACK_YEL printf("\033[43m"); //黄色
#define PRINT_BACK_BLU printf("\033[44m"); //蓝色
#define PRINT_BACK_PUR printf("\033[45m"); //紫色
#define PRINT_BACK_CYA printf("\033[46m"); //青色
#define PRINT_BACK_WHI printf("\033[47m"); //白色
/*输出属性设置*/
#define PRINT_ATTR_REC printf("\033[0m");   //重新设置属性到缺省设置
#define PRINT_ATTR_BOL printf("\033[1m");   //设置粗体
#define PRINT_ATTR_LIG printf("\033[2m");   //设置一半亮度(模拟彩色显示器的颜色)
#define PRINT_ATTR_LIN printf("\033[4m");   //设置下划线(模拟彩色显示器的颜色)
#define PRINT_ATTR_GLI printf("\033[5m");   //设置闪烁
#define PRINT_ATTR_REV printf("\033[7m");   //设置反向图象
#define PRINT_ATTR_THI printf("\033[22m");  //设置一般密度
#define PRINT_ATTR_ULIN printf("\033[24m"); //关闭下划线
#define PRINT_ATTR_UGLI printf("\033[25m"); //关闭闪烁
#define PRINT_ATTR_UREV printf("\033[27m"); //关闭反向图象
#endif

#include <stdio.h>
#include <stdlib.h>

//错误代码
typedef enum
{
    ERR_NONE = 0,     //没有错误
    ERR_NETWORK,      //网络错误
    ERR_SEND_DATA,    //发送失败
    ERR_GET_DATA,     //获取数据错误
    ERR_INIT,         //初始化
    ERR_PARM,         //参数错误
    PACKAGE_ASCII,    //包的ID错误
    ERR_UNDEFINE,     //未定义错误
    ERR_OOM,          //内存溢出错误
    ERR_WRITE,        //写入错误
    ERR_ACCESS,       //文件没有权限或不存在
    ERR_CHECK_FAILED, //校验错误
    UNKNOW            //未知错误
} ERR_CODE;

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG_PRINTF(format, ...) printf("[" __FILE__              \
":%s L:%d ] " format "", \
__FUNCTION__, __LINE__, ##__VA_ARGS__)

#define debuglog_red(format, ...) printf("\033[31m [" __FILE__              \
":%s L:%d ] " format " \033[0m ", \
__FUNCTION__, __LINE__, ##__VA_ARGS__)

#define debuglog_green(format, ...) printf("\033[32m [" __FILE__              \
":%s L:%d ] " format " \033[0m ", \
__FUNCTION__, __LINE__, ##__VA_ARGS__)

#define debuglog_yellow(format, ...) printf("\033[33m [" __FILE__              \
":%s L:%d ] " format " \033[0m ", \
__FUNCTION__, __LINE__, ##__VA_ARGS__)

#else
#define DEBUG_PRINTF(format, ...) 
#define debuglog_red(format, ...) 
#define debuglog_green(format, ...) 
#endif

#define DEBUG_FUNC_CHECK(x)                                    \
    do                                                         \
    {                                                          \
        ERR_CODE __err_rc = (x);                               \
        if (__err_rc != ERR_NONE)                              \
        {                                                      \
            DEBUG_PRINTF("sorry,FUNC error[%d]!\n", __err_rc); \
            return __err_rc;                                   \
        }                                                      \
    } while (0);
#define DEBUG_LOGIC_CHECK(x)                      \
    do                                            \
    {                                             \
        if ((x))                                  \
        {                                         \
            DEBUG_PRINTF("sorry,LOGIC error!\n"); \
            return ERR_PARM;                      \
        }                                         \
    } while (0);
#define DEBUG_NULL_CHECK(x)                      \
    do                                           \
    {                                            \
        if ((x == NULL))                         \
        {                                        \
            DEBUG_PRINTF("sorry,NULL error!\n"); \
            return ERR_PARM;                     \
        }                                        \
    } while (0);
#define DEBUG_FUNC_CHECK_N(x)                                  \
    do                                                         \
    {                                                          \
        ERR_CODE __err_rc = (x);                               \
        if (__err_rc != ERR_NONE)                              \
        {                                                      \
            DEBUG_PRINTF("sorry,FUNC error[%d]!\n", __err_rc); \
        }                                                      \
    } while (0);
#define DEBUG_LOGIC_CHECK_N(x)                    \
    do                                            \
    {                                             \
        if ((x))                                  \
        {                                         \
            DEBUG_PRINTF("sorry,LOGIC error!\n"); \
        }                                         \
    } while (0);
#define DEBUG_NULL_CHECK_N(x)                    \
    do                                           \
    {                                            \
        if ((x == NULL))                         \
        {                                        \
            DEBUG_PRINTF("sorry,NULL error!\n"); \
        }                                        \
    } while (0);
#define DEBUG_LOGIC_CHECK_V2(x, y) \
    do                             \
    {                              \
        if ((x))                   \
        {                          \
            debuglog_red(y);       \
            return ERR_PARM;       \
        }                          \
    } while (0);
}

// #include "debug_printf.h"
// /*本文件日志输出开关*/
// #if 1
// #define debuglog DEBUG_PRINTF
// #else
// #define debuglog(format,...)
// #endif
#endif /*DEBUG_PRINTF_H*/