#if 0
#include <stdio.h>
#include "sntp_client.h"
int sntp_client_main(int argc, char const *argv[])
{
    char stamp[13];
    update_sntp_time(NTP_SERVER1);
    get_system_time_stamp(stamp);
    printf("stamp[%s]",stamp);
    return 0;
}
#endif