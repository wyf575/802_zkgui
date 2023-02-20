#include "playerNList.h"
#include "../sntp_client/sntp_client.h"
#include "../linux_tool/busy_box.h"
#include "../linux_tool/user_util.h"
#include "../cJSON_file/json_app.h"
#include "../cJSON_file/read_json_file.h"
#include "../ibeelink_app/ibeelink_http.h"
#include "../device_info.h"
#include "../main_app.h"
typedef enum 
{
    REPORT_SUCCESS = 0,
    REPORT_FAILED,
    REPORT_DELETE
}reportType;

#define TASK_MAX_NUM 10
typedef struct 
{
    int successID[TASK_MAX_NUM];
    int failedID[TASK_MAX_NUM]; 
    int deleteID[TASK_MAX_NUM];
    int successIndex;
    int failedIndex;
    int deleteIndex;
}reportRes;

static reportRes report = {0};
/**
 * @brief 初始化播放列表
 * 
 */
static struct cListNode *nHead = NULL;	
void initPlayerNList(void)
{
    cListDestroy(&nHead);
    cListInit(&nHead);  
}

static void clearReportData(void)
{
    report.successIndex = 0;
    report.failedIndex = 0;
    report.deleteIndex = 0;
}


/**
 * @brief 添加上报的任务ID
 * @param type 上报任务的类型
 */
static void addTaskID2Report(int taskID,reportType type)
{
    /*防止上报数量超过数组大小 上报清空*/
    if (report.successIndex>=10 || report.failedIndex>=10 ||report.deleteIndex>=10)
    {
        reportADResult();
        clearReportData();
    }
    
    switch (type)
    {
    case REPORT_SUCCESS:
        for (size_t i = 0; i < report.successIndex; i++)
        {
            if (report.successID[i]==taskID)
            {
                return;
            }
        }
        report.successID[report.successIndex%TASK_MAX_NUM]=taskID;
        report.successIndex++;
        break;
    case REPORT_FAILED:
        for (size_t i = 0; i < report.failedIndex; i++)
        {
            if (report.failedID[i]==taskID)
            {
                return;
            }
        }
        report.failedID[report.failedIndex%TASK_MAX_NUM]=taskID;
        report.failedIndex++;
        break;
    case REPORT_DELETE:
        for (size_t i = 0; i < report.deleteIndex; i++)
        {
            if (report.deleteID[i]==taskID)
            {
                return;
            }
        }
        report.deleteID[report.deleteIndex%TASK_MAX_NUM]=taskID;
        report.deleteIndex++;
        break;
    default:
        break;
    }
}



/**
 * @brief 上报广告任务处理结果
 * 
 */
void reportADResult(void)
{
    // if (0 != report.successIndex+report.failedIndex+report.deleteIndex)
    // {
    //     /* 暂且注释 即使为空也要上报 做到事事有回响 若关心流量时再打开*/
    // }
    cJSON *root = cJSON_CreateObject(); // 创建根  
    cJSON_AddItemToObject(root, "success", cJSON_CreateIntArray(report.successID, report.successIndex>TASK_MAX_NUM?TASK_MAX_NUM:report.successIndex));  
    cJSON_AddItemToObject(root, "failed", cJSON_CreateIntArray(report.failedID, report.failedIndex>TASK_MAX_NUM?TASK_MAX_NUM:report.failedIndex));  
    cJSON_AddItemToObject(root, "delete", cJSON_CreateIntArray(report.deleteID, report.deleteIndex>TASK_MAX_NUM?TASK_MAX_NUM:report.deleteIndex));  
    cJSON_AddStringToObject(root,"deviceId",UnGetMD5DeviceId());
    clearReportData();
    char *rootStr = cJSON_PrintUnformatted(root); 
    replyADResult(rootStr);
    free(rootStr);
}


/**
 * @brief 判定网络列表当前视频是否在本地存在
 * 
 * @param mediaId 视频文件名称
 * @return true 本地存在
 * @return false 本地不存在
 */
