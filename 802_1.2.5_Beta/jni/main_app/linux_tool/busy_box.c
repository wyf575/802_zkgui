#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "busy_box.h"
#define CHECK_ADD_DIR "if [ ! -d %s  ];then  mkdir %s;else echo dir exist;fi"
#define CHECK_ADD_FILE "if [ ! -f %s  ];then echo>%s;else echo file exist;fi"


/**
* @brief         : system调用返回的健全代码
* @param[in]	 : 命令
* @param[out]    : none
* @return        : 正常返回0 
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.09.18
*/

//add by wyf 20221201
typedef void (*sighandler_t)(int);
int pox_system(const char *cmd_line)
{
	int ret = 0;
	sighandler_t old_handler;
	old_handler = signal(SIGCHLD,SIG_DFL);
	ret = system(cmd_line);
	signal(SIGCHLD,old_handler);
	return ret;
}

int call_system_cmd(const char *cmd_str)
{
    pid_t status;
    status = pox_system(cmd_str);
    if (-1 == status)
    {
        printf("system error!");
    }
    else
    {
        printf("exit status value = [0x%x]\n", status);
        if (WIFEXITED(status))
        {
            if (0 == WEXITSTATUS(status))
            {
                printf("run shell script successfully.\n");
            }
            else
            {
                printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
            }
        }
        else
        {
            printf("exit status = [%d]\n", WEXITSTATUS(status));
        }
    }
    return WEXITSTATUS(status);
}

/**
* @brief         : 检查并创建文件(夹)
* @param[in]	 : 文件(夹)路径
* @param[in]	 : 0：文件类型 1：文件夹类型
* @param[out]	 : None
* @return        : 成功返回0 
* @author        : aiden.xiang@ibeelink.com
* @date          : 2019.03.14
*/
int check_creat_file_dir(const char *file_dir,checkType type)
{
    char cmd_str[200];
    if (0 == type)
    {
        sprintf(cmd_str, CHECK_ADD_FILE, file_dir,file_dir);
    }
    else if(1 == type)
    {
        sprintf(cmd_str, CHECK_ADD_DIR, file_dir,file_dir);
    }
    else
    {
        return -1;
    }
    return call_system_cmd(cmd_str);
}

int get_system_output(char *cmd, char *output, int size)
{
    /*size 推荐64或128 其他过大值可能段错误*/
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

int set_file_all_authority(const char *filePath)
{
    if(0 == access(filePath,F_OK))
    {
        chmod(filePath,S_IXGRP|S_IXOTH|S_IWOTH);
    }
    return -1;
}
int user_remove_file(const char *filename)
{
    printf("The file to delete: %s\n",filename);
    int ret = remove(filename); 
    if (ret != 0)
    {
        perror("remove");
    }
    return ret;
} 
    
 