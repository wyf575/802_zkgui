/*
 * NetManager.c
 *
 *  Created on: 2021年3月11日
 *      Author: Administrator
 */
#include "NetManager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFLEN 20480

int getNetConnectStatus(char* netType){
	char path[50];
	sprintf(path, "/sys/class/net/%s/carrier", netType);
	printf("path = %s\n", path);
	int skfd = open(path, O_RDONLY);
	if(skfd < 0){
		 printf("cat %s error!\n", netType);
		 return 0;
	}else
		 printf("cat %s success!\n", netType);
	char netStatus[2];
	int netRet = read(skfd, netStatus, 1);
	printf("getNetConnectStatus - %s - %d\n", netStatus, netRet);
	if(netRet > 0 && strncmp(netStatus, "1", 1) == 0){
		printf("1\n") ;
		close(skfd);
		return 1;
	}else{
		printf("0\n") ;
		close(skfd);
		return 0;
	}

}

void* netListener(NetListenerCallback callback){
	printf("---------netListener-----------\n");
	int fd, retval;
	char buf[BUFLEN] = {0};
	int len = BUFLEN;
	struct sockaddr_nl addr;
	struct nlmsghdr *nh;
	struct ifinfomsg *ifinfo;
	struct rtattr *attr;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTNLGRP_LINK;
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	while ((retval = read(fd, buf, BUFLEN)) > 0)
	{
		for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval))
		{
			if (nh->nlmsg_type == NLMSG_DONE)
				break;
			else if (nh->nlmsg_type == NLMSG_ERROR){
				printf("---------netListener-----------end\n");
				return NULL;
			}else if (nh->nlmsg_type != RTM_NEWLINK)
				continue;
			ifinfo = NLMSG_DATA(nh);
			printf("netListener - %u: %s", ifinfo->ifi_index,
					(ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );
			attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));
			len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));
			for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))
			{
				if (attr->rta_type == IFLA_IFNAME)
				{
					char* value = (char*)RTA_DATA(attr);
					printf(" %s", value);
					char cmd[80];
					if(ifinfo->ifi_flags & IFF_LOWER_UP){
						sprintf(cmd, "udhcpc -i %s -s /etc/init.d/udhcpc.script", value);
					}else{
						sprintf(cmd, "route del default gw 0.0.0.0 dev %s", value);
					}
					system(cmd);
					if(callback != NULL){
						callback(value);
					}
					break;
				}
			}
			printf("\n");
		}
	}
	printf("---------netListener----------- end\n");
	return NULL;
}

int initNetManager(NetListenerCallback callback){
	pthread_t thread_id;
	int ret = pthread_create(&thread_id, NULL, (void*)netListener, callback);
	return !ret;
}