bool isExistLocal(const char *mediaId)
{
    if (NULL == mediaId)
    {
        printf("filePath is NULL!\n");
        return false;
    }
    char tmp[512] = {0};
    snprintf(tmp,512,"%s/%s",USER_NLIST_ROOT,mediaId);
    if(0 == access(tmp,F_OK))
    {
        return true;
    }
    return false;
}


/**
 * @brief 判定当前视频是否过期(或未开始)
 * 过期需要调用者删除 并通知服务器
 * @param dateStart 13位时间戳
 * @param dateEnd 13位时间戳
 * @return true 过期
 * @return false 未过期
 */
bool isExpiredDate(long int dateStart,long int dateEnd)
{
    long int currentTime = get_system_time_stamp(NULL); 
    if (currentTime<1620725428)
    {
        /* 表明时间未同步 无网络*/
        printf("Time out of sync[%ld]!\n",currentTime);
        return false;
    }
    if (currentTime*1000 > dateEnd)
    {
        printf("isExpiredDate currentTime[%ld] dateEnd[%ld]!\n",currentTime,dateEnd);
        return true;
    }
    return false;
}


/**
 * @brief 判定当前视频是否在当天播放时间段
 * 在播放时间段内 则调用播放器播放该视频
 * @param timeStart 时分的0到4位整数(23:59为2359)
 * @param timeEnd 时分的0到4位整数(00:01为1)
 * @return true 处于播放时间段
 * @return false 不处于播放时间段
 */
bool isPlayPeriod(long int dateStart,int timeStart,int timeEnd)
{
    long int currentTime = get_system_time_stamp(NULL); 
    if (currentTime<1620725428)
    {
        /* 表明时间未同步 无网络*/
        printf("currentTime[%ld] dateStart[%ld]!\n",currentTime,dateStart);
        return true;
    }
    int timeX = get_min_sec_int(currentTime);
    if (timeX <timeStart || timeX > timeEnd || currentTime < dateStart/1000)
    {
        printf("timeX[%d] timeStart[%d] timeEnd[%d] currentTime[%ld] dateStart[%ld]!\n",timeX,timeStart,timeEnd,currentTime,dateStart);
        return false;
    }
    return true;
}

/**
 * @brief 判定添加当前视频后是否超出总内存限制
 * 超出总内存 则拒绝下载该视频 并回复服务器
 * @param mediaSize 
 * @return true 溢出 拒绝下载 预留50M空间给OTA
 * @return false 未溢出可以下载
 */
bool isOverflowFree(long int mediaSize,const char *dataRoot)
{
    /* 获取系统可用内存 */
    char tmp[20] = {0};
    char cmd[356] = {0};
    snprintf(cmd,356,"df %s | sed -n \"2p\" | awk '{print $4}'",dataRoot);
    get_system_output(cmd,tmp,64);//计算结果是KB
    if ((mediaSize+51200) > atol(tmp))
    {
        printf("(mediaSize+1)[%ld] > atol(tmp)[%ld] !\n",(mediaSize+1),atol(tmp));
        return true;
    }
    return false;
}

/**
 * @brief 获取整个播放列表中视频总大小KB
 * 注意：蜜连平台下发的大小单位为B
 * @param ppFirst 链表首节点
 * @return long int 返回整个列表中视频大小
 */
long int getListAllSize(void)
{
    return cListMediaSize(nHead);
}

/**
 * @brief 根据任务id查找该任务
 * 
 * @param taskID 任务ID
 * @return playTask* 返回任务地址
 */
playTask *findByTaskID(int taskID)
{
    playTask task = {0};
    task.taskId = taskID;
    cListNode *node = cListFindByTaskID(nHead,task);
    if (NULL != node)
    {
        return &(node->data);
    }
    return NULL;
}

/**
 * @brief 新增一个任务到播放列表
 * 
 * @param task 新增的任务
 */
void addPlayTask(playTask task)
{
    cListPushBack(&nHead,task);
} 

