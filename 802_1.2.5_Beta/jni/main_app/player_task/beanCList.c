#include "beanCList.h"
#include <string.h>

void cListInit(cListNode **ppFirst) //初始化
{
    assert(ppFirst != NULL);
    *ppFirst = NULL;
}

void cListPrintByTaskID(cListNode *First)   //打印链表
{
    printf("RAM list taskId: ");
    for (cListNode*p = First; p != NULL; p = p->Next)
        printf("%d-->", p->data.taskId);
    printf("*************\n");
}

void cListPrintByMediaID(cListNode *First)   //打印链表
{
    printf("RAM list mediaId: ");
    for (cListNode*p = First; p != NULL; p = p->Next)
        printf("%s-->", p->data.mediaId);
    printf("*************\n");
}

void cListPrintByTaskName(cListNode *First)   //打印链表
{
    printf("RAM list mediaId: ");
    for (cListNode*p = First; p != NULL; p = p->Next)
        printf("%s-->", p->data.taskName);
    printf("*************\n");
}

DataType *cListGetFirstAvailable(cListNode *First)
{
    for (cListNode*p = First; p != NULL; p = p->Next)
    {
        if (p->data.taskId !=0 )
        {
            printf("******p->data.taskId*******\n");
            printf("%d-->", p->data.taskId);
            return &(p->data);
        }
    }
    printf("******Sorry there no Available DataType*******\n");
    return NULL;
}

long int cListMediaSize(cListNode *First)   
{
    long int AllSize = 0;
    for (cListNode*p = First; p != NULL; p = p->Next)
    {
        AllSize = AllSize + p->data.mediaSize;
    }
    return AllSize;
}

unsigned int cListNodeCount(cListNode *First)   //获取链表节点数
{
    int count = 0;
    for(cListNode* p = First; p != NULL; p = p->Next)
        count ++;
    return count;
}

static cListNode* CreateNode(DataType data) //申请新节点
{
    cListNode*node = (cListNode*)malloc(sizeof(cListNode));
    node->data = data;
    node->Next = NULL;
    return node;
}

void cListPushBack(cListNode** ppFirst, DataType data)  // 尾部插入
{
    assert(ppFirst != NULL);
    cListNode *node = CreateNode(data);
    if (*ppFirst == NULL)//判断链表不为空
    {
        *ppFirst = node;
        return;
    }
    //找链表中的最后一个节点
    cListNode* p = *ppFirst;
    while (p->Next != NULL)
        p = p->Next;
    p->Next = node;//插入新申请的节点
}

void cListPushFront(cListNode **ppFirst, DataType data) //头部插入
{
    assert(ppFirst != NULL);
    cListNode *node = CreateNode(data);
    node->Next = *ppFirst;
    *ppFirst = node;
}

void cListPopBack(cListNode **ppFirst)  // 尾部删除
{
    assert(ppFirst != NULL);
    assert(*ppFirst != NULL);
    if ((*ppFirst)->Next == NULL)
    {
        free(*ppFirst);
        *ppFirst = NULL;
        return;
    }
    cListNode*p = *ppFirst;
    while(p->Next->Next != NULL)
        p = p->Next;
    free(p->Next);
    p->Next = NULL;
}

void cListPopFront(cListNode **ppFirst) // 头部删除
{
    assert(ppFirst != NULL);
    assert(*ppFirst != NULL);   //链表不是空链表
    cListNode *first = *ppFirst;
    *ppFirst = (*ppFirst)->Next;
    free(first);
}

void cListInsertForNode(cListNode **ppFirst, cListNode *pPos, DataType data)   //按节点指针插入
{
    assert(ppFirst != NULL);
    if (*ppFirst == pPos)
    {
        cListPushFront(ppFirst, data);
        return;
    }
    cListNode*newNode = CreateNode(data);
    cListNode*p;

    for (p = *ppFirst; p->Next != pPos; p = p->Next){ }     //找到pos前的一个节点
    p->Next = newNode;          //改变的是字段内的值，而不是指针的值
    newNode->Next = pPos;
}

int cListInsert(cListNode **ppFirst, int Pos, DataType data)  //按位置插入
{
    cListNode* p = *ppFirst;
    for(int i = 0;i < Pos;i ++)
    {
        if(p == NULL)
            return 0;
        p = p->Next;
    }
    cListInsertForNode(ppFirst,p,data);
    return 1;
}

int cListErase(cListNode **ppFirst,int Pos)
{
    cListNode* p = *ppFirst;
    for(int i = 0;i < Pos;i ++)
    {
        if(p == NULL)
            return 0;
        p = p->Next;
    }
    cListEraseForNode(ppFirst,p);
    return 1;
}

void cListEraseForNode(cListNode **ppFirst, cListNode *pPos)   // 给定结点删除
{
    if (*ppFirst == pPos)
    {
        cListPopFront(ppFirst);
        return;
    }
    cListNode *p = *ppFirst;
    while (p->Next != pPos)
        p = p->Next;
    p->Next = pPos->Next;
    free(pPos);
}

void cListRemoveByTaskID(cListNode **ppFirst, DataType data)    //按值删除，只删遇到的第一个
{
    cListNode *p = *ppFirst;
    cListNode *prev = NULL;
    assert(ppFirst != NULL);
    if (*ppFirst == NULL)
            return;
    while (p)
    {
        if (p->data.taskId == data.taskId)
        {
            if (*ppFirst == p)  //删除的是第一个节点
            {
                *ppFirst = p->Next;
                free(p);
                p = NULL;
            }
            else            //删除中间节点
            {
                prev->Next = p->Next;
                free(p);
                p = NULL;
            }
            break;
        }
        prev = p;
        p = p->Next;
    }
}

