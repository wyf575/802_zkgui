#include "main_app/linux_tool/mt_timer.h"
#include "main_app/message_queue/message_queue.h"
#include "main_app/debug_printf/debug_printf.h"
#include "user_uart.h"

static msg_queue_thread uart_task_queue_id = {0};
static int user_uart_id = -1;

static void complete_uart_recv(void *arg)
{
    if (((_sysUartDate *)arg)->length <= 0)
    {
        debuglog_red("there no data receive!\n");
    }else
    {
        _sysUartDate *d =  message_queue_message_alloc_blocking(&uart_task_queue_id);
	
        d->length = (((_sysUartDate *)arg)->length)%CACHE_LEN;
        memcpy(d->data,((_sysUartDate *)arg)->data,d->length);
        
        memset(((_sysUartDate *)arg)->data,0,CACHE_LEN);
        ((_sysUartDate *)arg)->length = 0;
        
        message_queue_write(&uart_task_queue_id, d);
    }
}

void *user_uart_init_thread(void *arg)
{
	_sysUartDate d= {0};
	message_queue_init(&uart_task_queue_id, sizeof(_sysUartDate), 10);

    uart_init_t param = {"/dev/ttyS1",-1,1,9600,8,'n',1,true};
    user_uart_id = uart_init(&param);//只能配置为同步 该参数不开放 以免出错

	while (1)
	{
        printf("5min[%s]5min\n",__FUNCTION__);
		unsigned char tmp[1024] = {0};//满内存后覆盖前面的数据
		uart_simple_timer_start(50,complete_uart_recv,&d);		
		int readlen = read(user_uart_id, tmp, sizeof(tmp));
		if (readlen>0)
		{
			uart_simple_timer_end();
            int offset = (d.length%CACHE_LEN);
            int copyCount = (readlen+(d.length%CACHE_LEN)<CACHE_LEN)?readlen:(CACHE_LEN-(d.length%CACHE_LEN));
            d.length = d.length + copyCount;
            // debuglog_yellow("offset[%d] copyCount[%d]\n",offset,copyCount);
			memcpy(d.data+offset,tmp,copyCount);
            // debuglog_red("read[%d][%s][%d][%s]\n",readlen,tmp,d.length,d.data);
        }
	}
}

int user_uart_receive(void *data, int needLen)
{
    while (uart_task_queue_id.max_depth<=0)
    {
        usleep(500000);
    }
	_sysUartDate *d = message_queue_read(&uart_task_queue_id); 
	needLen = needLen>d->length?d->length:needLen;
	memcpy(data,d->data,needLen);
	message_queue_message_free(&uart_task_queue_id, d);
	return needLen;
}

int user_uart_send(int uartID, void *data, int needLen)
{
    if (user_uart_id<=0)
    {
        return 0;
    }
    for (size_t i = 0; i < needLen; i++)
    {
        printf("s[%d][%02X] ", i,((unsigned char *)data)[i]);
    }
    
    return uart_send(user_uart_id, data, needLen);
}

#if 0
void *uart_test_task(void *arg)
{
    while (1)
    {
        unsigned char data[1024] = {0};
        int len = user_uart_receive(0,data,sizeof(data));
        if (len>0)
        {
            debuglog_yellow("[%d][%s]\n",len,data);
            user_uart_send(0,data,len);
        }else
        {
            debuglog_yellow("NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN[%s]\n",data);
        }
    }
}
#endif