/**
 * @brief 从任务列表删除一个指定任务
 * 
 * @param taskID 需要删除的任务ID
 */
void delPlayTaskByID(int taskID)
{
    playTask task = {0};
    task.taskId = taskID;
    cListRemoveByTaskID(&nHead,task);
}

/**
 * @brief Get the Next Task object
 * 根据当前任务ID获取下一个任务地址
 * @param taskID 当前任务ID
 * @return playTask* 下一个任务地址
 */
playTask *getNextTask(int taskID)
{   
    playTask task = {0};
    task.taskId = taskID;
    return cListNextByTaskID(nHead, task);
}

/**
 * @brief 删除本地无效的文件
 * @caller 网络任务下发/播放任务过期
 * @attention 无效即RAM中已经被剔除而本地依然存在
 */
extern const char *get_set_current_media(char *media);
void stopPlayVideo(void) ;
bool deleteInvalidFiles(void)
{
    //1.轮询本地任务文件名
    //2.匹配RAM链表 若无则删除本地文件
    DIR *dir = opendir(USER_NLIST_ROOT);
    if (dir == NULL)
    {
        return 0;
    }
    bool needStart = false;
    //进入到当前读取目录
    if(0!=chdir(USER_NLIST_ROOT))
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
            playTask task = {0};
            strncpy(task.mediaId,ent->d_name,256);
            if (NULL == cListFindByMediaID(nHead,task))
            {
                if (strstr(get_set_current_media(NULL),ent->d_name))
                {
                    stopPlayVideo();
                    UnSleep_S(1);
                    needStart = true;
                }
                printf("deleteInvalidFiles[%s]\n",task.mediaId);
                user_remove_file(ent->d_name);
            }
        }
    }
    closedir(dir);
    return needStart;
}

/**
 * @brief 解析网络播放任务-解析广告列表信息
 * @caller 控制中心
 * @attention 负责将原始数据转换成RAM链表
 * @attention 负责将RAM链表转换成JSON组以及文件
 * @return true 需要停启播放器避免文件删除出错
 * @return false 不需要
 */
bool parseNListparams(cJSON *playList)
{
    int palyCountOnJSON = cJSON_GetArraySize(playList);
    //1. 网络去同步本地 本地RAM只增不减
    for (size_t i = 0; i < palyCountOnJSON; i++)
    {
        cJSON *item = cJSON_GetArrayItem(playList, i);
        playItem2RAMList(item);
    }
    //2. 本地去同步网络 删除合法广告且上报 本地RAM只减不增
    bool result = RAMListSyncItem(playList);

    //3.将最终的RAM链表存储起来 保证启动后无网络也能播放
    playRAM2JSONListAndFile();
    return result;
}

/**
 * @brief 将JSON转换成RAM链表
 * @caller 列表线程请求/file转换RAM请求
 * @param item 输入
 * @param base 输出：列表线程则传入临时值 而file转换为全局
 */
void JSON2RAMFunction(cJSON *item,playTask *base)
{
    parse_json_table(item,J_Long,"dateStart",&base->dateStart);
    parse_json_table(item,J_Long,"dateEnd",&base->dateEnd);
    parse_json_table(item,J_Int,"timeStart",&base->timeStart);
    parse_json_table(item,J_Int,"timeEnd",&base->timeEnd);
    parse_json_table(item,J_Int,"interval",&base->interval);
    parse_json_table(item,J_Int,"taskId",&base->taskId);
    parse_json_table(item,J_Int,"mediaType",&base->mediaType);
    parse_json_table(item,J_String,"mediaId",base->mediaId);
    parse_json_table(item,J_String,"taskName",base->taskName);
    parse_json_table(item,J_Long,"mediaSize",&base->mediaSize);
}

/**
 * @brief 单个任务格式转换
 * id号检查 本地存在 内存余量
 * @param item 单任务JSON
 * @caller 网络下发任务/启动时读取本地JSON
 * @attention 能下发的JSON必然未过期
 * @attention 可能触发下载视频和图片的操作
 * @return 0: 数据有效 其他：数据无效
 */
