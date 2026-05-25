/**
*****************************************************************************
* @brief  tc queue source
* @file   tc_queue.c
* @author AE/FAE team
* @date   28/JUL/2023
*****************************************************************************
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <b>&copy; Copyright (c) 2023 Tinychip Microelectronics Co.,Ltd.</b>
*
*****************************************************************************
*/

#include "tc.h"

/*队列缓冲区*/
static T_TcQueue tcQueues[TC_QUEUE_NUM];

//队列内存池，用来管理空闲的队列
static T_TcMem *tcQueuesMem = NULL;

/*队列初始化，成功返回1，失败返回-1*/
int TcQueueInit(void)
{
    memset(tcQueues,0x0,sizeof(tcQueues));

    if((tcQueuesMem = TcMemCreate(tcQueues,TC_QUEUE_NUM,sizeof(tcQueues[0]))) == NULL)
    {
        return -1;
    }

    return 1;
}

/*创建队列，若task非空，在队列入队时，会向对应的task发送MSG_TASK_QUEUE消息，携带参数为queue*/
T_TcQueue * TcQueueCreate(uint32_t queueLength,uint32_t queueItemSize,void * queueStorage,T_TcTask * task)
{
    T_TcQueue * queue = NULL;

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!queueStorage)
    {
        return NULL;
    }
#endif

    if((queue = TcMemGet(tcQueuesMem)) == NULL)
    {
        return NULL;
    }

    queue->queueItemSize = queueItemSize;
    queue->queueLength = queueLength;
    queue->buffer = queueStorage;
    queue->task = task;
    queue->front = queue->rear = 0;
    queue->itemNum = 0;

    return queue;
}

/*向队列发送数据，会从buffer中拷贝queueItemSize个字节到队列中，成功返回1，失败返回-1（队列满，或者无法向task发送消息）*/
int TcQueueSend(T_TcQueue * queue,const void * buffer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!queue || !buffer)
    {
        return -1;
    }
#endif

    TC_ENTER_CRITICAL();

    /*判断队列满*/
    if(queue->queueLength == queue->itemNum)
    {
        TC_EXIT_CRITICAL();
        return -1;
    }

    /*判断消息是否发送失败*/
    if(queue->task && TcTaskSendMsg(queue->task,MSG_TASK_QUEUE,queue) < 0)
    {
        TC_EXIT_CRITICAL();
        return -1;
    }

    /*入队列元素*/
    memcpy(&((uint8_t *)queue->buffer)[queue->rear * queue->queueItemSize],buffer,queue->queueItemSize);
    queue->rear = (queue->rear + 1) % queue->queueLength;
    ++queue->itemNum;

    TC_EXIT_CRITICAL();

    return 1;
}

/*从队列接收数据，会从队列中拷贝queueItemSize个字节到buffer中，成功返回1，失败返回-1（队列空）*/
int TcQueueReceive(T_TcQueue * queue,void * buffer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!queue || !buffer)
    {
        return -1;
    }
#endif

    TC_ENTER_CRITICAL();

    /*判断队列是否为空*/
    if(!queue->itemNum)
    {
        TC_EXIT_CRITICAL();
        return -1;
    }

    /*出队列元素*/
    memcpy(buffer,&((uint8_t *)queue->buffer)[queue->front * queue->queueItemSize],queue->queueItemSize);
    queue->front = (queue->front + 1) % queue->queueLength;
    queue->itemNum--;

    TC_EXIT_CRITICAL();

    return 1;
}

/*从队列头取出一个元素，但不弹出队列头，成功返回1，失败返回-1（队列空）*/
int TcQueuePeek(T_TcQueue * queue,void * buffer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!queue || !buffer)
    {
        return -1;
    }
#endif

    TC_ENTER_CRITICAL();

    /*判断队列是否为空*/
    if(!queue->itemNum)
    {
        TC_EXIT_CRITICAL();
        return -1;
    }

    /*出队列元素*/
    memcpy(buffer,&((uint8_t *)queue->buffer)[queue->front * queue->queueItemSize],queue->queueItemSize);

    TC_EXIT_CRITICAL();

    return 1;
}

/*计算队列长度*/
int TcQueueLength(T_TcQueue * queue)
{
    int qlen;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!queue)
    {
        return -1;
    }
#endif

    /*由于某些情况下，可能出现一直循环查询队列直到收到数据的情况，所以先在非临界区判空，此方法可以减少进去临界区关中断的时间*/
    if(!queue->itemNum)
    {
        return 0;
    }

    TC_ENTER_CRITICAL();

    qlen = queue->itemNum;

    TC_EXIT_CRITICAL();

    return qlen;
}

/*清空队列中数据*/
void TcQueueClear(T_TcQueue * queue)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!queue)
    {
        return;
    }
#endif

    TC_ENTER_CRITICAL();

    queue->front = queue->rear = 0;
    queue->itemNum = 0;

    TC_EXIT_CRITICAL();
}

/*销毁队列，释放队列到空闲链表中*/
void TcQueueDestroy(T_TcQueue * queue)
{
#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if(!queue)
    {
        return;
    }
#endif

    TcMemPut(tcQueuesMem,queue);
}
