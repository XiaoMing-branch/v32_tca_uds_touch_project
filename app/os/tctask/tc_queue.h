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

/**
 * @brief  环形消息队列结构体
 * @note   使用环形缓冲区实现固定大小的消息队列
 *         front指向队列头（出队位置），rear指向队列尾（入队位置）
 *         task字段非NULL时，入队操作会向该任务发送MSG_TASK_QUEUE消息
 */
typedef struct
{
	uint32_t queueItemSize;     /**< 队列中每个元素的大小（字节） */
	uint32_t queueLength;       /**< 队列最大可容纳元素个数 */
	uint32_t front;             /**< 队列头指针（出队位置索引） */
	uint32_t rear;              /**< 队列尾指针（入队位置索引） */
	uint32_t itemNum;           /**< 当前已入队列的元素个数 */
	void * buffer;              /**< 队列缓冲区起始地址 */
	T_TcTask *task;             /**< 关联任务（非NULL时，入队后向此任务发送MSG_TASK_QUEUE消息） */
}T_TcQueue;

/**
 * @brief  队列模块初始化
 * @note   清空全局队列数组tcQueues，创建队列内存池tcQueuesMem
 * @retval 1  - 初始化成功
 * @retval -1 - 初始化失败（内存池创建失败）
 */
int TcQueueInit(void);

/**
 * @brief  创建消息队列
 * @param  queueLength   - 队列最大元素个数
 * @param  queueItemSize - 每个元素的大小（字节）
 * @param  queueStorage  - 预分配的队列缓冲区地址
 * @param  task          - 关联任务（可NULL，非NULL时入队后发MSG_TASK_QUEUE消息）
 * @retval T_TcQueue* - 创建成功
 * @retval NULL       - 创建失败（参数无效或队列内存池耗尽）
 */
T_TcQueue * TcQueueCreate(uint32_t queueLength,uint32_t queueItemSize,void * queueStorage,T_TcTask * task);

/**
 * @brief  向队列发送数据（入队）
 * @param  queue  - 队列指针
 * @param  buffer - 待入队数据的源地址，将拷贝queueItemSize字节到队列中
 * @note   若队列已满则返回-1；若关联了task则发送MSG_TASK_QUEUE消息（消息发送失败也返回-1）
 *         环形缓冲区机制：rear = (rear + 1) % queueLength
 * @retval 1  - 入队成功
 * @retval -1 - 入队失败（队列满或消息发送失败）
 */
int TcQueueSend(T_TcQueue * queue,const void * buffer);

/**
 * @brief  从队列接收数据（出队）
 * @param  queue  - 队列指针
 * @param  buffer - 存放接收数据的目的地址，从队列中拷贝queueItemSize字节
 * @note   若队列为空则返回-1；环形缓冲区出队：front = (front + 1) % queueLength
 * @retval 1  - 出队成功
 * @retval -1 - 出队失败（队列空）
 */
int TcQueueReceive(T_TcQueue * queue,void * buffer);

/**
 * @brief  查看队列头部元素（不弹出）
 * @param  queue  - 队列指针
 * @param  buffer - 存放查看数据的目的地址
 * @note   与TcQueueReceive的区别是此操作不移动front指针
 * @retval 1  - 查看成功
 * @retval -1 - 查看失败（队列空）
 */
int TcQueuePeek(T_TcQueue * queue,void * buffer);

/**
 * @brief  计算队列中当前元素数量
 * @param  queue - 队列指针
 * @retval 当前队列中的元素个数
 */
int TcQueueLength(T_TcQueue * queue);

/**
 * @brief  清空队列中所有数据
 * @param  queue - 队列指针
 * @note   将front和rear都复位为0，itemNum置为0
 *         不清除buffer内容，仅重置状态
 */
void TcQueueClear(T_TcQueue * queue);

/**
 * @brief  销毁队列，释放队列节点回全局内存池
 * @param  queue - 队列指针
 * @note   调用TcMemPut将队列节点归还到tcQueuesMem内存池中
 */
void TcQueueDestroy(T_TcQueue * queue);

#endif
