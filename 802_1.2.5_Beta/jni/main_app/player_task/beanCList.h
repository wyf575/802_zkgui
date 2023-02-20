#ifndef BEAN_CLIST_H
#define BEAN_CLIST_H
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

///////////////////////////////////

typedef enum 
{
    PICTURE_TASK = 1,   //图片任务
    VIDEO_TASK          //视频任务
}taskType;//与平台mediaType字段对应

typedef struct 
{
    taskType mediaType;/*图片任务or视频任务*/
    char mediaId[256];/*服务端，不晓得干嘛的*/ //媒体名称 example /data/nlist/mediaID
    char taskName[256];/*服务端，广告任务名称*/
    int taskId;/*服务端，广告任务ID，和taskName同指向同一个任务*/
    int timeStart;/*广告播放开始时间*/
    int timeEnd;/*广告播放结束时间*/
    int interval;/*图片广告播放时间，也就是图片显示时间，单位s*/
    long int mediaSize;/*图片或时评大小*/
    long int dateStart;/*广告播放开始日期*/
    long int dateEnd;/*广告播放结束日期*/
}playTask;

#define DataType playTask
///////////////////////////////////////


//链表中一个节点的结构体
typedef struct cListNode {
    DataType data; // 值
    struct cListNode *Next; // 指向下一个结点
} cListNode;


// 初始化 ,构造一条空的链表
void cListInit(cListNode **ppFirst);

//打印链表
void cListPrintByTaskID(cListNode *First);
void cListPrintByMediaID(cListNode *First);
void cListPrintByTaskName(cListNode *First);

//获取链表中所有视频总大小KB
long int cListMediaSize(cListNode *First) ;

//获取链表节点数
unsigned int cListNodeCount(cListNode *First);

// 尾部插入
void cListPushBack(cListNode **ppFirst, DataType data);

// 头部插入
void cListPushFront(cListNode **ppFirst, DataType data);

// 尾部删除
void cListPopBack(cListNode **ppFirst);

// 头部删除
void cListPopFront(cListNode **ppFirst);

// 给定结点插入，插入到结点前
void cListInsertForNode(cListNode **ppFirst, cListNode *pPos, DataType data);

//按位置插入
int cListInsert(cListNode **ppFirst, int Pos, DataType data);

// 给定位置删除
int cListErase(cListNode **ppFirst,int Pos);

// 给定结点删除
void cListEraseForNode(cListNode **ppFirst, cListNode *pPos);

// 按值删除，只删遇到的第一个
void cListRemoveByTaskID(cListNode **ppFirst, DataType data);
 //按值删除，只删遇到的第一个
void cListRemoveByMediaID(cListNode **ppFirst, DataType data);

// 按值删除，删除所有的
void cListRemoveAll(cListNode **ppFirst, DataType data);

// 销毁 ，需要销毁每一个节点
void cListDestroy(cListNode **ppFirst);

// 按值查找，返回第一个找到的结点指针，如果没找到，返回 NULL
cListNode* cListFindByTaskID(cListNode *pFirst, DataType data);
cListNode* cListFindByMediaID(cListNode *pFirst, DataType data);

//根据当前数据返回下一个数据地址或NULL
DataType *cListNextByTaskID(cListNode *pFirst, DataType data);
DataType *cListNextByMediaID(cListNode *pFirst, DataType data);
DataType *cListDataByNode(cListNode *node);
DataType *cListGetFirstAvailable(cListNode *First);

#endif 

