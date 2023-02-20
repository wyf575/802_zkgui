
#if 0
#include "debug_printf.h"

/*本文件日志输出开关*/
#if 1
#define debuglog DEBUG_PRINTF
#else
#define debuglog(format,...)  
#endif

int debug_printf_main(int argc, char const *argv[]){
    debuglog("xiangwei come on!\n");
    return 0;
}

#endif
	


