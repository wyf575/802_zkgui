#ifndef USER_IFCONFIG_H
#define USER_IFCONFIG_H
typedef struct 
{
	char iface[20];
	char status[20];
	char gateway[20];
	char dns_ip[20];
	char ip_str[20];
	char mac_str[20];
	char mask_str[20];
}ifconfig_str;

int get_ifconfig_param(ifconfig_str *ifconfig);
int get_gateway(char *dev_name, char *gateway);

#endif //IFCONFIG_H
