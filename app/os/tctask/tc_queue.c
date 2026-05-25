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

/**
 * @brief  全局队列缓冲区数组
 * @note   TC_QUEUE_NUM由tc_conf.h配置
 */
static T_TcQueue tcQueues[TC_QUEUE_NUM];

/**< 队列节点内存池，用于管理空闲的T_TcQueue节点 */
static T_TcMem *tcQueuesMem = NULL;

/**
 * @brief  队列模块初始化
 * @note   清空全局队列数组tcQueues，通过TcMemCreate创建队列节点内存池
 *         内存池大小 = TC_QUEUE_NUM * sizeof(T_TcQueue)
 * @retval 1  - 初始化成功
 * @retval -1 - 初始化失败
 */
int TcQueueInit(void)
{
    memset(tcQueues,0x0,sizeof(tcQueues));

    if((tcQueuesMem = TcMemCreate(tcQueues,TC_QUEUE_NUM,sizeof(tcQueues[0]))) == NULL)
    {
        return -1;
    }

    return 1;
}

/**
 * @brief  创建消息队列
 * @param  queueLength   - 队列最大元素个数
 * @param  queueItemSize - 每个元素大小（字节）
 * @param  queueStorage  - 预分配的队列缓冲区（不能为NULL）
 * @param  task          - 关联任务（可NULL，非NULL时入队后向该任务发送MSG_TASK_QUEUE消息）
 * @note   从队列内存池中分配一个T_TcQueue节点，初始化为空队列状态
 *         用户需自行保证queueStorage的尺寸 >= queueLength * queueItemSize
 * @retval T_TcQueue* - 创建成功
 * @retval NULL       - 创建失败
 */
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

/**
 * @brief  向队列发送数据（入队）
 * @param  queue  - 队列指针
 * @param  buffer - 待入队数据的源地址
 * @note   使用环形缓冲区机制：数据写入rear位置，rear前进(queue->rear+1)%queueLength
 *         入队前检查队列是否已满；若关联了task则先向其发送MSG_TASK_QUEUE消息
 *         消息发送失败时入队操作回滚（返回-1）
 *         操作在临界区保护下执行
 * @retval 1  - 入队成功
 * @retval -1 - 入队失败（队列满或消息发送失败）
 */
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

/**
 * @brief  从队列接收数据（出队）
 * @param  queue  - 队列指针
 * @param  buffer - 存放接收数据的目的地址
 * @note   从front位置拷贝数据到buffer，然后front前进
 *         队列为空时返回-1（非阻塞）
 *         操作在临界区保护下执行
 * @retval 1  - 出队成功
 * @retval -1 - 出队失败（队列空或参数无效）
 */
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

/**
 * @brief  查看队列头部元素（不弹出，不修改队列状态）
 * @param  queue  - 队列指针
 * @param  buffer - 存放查看数据的目的地址
 * @note   与TcQueueReceive的区别是此操作不修改front和itemNum
 *         队列为空时返回-1
 * @retval 1  - 查看成功
 * @retval -1 - 查看失败（队列空或参数无效）
 */
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

/**
 * @brief  获取队列当前元素个数
 * @param  queue - 队列指针
 * @note   先在非临界区快速判空以优化性能（避免频繁开关中断）
 *         对于循环查询队列的场景可减少关中断时间
 * @retval 当前队列中的元素个数（0表示空）
 */
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

/**
 * @brief  清空队列中所有数据
 * @param  queue - 队列指针
 * @note   将front、rear重置为0，itemNum清零
 *         不清除buffer内容，仅重置队列状态
 */
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

/**
 * @brief  销毁队列，释放队列节点回内存池
 * @param  queue - 队列指针
 * @note   调用TcMemPut将T_TcQueue节点归还到队列内存池
 *         注意：不释放用户传入的queueStorage缓冲区
 */
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
