#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include "UserIfconfig.h"

#define DNS_CMD "cat /etc/resolv.conf | grep nameserver | awk 'NR==1{print}' |awk '{ print $2 }'"
struct sockaddr_in sin_u;
struct sockaddr_in netmask;
struct sockaddr_in broad;
struct ifreq ifr;
unsigned char arp[6];
#include <stdio.h>

#include <arpa/inet.h>   
#include <linux/rtnetlink.h>     
#include <net/if.h> 
#include <stdlib.h> 
#include <string.h>

#define BUFSIZE 8192  

struct route_info{      
    u_int dstAddr;      
    u_int srcAddr;      
    u_int gateWay;      
char ifName[IF_NAMESIZE];     
};   

int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)     
{       
    struct nlmsghdr *nlHdr;       
    int readLen = 0, msgLen = 0;   
    do
    {   
        //收到内核的应答         
        if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0)         
        {           
            perror("SOCK READ: ");           
            return -1;         
        }   
        nlHdr = (struct nlmsghdr *)bufPtr;
        //检查header是否有效         
        if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))         
        {           
            perror("Error in recieved packet");           
            return -1;         
        } 
        if(nlHdr->nlmsg_type == NLMSG_DONE)          
        {           
            break;         
        }         
        else         
        {                      
            bufPtr += readLen;           
            msgLen += readLen;         
        }    
        if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)          
        {                    
            break;         
        }
    }while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));   
    return msgLen;    
}     


//分析返回的路由信息     
void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo,char *dev_name,char *gateway)     
{       
    struct rtmsg *rtMsg;       
    struct rtattr *rtAttr;       
    int rtLen;       
    char *tempBuf = NULL;       
    struct in_addr dst;       
    struct in_addr gate;              
    tempBuf = (char *)malloc(100);       
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);   

    // If the route is not for AF_INET or does not belong to main routing table       
    //then return.        
    if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) 
    {
        return;   
    }

    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);       
    rtLen = RTM_PAYLOAD(nlHdr);   

    for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen))
    {        
        switch(rtAttr->rta_type) 
        {        
        case RTA_OIF:         
            if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);         
            break;        
        case RTA_GATEWAY:         
            rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);         
            break;        
        case RTA_PREFSRC:         
            rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);         
            break;        
        case RTA_DST:         
            rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);        
            break;        
        } 
    } 

    dst.s_addr = rtInfo->dstAddr;       
    if(strstr((char *)inet_ntoa(dst), "0.0.0.0"))       
    {         
        // printf("oif:%s",rtInfo->ifName);
        if (NULL != dev_name)
        {
            strcpy(dev_name, rtInfo->ifName);
        }
        gate.s_addr = rtInfo->gateWay;
        if (NULL != gateway)
        {
            sprintf(gateway,"%s" ,(char *)inet_ntoa(gate));  
        }
        // printf("gw%s\n",gateway);         
        gate.s_addr = rtInfo->srcAddr;         
        // printf("src:%s\n",(char *)inet_ntoa(gate));         
        gate.s_addr = rtInfo->dstAddr;        
        // printf("dst:%s\n",(char *)inet_ntoa(gate));        
    }   

    free(tempBuf);      
    return;     
}   


int get_gateway(char *dev_name,char *gateway)
{      
    struct nlmsghdr *nlMsg;      
    struct rtmsg *rtMsg;      
    struct route_info *rtInfo;      
    char msgBuf[BUFSIZE];            
    int sock, len, msgSeq = 0;   

    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)      
    {       
        perror("Socket Creation: ");       
        return -1;      
    }                 

    memset(msgBuf, 0, BUFSIZE);                 
    nlMsg = (struct nlmsghdr *)msgBuf;      
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);                 
    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.      
    nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .            
    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.      
    nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.      
    nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.     

    if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
    {       
        printf("Write To Socket Failed…\n");       
        return -1;      
    }                  


    if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0)
    {      
        printf("Read From Socket Failed…\n");      
        return -1;      
    }   

    rtInfo = (struct route_info *)malloc(sizeof(struct route_info));   

    for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len))
    {       
        memset(rtInfo, 0, sizeof(struct route_info));       
        parseRoutes(nlMsg, rtInfo,dev_name,gateway);
    }
    free(rtInfo);      
    close(sock);      
    return 0;     
}  

static int get_system_output(char *cmd, char *output, int size)
{
    FILE *fp=NULL;  
    fp = popen(cmd, "r");   
    if (fp)
    {
		if(fgets(output, size, fp) != NULL)
        {       
            if(output[strlen(output)-1] == '\n')            
                output[strlen(output)-1] = '\0';    
        }   
        pclose(fp); 
    }
    return strlen(output);
}

static void get_local_ip(int sock_fd,ifconfig_str *ifconfig)
{
	strncpy(ifr.ifr_name, ifconfig->iface, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	if (ioctl(sock_fd, SIOCGIFADDR, &ifr) == 0)
	{
		memcpy(&sin_u, &ifr.ifr_addr, sizeof(sin_u));
		strcpy(ifconfig->ip_str,inet_ntoa(sin_u.sin_addr));
	}
}

static void get_mac_addr(int sock_fd,ifconfig_str *ifconfig)
{
	if (ioctl(sock_fd, SIOCGIFHWADDR, &ifr) == 0)
	{
		memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
		sprintf(ifconfig->mac_str,"%02X:%02X:%02X:%02X:%02X:%02X", arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
	}
}

static void get_netmask_addr(int sock_fd,ifconfig_str *ifconfig)
{
	if (ioctl(sock_fd, SIOCGIFNETMASK, &ifr) == 0)
	{
		memcpy(&netmask, &ifr.ifr_netmask, sizeof(netmask));
		strcpy(ifconfig->mask_str,inet_ntoa(netmask.sin_addr));
	}
}

int get_ifconfig_param(ifconfig_str *ifconfig)
{
	int sockfd = -1;
	get_gateway(ifconfig->iface,ifconfig->gateway);         
	get_system_output(DNS_CMD,ifconfig->dns_ip,64);
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		perror("socket");
		return -1;
	}
	get_local_ip(sockfd,ifconfig);
	get_mac_addr(sockfd,ifconfig);
	get_netmask_addr(sockfd,ifconfig);
	return 0;
}

// int main(int argc, char const *argv[])
// {
// 	ifconfig_str test = {0};
// 	get_ifconfig_param(&test);
// 	printf("%s\n%s\n%s\n%s\n%s\n%s\n", test.iface,test.ip_str,test.mac_str,test.mask_str,test.gateway,test.dns_ip);
// 	return 0;
// }

