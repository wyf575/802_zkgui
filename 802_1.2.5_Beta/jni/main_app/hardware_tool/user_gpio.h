#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "main_app/main_app.h"
int init_rs485_control(void);
int rs485_send_control(void);
int rs485_recv_control(void);

int init_audio_control(void);
int audio_open_control(void);
int audio_close_control(void);

int modem_init_control(void);
int modem_open_control(void);

//honestar karl.hong add 20210918
int modem_poweron(void);
int modem_poweroff(void);
//honestar karl.hong add 20210918


#endif // !USER_GPIO_DRIVER_H
