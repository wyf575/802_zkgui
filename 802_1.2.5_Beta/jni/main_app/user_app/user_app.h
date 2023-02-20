#ifndef USER_APP_H
#define USER_APP_H
#include "main_app/main_app.h"
#include "main_app/hardware_tool/user_uart.h"
#include "main_app/cJSON_file/json_app.h"
#include "main_app/ibeelink_app/ibeelink_mqtt.h"
#include "main_app/message_queue/message_queue.h"
#include "main_app/net_listener.h"

#if (USER_APP_TYPE==USER_APP_CARGO)
#include "cargo_driver.h"
#endif

#if (USER_APP_TYPE==USER_APP_MASK)
#include "mask_driver.h"
#endif

#if (USER_APP_TYPE==USER_APP_NEW_MASK)
#include "mask_driver.h"
#endif
#if (USER_APP_TYPE==USER_APP_NEW2_MASK)
#include "mask_driver.h"
#endif
void start_user_app(void);
void user_report_alarm_func(cJSON *extend);
void user_app_reply_func(post_back_t *post_back);

#endif //USER_APP_H