void cListRemoveByMediaID(cListNode **ppFirst, DataType data)    //按值删除，只删遇到的第一个
{
    cListNode *p = *ppFirst;
    cListNode *prev = NULL;
    assert(ppFirst != NULL);
    if (*ppFirst == NULL)
            return;
    while (p)
    {
        if (!strcmp(p->data.mediaId,data.mediaId))
        {
            if (*ppFirst == p)  //删除的是第一个节点
            {
                *ppFirst = p->Next;
                free(p);
                p = NULL;
            }
            else            //删除中间节点
            {
                prev->Next = p->Next;
                free(p);
                p = NULL;
            }
            break;
        }
        prev = p;
        p = p->Next;
    }
}


void cListRemoveAll(cListNode **ppFirst, DataType data) // 按值删除，删除所有的
{
    cListNode *p = NULL;
    cListNode *prev = NULL;
    assert(ppFirst != NULL);
    if (*ppFirst == NULL)
        return;
    p = *ppFirst;
    while (p)
    {
        if (p->data.taskId == data.taskId)
        {
            if (*ppFirst == p)//删除的是第一个节点
            {
                *ppFirst = p->Next;
                free(p);
                p = *ppFirst;
            }
            else    //删除中间节点
            {
                prev->Next = p->Next;
                free(p);
                p = prev;
            }
        }
        prev = p;
        p = p->Next;
    }
}

void cListDestroy(cListNode **ppFirst)  // 销毁 ，需要销毁每一个节点
{
    cListNode*p = NULL;
    cListNode*del = NULL;
    assert(ppFirst);
    p = *ppFirst;
    while (p)
    {
        del = p;
        p = p->Next;
        free(del);
        del = NULL;
    }
    *ppFirst = NULL;
}

cListNode* cListFindByTaskID(cListNode *pFirst, DataType data)  // 按值查找，返回第一个找到的结点指针，如果没找到，返回 NULL
{
    for(cListNode *p = pFirst; p != NULL; p = p->Next)
    {
        if (p->data.taskId == data.taskId)
            return p;
    }
    return NULL;
}

cListNode* cListFindByMediaID(cListNode *pFirst, DataType data)  // 按值查找，返回第一个找到的结点指针，如果没找到，返回 NULL
{
    for(cListNode *p = pFirst; p != NULL; p = p->Next)
    {
        if (!strcmp(p->data.mediaId,data.mediaId))
            return p;
    }
    return NULL;
}

DataType *cListNextByTaskID(cListNode *pFirst, DataType data)
{
    cListNode* node = cListFindByTaskID(pFirst,data);
    if (NULL != node )
    {
        if (NULL != node->Next)
        {
            return &(node->Next->data);
        }else if(NULL != pFirst)
        {
            // return &(pFirst->Next->data);
            return &(pFirst->data);
        }
    }
    return NULL;
}

DataType *cListNextByMediaID(cListNode *pFirst, DataType data)
{
    cListNode* node = cListFindByMediaID(pFirst,data);
    if (NULL != node )
    {
        if (NULL != node->Next)
        {
            return &(node->Next->data);
        }else if(NULL != pFirst)
        {
            // return &(pFirst->Next->data);
            return &(pFirst->data);
        }
    }
    return NULL;
}

DataType *cListDataByNode(cListNode *node)
{
    if (NULL != node)
    {
        return &(node->data);
    }
    return NULL;
}

#if 0
int demo(int argc, char const *argv[])
{
    struct cListNode *head = NULL;		//创建头指针，初始化为NULL
    cListInit(&head);                   //初始化指针（可有可无）

    //1：添加数据
    for(int i = 0;i < 30;i ++)
    {
        if(i > 10)
            cListPushFront(&head,i);        //头部插入
        else
            cListPushBack(&head,i);         //尾部插入
    }
    printf("\n 1:  count:%d \n", cListNodeCount(head));    //获取链表节点数，打印
    cListPrintByTaskID(head);   //打印链表

    //2：删除数据
    for(int i = 0;i < 10;i ++)
    {
        if(i > 5)
            cListPopBack(&head);        //删除尾部
        else
            cListPopFront(&head);       //头部删除
    }
    printf("\n 2:  count:%d \n", cListNodeCount(head));
    cListPrintByTaskID(head);

    //3：插入数据
    cListInsert(&head,5,1111);
    cListInsertForNode(&head,head->Next->Next->Next->Next->Next,10000);
    cListInsert(&head,1000,2222);       //无效插入
    printf("\n 3:  count:%d \n", cListNodeCount(head));
    cListPrintByTaskID(head);

    //4：删除指定节点
    cListErase(&head,5);
    cListEraseForNode(&head,head->Next->Next);
    cListErase(&head,1000);     //无效删除
    cListPrintByTaskID(head);
    printf("\n 4:  count:%d \n", cListNodeCount(head));
    cListPrintByTaskID(head);

    //5：删除所有节点
    cListDestroy(&head);
    printf("\n 5:  count:%d    ", cListNodeCount(head));
    cListPrintByTaskID(head);
    return 0;
}
#endif

