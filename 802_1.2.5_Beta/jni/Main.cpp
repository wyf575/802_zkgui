#include "entry/EasyUIContext.h"
#include "uart/UartContext.h"
#include "manager/ConfigManager.h"
#include "appconfig.h"
#include "main_app/main_app.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void onEasyUIInit(EasyUIContext *pContext) {
	// 初始化时打开串口
	// printf("get uart name %s\n", CONFIGMANAGER->getUartName().c_str());
#ifdef SING7_SERIAL
	UARTCONTEXT->openUart("/dev/ttyS2", 0000015);//9600
#else
	//UARTCONTEXT->openUart(CONFIGMANAGER->getUartName().c_str(), CONFIGMANAGER->getUartBaudRate());
//	UARTCONTEXT->openUart("/dev/ttyS1", 0010002);//115200
#endif
	GetPanelSetting();//honestar karl.hong add 20210826
    register_user_thread(app_main_thread, NULL, NULL);

	// printf("============ app_main[%d] is running ==============\n",app_main(0,NULL));
}

void onEasyUIDeinit(EasyUIContext *pContext) {
	printf("----onEasyUIDeinit-----\n");
	system("echo 1 > /sys/class/gpio/gpio23/value");
	UARTCONTEXT->closeUart();
	unregister_user_thread();
}

const char* onStartupApp(EasyUIContext *pContext) {
//	return "doubleScreenActivity";
	return "mainActivity";
	// return	"player1366x768Activity";

//	return "windowActivity";
//	return "playerActivity";
//	return "player1280x1024Logic";

}


#ifdef __cplusplus
}
#endif  /* __cplusplus */

