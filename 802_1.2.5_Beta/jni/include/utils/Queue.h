/*
 * Queue.h
 *
 *  Created on: 2021年1月5日
 *      Author: Administrator
 */

#ifndef JNI_INCLUDE_UTILS_QUEUE_H_
#define JNI_INCLUDE_UTILS_QUEUE_H_

/**********在队头删除节点，队尾添加节点*************/
#include<iostream>
using namespace std;
template<typename T>
class Queue
{
public:
    Queue();
    ~Queue();
    bool empty() const;
    int size() const;
    void clear();
    void push(const T & node);
    T pop();
    T&  front();
    T   front() const;
private:       //也可以直接用来链表list直接构造
    struct  QueueNode
    {
        T data;
        QueueNode* next;
        QueueNode(const T& Newdata, QueueNode* nextnode=NULL) :data(Newdata), next(nextnode)
        { }
       // QueueNode() = default;
    };
    QueueNode * Front;  //队头指针
    QueueNode * rear;  // 队尾指针
    int count;
};
//此处不设头节点
template<typename T>
Queue<T>::Queue() :Front (NULL),  rear (NULL), count(0)
{}
template<typename T>
Queue<T>::~Queue()
{
    clear();
}
template<typename T>
void Queue<T>::push(const T & node)
{
    if(Front==NULL)
        Front=rear=new QueueNode(node);
    else
    {
     QueueNode * newqueuenode = new QueueNode(node);
    rear->next = newqueuenode;
    rear = newqueuenode;
    }
    count++;
}
template<typename T>
bool Queue<T>::empty() const
{
    return Front==NULL;
}

template<typename T>
int Queue<T>::size() const
{
    return count;
}

template<typename T>
void Queue<T>::clear()
{
    while (Front)
    {
        QueueNode * FrontofQueue = Front;
        Front = Front->next;
        delete FrontofQueue;
    }
    count = 0;
}

template<typename T>
T Queue<T>::pop()
{
    if (empty())
    {
        cerr << "Error, queue is empty!";
    }
    QueueNode * FrontofQueue = Front;
    Front = Front->next;
    delete FrontofQueue;
    count--;
    return FrontofQueue->data;
}

template<typename T>
T& Queue<T>::front()
{
    if (empty())
    {
        cerr << "Error, queue is empty!";
    }
    return Front->data;
}

template<typename T>
T Queue<T>::front() const
{
    if (empty())
    {
        cerr << "Error, queue is empty!";
    }
    return Front->data;
}

#endif /* JNI_INCLUDE_UTILS_QUEUE_H_ */
