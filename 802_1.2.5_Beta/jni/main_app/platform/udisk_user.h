#ifndef UDISK_USER_H
#define UDISK_USER_H

#define U_DISK_LENGTH 17
/**
 * @brief 在回调里设置路径
 * 注意：在U盘拔出后一定要同步设置为""
 * 
 */
const char *get_set_usb_path(char *setPath);
void initUDiskConfig(void);

#endif

