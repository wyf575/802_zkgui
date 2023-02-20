#include "playerUList.h"
#include "../linux_tool/user_util.h"

/**
 * @brief 初始化U盘播放列表
 * 
 */
static struct cListNode *uHead = NULL;	
void initPlayerUList(void)
{
    cListDestroy(&uHead);
    cListInit(&uHead);  
}

/**
 * @brief 新增一个任务到播放列表
 * 
 * @param task 新增的任务
 */
void addPlayUTaskByMediaID(const char *mediaId)
{
    playTask task = {0};
    strncpy(task.mediaId,mediaId,256);
    cListPushBack(&uHead,task);
}

/**
 * @brief 从任务列表删除一个指定任务
 * 
 * @param mediaId 需要删除的U盘文件名称(唯一)
 */
void delPlayUTaskByMediaId(const char *mediaId)
{
    playTask task = {0};
    strncpy(task.mediaId,mediaId,256);
    cListRemoveByMediaID(&uHead,task);
}

/**
 * @brief Get the Next Task object
 * 根据当前文件名称获取下一个任务地址
 * @param mediaId 当前U盘文件名称(唯一)
 * @return playTask* 下一个任务地址
 */
playTask *getNextUTaskByMediaId(const char *mediaId)
{
    playTask task = {0};
    strncpy(task.mediaId,mediaId,256);
    return cListNextByMediaID(uHead, task);
}

int matchEndStr(const char *fileName)
{
    const char *matchStr[4] = {".mp4",".avi",".MP4",".AVI"};
    for (size_t i = 0; i < 4; i++)
    {
        if(1 == is_end_with((const char*)fileName,(char *)matchStr[i]))
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Get the Files Name By Clist object
 * 
 * @param dirPath 
 * @param files 
 * @return int 
 */
static int getFilesNameByClist(char *dirPath)
{
    DIR *dir = opendir(dirPath);
    int count = 0;
    if (dir == NULL)
    {
        return 0;
    }
    //进入到当前读取目录
    if(0 != chdir(dirPath))
    {
        return 0;
    } 
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        struct stat st;
        stat(ent->d_name, &st);
        if(!S_ISDIR(st.st_mode))
        {
            // printf("=====[%s]=====\n",ent->d_name);
            if (count<100)
            {
                if ( 1 != matchEndStr(ent->d_name) )
                {
                    continue;
                }
                char tmpStr[512] = {0};
                snprintf(tmpStr,512,"%s/%s",dirPath,ent->d_name);
                // printf("%s\n ",tmpStr);
                addPlayUTaskByMediaID(tmpStr);
                count++;
            }
        }
    }
    closedir(dir);
    return count;
}

/**
 * @brief 根据任务id查找该任务
 * 
 * @param taskID 任务ID
 * @return playTask* 返回任务地址
 */
playTask *findByMediaID(char *MediaID)
{
    playTask task = {0};
    strncpy(task.mediaId,MediaID,sizeof(task.mediaId));
    cListNode *node = cListFindByMediaID(uHead,task);
    if (NULL != node)
    {
        return &(node->data);
    }
    return NULL;
}

/**
 * @brief 扫描U盘目录下的文件
 * 每次插拔时调用
 * @param dirPath U盘路径
 * @return int 文件个数
 */
int scanUDiskFile(char *dirPath)
{    
    initPlayerUList();
    int count = getFilesNameByClist(dirPath);
    printRAMUMediaID("scanUDiskFile");
    return count;
}

/**
 * @brief 获取下一个即将播放的广告
 * @caller 供控制中心调用
 * @return const char* 
 */
const playTask *getUListNextPlayTask(void)
{
    static bool firstCall = true;
    static playTask *base = NULL;
    if (firstCall)
    {
        /*首次调用时 获取头节点*/
        firstCall = false;
        base = cListDataByNode(uHead);
    }else
    {
        base = cListNextByMediaID(uHead, *base);
    }
    printRAMUMediaID("getUListNextPlayTask");
    if (NULL == base)
    {
        return NULL;
    }
    return (const playTask *)base;
}

/**
 * @brief 给外部提供打印mediaID接口
 * 
 * @param caller 外部调用者
 */
void printRAMUMediaID(char *caller)
{
    printf("\n*************Ucaller is [%s]",caller);
    cListPrintByMediaID(uHead);
}

/**
 * @brief 给外部提供打印taskID接口
 * 
 * @param caller 外部调用者
 */
void printRAMUTaskID(char *caller)
{
    printf("\n*************Ucaller is [%s]",caller);
    cListPrintByTaskID(uHead);
}
