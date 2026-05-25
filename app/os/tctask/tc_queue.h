/**
*****************************************************************************
* @brief  tc queue header
* @file   tc_queue.h
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

#ifndef TC_QUEUE_H__
#define TC_QUEUE_H__

typedef struct
{
	uint32_t queueItemSize;				/*队列每元素大小*/
	uint32_t queueLength;					/*队列中可容纳元素个数*/
	uint32_t front;								/*队列头指针*/
	uint32_t rear;								/*队列尾指针*/
	uint32_t itemNum;							/*已经入队列元素个数*/
	void * buffer;		//队列缓冲区
	T_TcTask *task;					//当本字段非空时，队列收到数据时 ，会向任务发送MSG_TASK_QUEUE消息
}T_TcQueue;

/*队列初始化，成功返回1，失败返回-1*/
int TcQueueInit(void);

/*创建队列，若task非空，在队列入队时，会向对应的task发送MSG_TASK_QUEUE消息，携带参数为queue*/
T_TcQueue * TcQueueCreate(uint32_t queueLength,uint32_t queueItemSize,void * queueStorage,T_TcTask * task);

/*向队列发送数据，会从buffer中拷贝queueItemSize个字节到队列中，成功返回1，失败返回-1（队列满，或者无法向task发送消息）*/
int TcQueueSend(T_TcQueue * queue,const void * buffer);

/*从队列接收数据，会从队列中拷贝queueItemSize个字节到buffer中，成功返回1，失败返回-1（队列空）*/
int TcQueueReceive(T_TcQueue * queue,void * buffer);

/*从队列头取出一个元素，但不弹出队列头，成功返回1，失败返回-1（队列空）*/
int TcQueuePeek(T_TcQueue * queue,void * buffer);

/*计算队列长度*/
int TcQueueLength(T_TcQueue * queue);

/*清空队列中数据*/
void TcQueueClear(T_TcQueue * queue);

/*销毁队列，释放队列到空闲链表中*/
void TcQueueDestroy(T_TcQueue * queue);

#endif
