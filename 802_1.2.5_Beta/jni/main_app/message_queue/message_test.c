#if 0


#include "debug_printf.h"
#include "message_queue.h"
#include <pthread.h>
#include <unistd.h>


void *test1(void *arg);
void *test2(void *arg);
msg_queue_thread test_msg;
int message_queue_main(int argc, char const *argv[])
{
    #define PTHREAD_COUNT 2
    int i;
    pthread_t pthread_id[PTHREAD_COUNT];/*线程id*/
    int ret; 
    int fd = -1;
    /*防止被守护进程重复启动 注册进程清理函数*/
    //atexit(delete_file);	
    /*初始化本进程需要使用的线程队列*/
    message_queue_init(&test_msg, sizeof(int), 10);
    
    /*初始化本进程需要使用的进程消息队列*/
    /*在各线程中分别初始化*/

    /*注册本进程需要管理的线程 */
    void *pthread_func_array[PTHREAD_COUNT]={
       test1,
       test2,
    };
    /*主进程创建本进程的线程 并首次启动子进程*/
    for(i=0;(i<PTHREAD_COUNT)&&(pthread_func_array[i]!=NULL);i++){
    //for(i=0;i<PTHREAD_COUNT;i++){
        ret=pthread_create(&pthread_id[i],NULL,pthread_func_array[i],NULL);
        if(ret!=0){              
            printf("Create pthread[%d] error!\n",i); 
            exit(0);    
        }else{
            printf("Create pthread[%d] success!\n",i); 
        }
    }
    void *pthread_exit_ptr[2]={NULL,NULL};
    /*本函数最后一件事 阻塞回收本进程的线程资源*/
    for(i=0;(i<PTHREAD_COUNT)&&(pthread_func_array[i]!=NULL);i++){
        pthread_join(pthread_id[i],&pthread_exit_ptr[i]);
        if(0==i){printf("pthread_exit_int:[%s]",(char *)pthread_exit_ptr[i]);}
        if(1==i){printf("pthread_exit_int:[%d]",*(int *)pthread_exit_ptr[i]);}
    }
    return 0;
}

void *test1(void *arg){
    int send_int=1314520;
    int *pint = NULL;
    int count=0;
    for(;;){
        pint = message_queue_message_alloc_blocking(&test_msg);
        pint = &send_int;
        printf("send_int[%d] is [%d]\n",count,send_int);
        message_queue_write(&test_msg, pint);
        sleep(1);
        if(++count >10){
            break;
        }
    }
    /*仔细思考与test2的区别*/
    pthread_exit((void*)("this is a test!"));
}

void *test2(void *arg){
    int read_int=0;
    int *pint = NULL;
    int count=0;
    static int test_num = 666;/*仔细思考 为啥 且不建议这样做*/
    int *exit_code = &test_num;
    for(;;){
        pint = message_queue_read(&test_msg); 
        read_int = *pint;
        message_queue_message_free(&test_msg, pint);
        printf("read_int[%d] is [%d]\n",count,read_int);
         if(++count >10){
            break;
        }
    }
    printf("exit[%d] \n",*exit_code);
    pthread_exit((void*)exit_code);

}
static msg_queue_thread player_task_queue_id = {0};

void *handleStartPlayerThread222(void *arg)
{
    message_queue_init(&player_task_queue_id, 256, 20);
    if (player_task_queue_id.max_depth>0)
    {
        char *needCaller =  message_queue_message_alloc_blocking(&player_task_queue_id);
        strncpy(needCaller,"xxxxxxxxxxxxxxxxxxxxx",256);
        printf("request_player_start_stop[%s]\n",needCaller);
        // message_queue_write(&player_task_queue_id, needCaller);
    }
    /*读取到队列请求 读取播放列表目录 判定播放列表类型 播放文件类型*/
    while (1)
    {
        char *needCaller = message_queue_read(&player_task_queue_id); 
        printf("[thread]handleStartPlayerThread![%s]\n",needCaller);
        message_queue_message_free(&player_task_queue_id, &needCaller);
    }

}
#if 0
   // pthread_attr_t attr;
    // size_t stack_size;
    // int status;

    // ret = pthread_attr_init(&attr); /*初始化线程属性*/
    // if (ret != 0){
    //     printf("error\n");
    // }
    // ret = pthread_attr_getstacksize(&attr, &stack_size);
    // if (ret != 0){
    //     printf("error\n");
    // }else{
    //     printf(" ############### stack_size[%ld] ###############\n",stack_size);
    // }
    // ret = pthread_attr_setstacksize(&attr, 16*1024*1024);
    // if (ret != 0){
    //     printf("error\n");
    // }
       // ret = pthread_attr_getstacksize(&attr, &stack_size);
    // if (ret != 0){
    //     printf("error\n");
    // }else{
    //     printf(" ############### stack_size2[%ld] ###############\n",stack_size);
    // }
    // ret = pthread_attr_destroy(&attr); /*不再使用线程属性，将其销毁*/
    // if (ret != 0){
    //     printf("error\n");
    // }
#endif
#endif