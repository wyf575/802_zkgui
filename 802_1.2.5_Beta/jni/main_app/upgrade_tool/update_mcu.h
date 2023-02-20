#ifndef UPDATE_MCU_H
#define UPDATE_MCU_H
#include "../main_app.h"

#if (USER_APP_TYPE==USER_APP_CARGO)
void mcu_update_task(void *param);
int update_mcu_callback(char *server, int status);
const char *GetSetMcuMinVersion(char *SetVer);
#endif

#endif/*UPDATE_MCU_H*/
