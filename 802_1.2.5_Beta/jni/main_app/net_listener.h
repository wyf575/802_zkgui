#ifndef NET_LISTENER_H
#define NET_LISTENER_H


typedef enum {
    NET_STATUS_UNKNOW = 0,  /*网络状态未知*/
    NET_STATUS_OK ,         /*网络状态正常 可以进行网络交互*/
    NET_STATUS_FAILE,       /*网络状态异常 不能进行网络交互*/
}net_status_type;


int get_network_status(void);
void *keep_net_thread(void *arg);
int get_app_network_status(void);
void set_app_network_status(net_status_type status);



#endif // !NET_LISTENER_H