int playItem2RAMList(cJSON *item)
{
    playTask base = {0};
    JSON2RAMFunction(item,&base);

    //id号检查 本地是否存在 内存余量
    printRAMNTaskID("playItem2RAMList");
    playTask *ram = findByTaskID(base.taskId);
    printf("base.taskId[%d] check RAM[%p] and LocalFile[%d]\n",base.taskId,ram,isExistLocal(base.mediaId));
    if (NULL != ram && isExistLocal(base.mediaId))
    {
        //RAM和本地都存在该任务 更新时间参数 和图片轮播时间
        ram->dateEnd = base.dateEnd;
        ram->dateStart = base.dateStart;
        ram->timeEnd = base.timeEnd;
        ram->timeStart = base.timeStart;
        ram->interval = base.interval;
        addTaskID2Report(base.taskId,REPORT_SUCCESS);
        printf("#################本地和RAM都存在该任务 更新时间参数###########\n"); 
        return 0;
    }
    //其他情况一律视为新任务 即使本地存在也将覆盖
    // RAM Y LOCAL Y    ：上面已处理
    // RAM Y LOCAL N    ：删除RAM重新下载
    // RAM N LOCAL Y    ：覆盖本地
    // RAM N LOCAL N    ：正常情况 新增任务
    delPlayTaskByID(base.taskId);
    if (isOverflowFree(base.mediaSize/1024,USER_NLIST_ROOT))
    {
        /* 添加失败列表到服务器回复中 */
        printf("base.taskId[%d] mediaSize[%ld] REPORT_FAILED!!!!!!!!!!\n",base.taskId,base.mediaSize/1024);
        addTaskID2Report(base.taskId,REPORT_FAILED); 
        return -1;
    }
    //TODO：开始下载任务 转到ibeelink_http.c中进行
    char downURL[256] = {0};
    if (VIDEO_TASK == base.mediaType)
    {
        parse_json_table(item,J_String,"mediaResource",downURL);
    }else if (PICTURE_TASK == base.mediaType)
    {
        cJSON *picResource = cJSON_GetObjectItem(item,"picResource");
        cJSON *picItem0 = cJSON_GetArrayItem(picResource, 0);
        snprintf(downURL,256,"%s",picItem0->valuestring);
    }
    downloadTaskFile(downURL,base.mediaId);
    //添加任务到RAM中
    addPlayTask(base);
    if (NULL != findByTaskID(base.taskId) && isExistLocal(base.mediaId))
    {
        addTaskID2Report(base.taskId,REPORT_SUCCESS); 
    }else
    {
        addTaskID2Report(base.taskId,REPORT_FAILED); 
    }
    // printRAMNMediaID("playItem2RAMList");
    return 0;
}

/**
 * @brief 网络列表中是否存在某个任务ID
 * 
 * @param playList 
 * @param taskID 
 * @return true 
 * @return false 
 */
