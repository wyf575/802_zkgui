 
/* ntpclient.c */
#include "sntp_client.h"
#include "main_app/main_app.h"
 
in_addr_t inet_host(const char *host)
{
    in_addr_t saddr;
    struct hostent *hostent;
 
    if ((saddr = inet_addr(host)) == INADDR_NONE) 
    {
        if ((hostent = gethostbyname(host)) == NULL)
            return INADDR_NONE;
 
        memmove(&saddr, hostent->h_addr, hostent->h_length);
    }
 
    return saddr;
}
 
 
int get_ntp_packet(void *buf, size_t *size)
{
    struct ntphdr *ntp;
    struct timeval tv;
 
 
    if (!size || *size<NTP_HLEN)
        return -1;
 
    memset(buf, 0, *size);
 
    ntp = (struct ntphdr *) buf;
    ntp->ntp_li = NTP_LI;
    ntp->ntp_vn = NTP_VN;
    ntp->ntp_mode = NTP_MODE;
    ntp->ntp_stratum = NTP_STRATUM;
    ntp->ntp_poll = NTP_POLL;
    ntp->ntp_precision = NTP_PRECISION;
 
    gettimeofday(&tv, NULL);
    ntp->ntp_transts.intpart = htonl(tv.tv_sec + JAN_1970);
    ntp->ntp_transts.fracpart = htonl(USEC2FRAC(tv.tv_usec));
 
    *size = NTP_HLEN;
 
    return 0;
}
 
 
void print_ntp(struct ntphdr *ntp)
{
    time_t time;
 
    printf("LI:\t%d \n", ntp->ntp_li);
    printf("VN:\t%d \n", ntp->ntp_vn);
    printf("Mode:\t%d \n", ntp->ntp_mode);
    printf("Stratum:\t%d \n", ntp->ntp_stratum);
    printf("Poll:\t%d \n", ntp->ntp_poll);
    printf("precision:\t%d \n", ntp->ntp_precision);
 
    printf("Route delay:\t %lf \n",
        ntohs(ntp->ntp_rtdelay.intpart) + NTP_REVE_FRAC16(ntohs(ntp->ntp_rtdelay.fracpart)));
    printf("Route Dispersion:\t%lf \n",
        ntohs(ntp->ntp_rtdispersion.intpart) + NTP_REVE_FRAC16(ntohs(ntp->ntp_rtdispersion.fracpart)));
    printf("Referencd ID:\t %d \n", ntohl(ntp->ntp_refid));
 
 
    time = ntohl(ntp->ntp_refts.intpart) - JAN_1970;
    printf("Reference:\t%d %d %s \n",
        ntohl(ntp->ntp_refts.intpart) - JAN_1970,
        FRAC2USEC(ntohl(ntp->ntp_refts.fracpart)),
        ctime(&time));
 
    time = ntohl(ntp->ntp_orits.intpart) - JAN_1970;
    printf("Originate:\t%d %d frac=%d (%s) \n",
        ntohl(ntp->ntp_orits.intpart) - JAN_1970,
        FRAC2USEC(ntohl(ntp->ntp_orits.fracpart)),
        ntohl(ntp->ntp_orits.fracpart),
        ctime(&time) );
 
    time = ntohl(ntp->ntp_recvts.intpart) - JAN_1970;
    printf("Receive:\t%d %d (%s) \n",
        ntohl(ntp->ntp_recvts.intpart) - JAN_1970,
        FRAC2USEC(ntohl(ntp->ntp_recvts.fracpart)),
        ctime(&time) );
 
    time = ntohl(ntp->ntp_transts.intpart) - JAN_1970;
    printf("Transmit:\t%d %d (%s) \n",
        ntohl(ntp->ntp_transts.intpart) - JAN_1970,
        FRAC2USEC(ntohl(ntp->ntp_transts.fracpart)),
        ctime(&time) );
}
 
 
double get_rrt(const struct ntphdr *ntp, const struct timeval *recvtv)
{
    double t1, t2, t3, t4;
 
    t1 = NTP_LFIXED2DOUBLE(&ntp->ntp_orits);
    t2 = NTP_LFIXED2DOUBLE(&ntp->ntp_recvts);
    t3 = NTP_LFIXED2DOUBLE(&ntp->ntp_transts);
    t4 = recvtv->tv_sec + recvtv->tv_usec / 1000000.0;
 
    return (t4 - t1) - (t3 - t2);
}
 
 
double get_offset(const struct ntphdr *ntp, const struct timeval *recvtv)
{
    double t1, t2, t3, t4;
 
    t1 = NTP_LFIXED2DOUBLE(&ntp->ntp_orits);
    t2 = NTP_LFIXED2DOUBLE(&ntp->ntp_recvts);
    t3 = NTP_LFIXED2DOUBLE(&ntp->ntp_transts);
    t4 = recvtv->tv_sec + recvtv->tv_usec / 1000000.0;
 
    return ((t2 - t1) + (t3 - t4)) / 2;
}
 
 
int update_sntp_time(char *sntp_server)
{
    char buf[BUFSIZE];
    size_t nbytes;
    int sockfd, maxfd1;
    struct sockaddr_in servaddr;
    fd_set readfds;
    struct timeval timeout, recvtv, tv;
    double offset;
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(NTP_PORT);
    servaddr.sin_addr.s_addr = inet_host(sntp_server);
    //servaddr.sin_addr.s_addr = inet_host("119.28.183.184");
 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        perror("socket error");
        exit(-1);
    }
 
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) != 0) 
    {
        perror("connect error");
        exit(-1);
    }
 
    nbytes = BUFSIZE;
    if (get_ntp_packet(buf, &nbytes) != 0) 
    {
        fprintf(stderr, "construct ntp request error \n");
        exit(-1);
    }
    send(sockfd, buf, nbytes, 0);
 
 
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    maxfd1 = sockfd + 1;
 
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
 
    if (select(maxfd1, &readfds, NULL, NULL, &timeout) > 0) 
    {
        if (FD_ISSET(sockfd, &readfds)) 
        {
            if ((nbytes = recv(sockfd, buf, BUFSIZE, 0)) < 0) 
            {
                perror("recv error");
                exit(-1);
            }
            //计算客户端时间与服务器端时间偏移量
            gettimeofday(&recvtv, NULL);
            offset = get_offset((struct ntphdr *) buf, &recvtv);
            //更新系统时间
            gettimeofday(&tv, NULL);
            tv.tv_sec += (int) offset;
            tv.tv_usec += offset - (int) offset;
            printf("sec[%ld] usec[%ld]\n",tv.tv_sec,tv.tv_usec);
            if (settimeofday(&tv, NULL) != 0) 
            {
                perror("settimeofday error");
                exit(-1);
            }
            printf("%s \n", ctime((time_t *) &tv.tv_sec));
        }
    }
 
    close(sockfd);
 
    return 0;
}

long int get_system_time_stamp(char *stamp)
{
    struct timeval recvtv = {0};
    gettimeofday(&recvtv, NULL);
    if (NULL != stamp)
    {
        sprintf(stamp,"%ld",recvtv.tv_sec);
    }
    return recvtv.tv_sec;
}

int formart_str_time(time_t need_stamp,char *formart_num)
{
	#define CCT (+8)
	struct tm *info =NULL;
	info = gmtime(&need_stamp);
	if(NULL==info)
    {
        return -1;
    }
	sprintf(formart_num, "%02d%02d%02d%02d%02d%02d", info->tm_year - 100, \
	info->tm_mon + 1,info->tm_mday, (info->tm_hour + CCT) % 24, info->tm_min, info->tm_sec);
    return 0;
}

/**
 * @brief Get the min sec int object
 * 获取时分组合的int类型整数 
 * 如23:59为2359，00:01为1
 * @param stamp 外部提供时间错
 * @return int 组合整数
 */
int get_min_sec_int(time_t stamp)
{
    #define CCT (+8)
	struct tm *info =NULL;
	info = gmtime(&stamp);
	if(NULL==info)
    {
        return -1;
    }
    return (((info->tm_hour + CCT) % 24)*100+info->tm_min);
}