bool playListHaveTaskID(cJSON *playList,int taskID)
{
    int itemID = 0;
    int palyCountOnJSON = cJSON_GetArraySize(playList);
    for (size_t i = 0; i < palyCountOnJSON; i++)
    {
        cJSON *item = cJSON_GetArrayItem(playList, i);
        parse_json_table(item,J_Int,"taskId",&itemID);
        if (itemID == taskID)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief 本地RAM去同步网络列表
 * @attention 删除本地文件前请先停止播放
 * @attention 删除操作再控制中心那层
 * @return true 需要删除本地文件
 * @return false 不需要
 */
bool RAMListSyncItem(cJSON *playList)
{
    int count = getNListCount();
    printRAMNTaskID("RAMListSyncItem");
    playTask *base = cListDataByNode(nHead);
    bool result = false;
    for (size_t i = 0; i < count && base != NULL; i++)
    {
        int tmpID = base->taskId;
        base = getNextTask(base->taskId);
        if(false == playListHaveTaskID(playList,tmpID))
        {
            delPlayTaskByID(tmpID);
            addTaskID2Report(tmpID,REPORT_DELETE); 
            result = true;
        }
    }
    return result;
}

/**
 * @brief RAM链表转JSON数组和JSON文件
 * @caller 过期删除任务后/网络下发任务后
 * @return int 
 */
void playRAM2JSONListAndFile(void)
{
    int count = getNListCount();
    cJSON *arrayRoot = cJSON_CreateArray();
    playTask *base = cListDataByNode(nHead);
    //遍历RAM链表 转换成JSON数组 并存储到文件中
    for (size_t i = 0; i < count && base != NULL; i++)
    {
        cJSON *item = cJSON_CreateObject();
        create_json_table(item,J_Long,"dateStart",base->dateStart);
        create_json_table(item,J_Long,"dateEnd",base->dateEnd);
        create_json_table(item,J_Int,"timeStart",base->timeStart);
        create_json_table(item,J_Int,"timeEnd",base->timeEnd);
        create_json_table(item,J_Int,"taskId",base->taskId);
        create_json_table(item,J_Int,"interval",base->interval);
        create_json_table(item,J_Int,"mediaType",base->mediaType);
        create_json_table(item,J_String,"mediaId",base->mediaId);
        create_json_table(item,J_String,"taskName",base->taskName);
        create_json_table(item,J_Long,"mediaSize",base->mediaSize);
        create_json_table(arrayRoot,J_Array,item);
        base = getNextTask(base->taskId);
    } 
    char *playList = cJSON_PrintUnformatted(arrayRoot);
    unsigned long buffLength = 250*count;
    char buffLengthStr[20] = {0};
    snprintf(buffLengthStr,20,"%lu",buffLength);

    printf("@@@@@@@@@@@@@save[%s][%s]\n",buffLengthStr,playList);
    save_pairs_file("PLAY_BUFF",buffLengthStr,PLAY_BUFF_FILE);
    save_pairs_fileV2("PLAY_COUNT",&count,INT_TYPE,PLAY_BUFF_FILE);
    save_pairs_fileV3("PLAY_LIST",playList,CHAR_TYPE,buffLength,PLAY_LIST_FILE);
    call_system_cmd("sync");
    free(playList);
    cJSON_Delete(arrayRoot);
}

/**
 * @brief JSON组文件转换为RAM链表
 * @caller 设备启动 只是单纯的转换 不做任何处理
 * @attention 保证无网络的情况下能播放任务
 * @attention 预留待调试
 */
bool JSONListFile2playRAM(void)
{
    char buffLengthStr[20] = {0};
    read_pairs_file("PLAY_BUFF",buffLengthStr,PLAY_BUFF_FILE);
    long int buffLength = atol(buffLengthStr);
    char *playListStr = (char *)malloc(buffLength);
    memset(playListStr,0,buffLength);

    read_pairs_fileV3("PLAY_LIST",playListStr,CHAR_TYPE,buffLength,PLAY_LIST_FILE);
    cJSON *playList = cJSON_Parse(playListStr);
    free(playListStr);

    if (NULL != playList)
    {
        int palyCountOnJSON = cJSON_GetArraySize(playList);
        //1.逐个任务进行校验和格式转换
        for (size_t i = 0; i < palyCountOnJSON; i++)
        {
            cJSON *item = cJSON_GetArrayItem(playList, i);
            playTask base = {0};
            JSON2RAMFunction(item,&base);
            addPlayTask(base);
        }
        printRAMNMediaID("JSONListFile2playRAM");
        cJSON_Delete(playList);
        return deleteInvalidFiles();
    }
    return false;
}

/**
 * @brief 获取Nlist列表中的长度
 * 
 * @return int 
 */
int getNListCount(void)
{
    return cListNodeCount(nHead);
    // int ramCount = cListNodeCount(nHead);
    // int fileCount = getTypeFilesCount(USER_NLIST_ROOT,NULL,NULL);
    // return (ramCount>fileCount?ramCount:fileCount);
}

/**
 * @brief 对即将要播放的任务校验
 * 
 * @attention 内部做的处理外部无感
 * @param base 
 * @return true 可以播放
 * @return false 不能播放
 */
bool currentTaskCheck(playTask *base)
{
    if (false == isExistLocal(base->mediaId) || isExpiredDate(base->dateStart,base->dateEnd))
    {
        /*当前将要播放的广告 本地不存在或者过期 均要删除*/
        addTaskID2Report(base->taskId,REPORT_DELETE);
        delPlayTaskByID(base->taskId); 
        reportADResult();
        deleteInvalidFiles();
        return false;
    } 
    if (isPlayPeriod(base->dateStart,base->timeStart,base->timeEnd))
    {
        return true;
    }
    return false;
}

/**
 * @brief 给getUListNextPlayTask打补丁
 * @attention 
 */
static bool firstCall = true;
static bool firstCallForPic = true;
void resetNFirstCall(void)
{
    firstCall = true;
	firstCallForPic = true;
}

/**
 * @brief 获取下一个即将播放的广告
 * @caller 供控制中心调用
 * @return const char* 
 */

const playTask *getNListNextPlayTask()
{
    static int nextID = 0;
    playTask *base = NULL;
    int ramCount = getNListCount();
    for (size_t i = 0; i < ramCount; i++)
    {
        /*完善在当前广告不可用的前提下 给出可用的广告 除非均不可用 返回空*/
        if (firstCall)
        {
            /*首次调用时 获取头节点*/
            firstCall = false;
            base = cListDataByNode(nHead);
            // base = cListGetFirstAvailable(nHead);
        }
        else
        {
            base = findByTaskID(nextID);
            if (NULL == base)
            {
                return NULL;
            }
        }
        
        /*当前节点可能因为过期被删除 因此要提前获取下一节点*/
        printRAMNTaskID("getNListNextPlayTask");

        playTask *temp = getNextTask(base->taskId);
        if (NULL != temp)
        {
            nextID = temp->taskId;
        }
        if(true == currentTaskCheck(base))
        {
            return (const playTask *)base;
        }
    }
    return NULL;
}

/*
*分屏播放时，获取图片广告
*/
const playTask *getNListNextPlayTaskForPic()
{
    static int nextID = 0;
    playTask *base = NULL;
    int ramCount = getNListCount();
    for (size_t i = 0; i < ramCount; i++)
    {
        /*完善在当前广告不可用的前提下 给出可用的广告 除非均不可用 返回空*/
        if (firstCallForPic)
        {
            /*首次调用时 获取头节点*/
            firstCallForPic = false;
            base = cListDataByNode(nHead);
            // base = cListGetFirstAvailable(nHead);
        }else
        {
            base = findByTaskID(nextID);
            if (NULL == base)
            {
                return NULL;
            }
        }
        
        /*当前节点可能因为过期被删除 因此要提前获取下一节点*/
        //printRAMNTaskID("getNListNextPlayTask");

        playTask *temp = getNextTask(base->taskId);
        if (NULL != temp)
        {
            nextID = temp->taskId;
        }
        if(true == currentTaskCheck(base))
        {
            return (const playTask *)base;
        }
    }
    return NULL;
}


/**
 * @brief 给外部提供打印mediaID接口
 * 
 * @param caller 外部调用者
 */
void printRAMNMediaID(char *caller)
{
    printf("\n*************Ncaller is [%s]",caller);
    cListPrintByMediaID(nHead);
}

/**
 * @brief 给外部提供打印taskID接口
 * 
 * @param caller 外部调用者
 */
void printRAMNTaskID(char *caller)
{
    printf("\n*************Ncaller is [%s]",caller);
    cListPrintByTaskID(nHead);
}

/**
 * @brief 给外部提供打印taskName接口
 * @attention 方便与平台对照查看
 * @param caller 外部调用者
 */
void printRAMNTaskName(char *caller)
{
    printf("\n*************Ncaller is [%s]",caller);
    cListPrintByTaskName(nHead);
}
