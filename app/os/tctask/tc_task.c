/**
*****************************************************************************
* @brief  tc task source
* @file   tc_task.c
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
 * @brief  全局任务描述数组
 * @note   tcTasks[priority][index] 按优先级分组存储所有任务
 *         每个优先级最大任务数由TC_TASK_NUM配置
 */
T_TcTask tcTasks[TC_TASK_PRIO_LOW + 1][TC_TASK_NUM];

/**
 * @brief  消息缓冲区
 * @note   所有T_TcMsg消息节点从该缓冲区中分配，由消息内存池tcMsgsMem管理
 */
static T_TcMsg tcMsgs[TC_MSG_NUM];

/**< 消息内存池，用于管理消息节点的分配和释放 */
static T_TcMem *tcMsgsMem = NULL;

/**
 * @brief  待处理消息链表数组
 * @note   每个优先级对应一个链表头，存储发送到该优先级任务的待处理消息
 *         tcMsgsTable[priority] 为当前优先级所有待处理消息的链表头
 */
static struct T_TcListHead tcMsgsTable[TC_TASK_PRIO_LOW + 1];

/**< 当前正在执行的任务指针，由调度器在任务回调前设置 */
T_TcTask *currentTask = NULL;

/**< 处理所有任务的bitFlag标记位 */
static void TcTaskBitFlagRun(void);

/**
 * @brief  任务模块初始化
 * @note   清空任务数组和消息数组，初始化各优先级的待处理消息链表
 *         调用TcMemCreate创建消息节点内存池
 * @retval 1  - 初始化成功
 * @retval -1 - 初始化失败（内存池创建失败）
 */
int TcTaskInit(void)
{
    memset(tcTasks, 0x0, sizeof(tcTasks));
    memset(tcMsgs, 0x0, sizeof(tcMsgs));

    //初始化待处理消息列表
    for (size_t i = 0; i < sizeof(tcMsgsTable) / sizeof(tcMsgsTable[0]); i++)
    {
        TcListInit(&tcMsgsTable[i]);
    }

    if ((tcMsgsMem = TcMemCreate(tcMsgs, TC_MSG_NUM, sizeof(tcMsgs[0]))) == NULL)
    {
        return -1;
    }

    return 1;
}

/**
 * @brief  创建任务
 * @param  name     - 任务名称（用于调试标识）
 * @param  callback - 任务回调函数（不能为NULL）
 * @param  param    - 传递给MSG_TASK_INIT消息的参数
 * @param  priority - 任务优先级（TC_TASK_PRIO_CRITICAL ~ TC_TASK_PRIO_LOW）
 * @note   在对应优先级的任务数组中查找空闲槽位（isUsed==0），填充任务控制块
 *         创建后自动发送MSG_TASK_INIT消息触发任务初始化，若发送失败则回滚创建
 *         taskId编码方式：priority * TC_TASK_NUM + index
 *         操作在临界区保护下执行
 * @retval T_TcTask* - 创建成功，返回任务指针
 * @retval NULL      - 创建失败（优先级无效/槽位已满/初始化消息发送失败）
 */
T_TcTask *TcTaskCreate(const char *name, TC_TASK_CALLBACK callback, void *param, uint32_t priority)
{
    T_TcTask *task = NULL;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if (!callback)
    {
        return NULL;
    }
#endif

    if (priority > TC_TASK_PRIO_LOW)
    {
        return NULL;
    }

    TC_ENTER_CRITICAL();

    for (uint32_t i = 0; i < TC_TASK_NUM; i++)
    {
        if (tcTasks[priority][i].isUsed == 0)
        {
            task = &tcTasks[priority][i];
            task->taskId = (uint16_t)(priority * TC_TASK_NUM + i);
            task->isUsed = 1;
            task->name = name;
            task->callback = callback;
            /*统计任务运行状态*/
#if TC_GENERATE_RUN_TIME_STATS
            task->tickCntPer5S = 0;
#endif

            /*向任务发送初始化消息*/
            if (TcTaskSendMsg(task, MSG_TASK_INIT, param) < 0)
            {
                /*发送初始化消息失败，创建任务失败*/
                task->isUsed = 0;
                task = NULL;
            }
            break;
        }
    }

    TC_EXIT_CRITICAL();

    return task;
}

/**
 * @brief  向目标任务发送消息
 * @param  task  - 目标任务指针
 * @param  msg   - 消息类型
 * @param  param - 消息参数
 * @note   从消息内存池tcMsgsMem中分配一个T_TcMsg节点
 *         填充taskId、msg、param后添加到对应优先级的待处理消息链表尾部
 *         若消息内存池耗尽则返回-1
 *         操作在临界区保护下执行（链表操作）
 * @retval 1  - 发送成功
 * @retval -1 - 发送失败（消息内存池已无空闲节点或参数无效）
 */
int TcTaskSendMsg(T_TcTask *task, uint32_t msg, void *param)
{
    T_TcMsg *tcMsg = NULL;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if (!task)
    {
        return -1;
    }
#endif

    if ((tcMsg = TcMemGet(tcMsgsMem)) == NULL)      //消息节点被耗尽
    {
        return -1;
    }

    //设置消息内容
    tcMsg->taskId = task->taskId;
    tcMsg->msg = msg;
    tcMsg->param = param;

    TC_ENTER_CRITICAL();

    //加到待处理消息列表中
    TcListAddTail(&tcMsg->list, &tcMsgsTable[tcMsg->taskId / TC_TASK_NUM]);

    TC_EXIT_CRITICAL();

    return 1;
}

/**
 * @brief  清除指定任务的所有待处理消息
 * @param  task - 目标任务指针
 * @note   遍历该任务所在优先级的待处理消息链表，删除所有taskId匹配的T_TcMsg节点
 *         被删除的消息节点通过TcMemPut释放回消息内存池
 *         操作在临界区保护下执行
 * @retval 1  - 清除成功
 * @retval -1 - 参数无效
 */
int TcTaskClrMsg(T_TcTask *task)
{
    uint16_t taskId;
    static struct T_TcListHead *head = NULL;
    T_TcMsg *pos = NULL, *n = NULL;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if (!task)
    {
        return -1;
    }
#endif

    taskId = task->taskId;
    head = &tcMsgsTable[taskId / TC_TASK_NUM];

    TC_ENTER_CRITICAL();

    /*遍历待处理消息，并删除*/
    TcListForEachEntrySafe(pos, n, head, list)
    {
        if (pos->taskId == taskId)
        {
            TcListDel(&pos->list);
            /*将消息释放回内存缓冲区*/
            TcMemPut(tcMsgsMem, pos);
        }
    }

    TC_EXIT_CRITICAL();

    return 1;
}

/**
 * @brief  清空所有优先级的所有待处理消息
 * @note   遍历tcMsgsTable中所有优先级（0~TC_TASK_PRIO_LOW）的链表
 *         释放所有消息节点回tcMsgsMem内存池
 *         通常在系统重置或全局消息清理时调用
 */
void TcTaskClrAllMsg(void)
{
    uint8_t prio;
    static struct T_TcListHead *head = NULL;
    T_TcMsg *pos = NULL, *n = NULL;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    for (prio = 0; prio < sizeof(tcMsgsTable) / sizeof(tcMsgsTable[0]); prio++)
    {
        head = &tcMsgsTable[prio];

        /*遍历待处理消息，并删除*/
        TcListForEachEntrySafe(pos, n, head, list)
        {
            TcListDel(&pos->list);
            /*将消息释放回内存缓冲区*/
            TcMemPut(tcMsgsMem, pos);
        }
    }

    TC_EXIT_CRITICAL();
}

/**
 * @brief  安全销毁任务
 * @param  task - 目标任务指针
 * @note   通过向任务自身发送MSG_TASK_DESTROY消息实现安全销毁
 *         任务在调度器执行到该消息时会将isUsed置0
 *         失败原因通常为消息内存池耗尽
 * @retval 1  - 销毁消息发送成功
 * @retval -1 - 发送MSG_TASK_DESTROY消息失败
 */
int TcTaskDestroy(T_TcTask *task)
{
    if (TcTaskSendMsg(task, MSG_TASK_DESTROY, 0) < 0)
    {
        return -1;
    }

    return 1;
}

/**
 * @brief  强制销毁任务（不经过消息机制）
 * @param  task - 目标任务指针
 * @note   直接将任务的isUsed置为0，不发送MSG_TASK_DESTROY消息
 *         如果任务正在执行回调，可能导致状态不一致，需谨慎使用
 *         通常用于紧急情况下的任务清理
 */
void TcTaskForceDestroy(T_TcTask *task)
{
    task->isUsed = 0;
}

/**
 * @brief  向所有处于使用中的任务广播消息（同步执行）
 * @param  msg   - 消息类型
 * @param  param - 消息参数
 * @note   遍历所有优先级和所有槽位，对isUsed非零的任务直接调用其回调函数
 *         不经过消息队列，由调用者同步执行所有任务处理
 *         适用于需要所有任务同时响应的事件（如系统休眠/唤醒）
 */
void TcTaskBroadcastHandleMsg(uint32_t msg, void *param)
{
    size_t i, j;

    for (i = 0; i < sizeof(tcTasks) / sizeof(tcTasks[0]); i++)
    {
        for (j = 0; j < sizeof(tcTasks[0]) / sizeof(tcTasks[0][0]); j++)
        {
            if (tcTasks[i][j].isUsed != 0)       //有效任务
            {
                tcTasks[i][j].callback(msg, param);
            }
        }
    }
}

/**
 * @brief  任务调度器核心函数（按优先级轮询处理消息）
 * @note   TCTask采用优先级驱动的消息处理机制：
 *         1. 从最高优先级（CRITICAL=0）开始遍历tcMsgsTable链表
 *         2. 对于PRIO_MID和PRIO_LOW，在处理消息前先调用TcTimerExec()处理定时器
 *         3. 同一优先级的所有消息在当前轮次全部处理完，才进入下一优先级
 *         4. 处理每条消息时：
 *            - 从链表中取出消息节点
 *            - 设置currentTask指向任务
 *            - 调用任务回调函数处理消息
 *            - 若消息为MSG_TASK_DESTROY，将task->isUsed置0
 *            - 释放消息节点回内存池
 *         5. 所有消息处理完毕后，调用TcTaskBitFlagRun()处理标记位
 *         @note 链表的判空操作在临界区外首次检查以减少关中断时间，进出临界区内再次确认
 */
void TcTaskExec(void)
{
    size_t i;
    T_TcMsg *curMsg = NULL;
    T_TcTask *task = NULL;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    //while(1)
    //{
    for (i = 0; i < sizeof(tcMsgsTable) / sizeof(tcMsgsTable[0]); i++)
    {
        if (i >= TC_TASK_PRIO_HIGH) //软定时任务优先级低于CRITICAL任务
        {
            TcTimerExec();
        }
        if (!TcListEmpty(&tcMsgsTable[i]))  /*有消息待处理，不放在临界区是考虑到关中断时间，所以下面在临界区内又重复检查了一次*/
        {
            while (!TcListEmpty(&tcMsgsTable[i]))       //同一优先级消息，需要执行完后才进行下一轮
            {
                TC_ENTER_CRITICAL();

                if (TcListEmpty(&tcMsgsTable[i]))       //队列被清空了
                {
                    TC_EXIT_CRITICAL();
                    break;
                }

                curMsg = TcListFirstEntry(&tcMsgsTable[i], T_TcMsg, list);
                TcListDel(&curMsg->list);       //从待处理消息中移出消息

                TC_EXIT_CRITICAL();

                task = &tcTasks[curMsg->taskId / TC_TASK_NUM][curMsg->taskId % TC_TASK_NUM];
                if (task->isUsed != 0)
                {
                    //任务是有效的
                    currentTask = task;
                    currentTask->callback(curMsg->msg, curMsg->param);
                    currentTask = NULL;

                    if (curMsg->msg == MSG_TASK_DESTROY)    //销毁当前任务
                    {
                        task->isUsed = 0;
                    }
                }

                //将消息释放回内存缓冲区
                TcMemPut(tcMsgsMem, curMsg);
            }
            break;
        }
    }
    //  if(i >= sizeof(tcMsgsTable)/sizeof(tcMsgsTable[0]))
    //  {
    //      break;
    //  }
    //}

    TcTaskBitFlagRun();     //处理任务标记位
}

/**
 * @brief  处理所有任务标记位（bitFlag）
 * @note   遍历所有优先级和槽位，对isUsed非零且bitFlag非零的任务
 *         发送MSG_TASK_BITFLAG消息（直接调用回调，不经过消息队列）
 *         此机制允许任务在ISR中通过TcTaskSetBitFlag设置标记位，
 *         然后在主循环中得到处理
 */
static void TcTaskBitFlagRun(void)
{
    for (size_t i = 0; i < sizeof(tcTasks) / sizeof(tcTasks[0]); ++i)      //遍历task表
    {
        for (size_t j = 0; j < sizeof(tcTasks[0]) / sizeof(tcTasks[0][0]); ++j)
        {
            if (tcTasks[i][j].isUsed != 0 && TcTaskGetBitFlag(&tcTasks[i][j]) != 0)
            {
                //任务是有效的
                currentTask = &tcTasks[i][j];
                currentTask->callback(MSG_TASK_BITFLAG, NULL);
                currentTask = NULL;
            }
        }
    }
}

/**
 * @brief  任务运行时统计模块
 * @note   仅在TC_GENERATE_RUN_TIME_STATS使能时编译
 *         用于统计各任务的CPU利用率
 */
#if TC_GENERATE_RUN_TIME_STATS

/**
 * @brief  清除所有任务的tickCntPer5S计数器
 * @note   将每个任务的tickCntPer5S当前值保存到lastTickCnt中
 *         然后将tickCntPer5S清零，供下一统计周期使用
 *         由TcRecordTaskRunTime每5秒调用一次
 */
void TcAllTaskClrTickCnt(void)
{
    int i, j;

    for (i = 0; i < sizeof(tcTasks) / sizeof(tcTasks[0]); i++)
    {
        for (j = 0; j < sizeof(tcTasks[0]) / sizeof(tcTasks[0][0]); j++)
        {
            tcTasks[i][j].lastTickCnt = tcTasks[i][j].tickCntPer5S;
            tcTasks[i][j].tickCntPer5S = 0;
        }
    }
}

/**
 * @brief  获取指定任务的CPU利用率统计
 * @param  task  - 目标任务指针
 * @param  stats - 输出的统计信息结构体
 * @note   CPU利用率计算方式：
 *         cpuPercent = task->lastTickCnt * 100 / (5000 * TC_SYSTICK_HZ / 1000)
 *         即过去5秒内任务占用的tick数占总tick数的百分比
 * @retval 1  - 获取成功
 * @retval -1 - 参数无效
 */
int TcGetTaskRunStats(T_TcTask *task, T_TcTaskStats *stats)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

#if TC_DEBUG_PARAM
    /*传入参数合法性检测*/
    if (!task || !stats)
    {
        return -1;
    }
#endif

    TC_ENTER_CRITICAL();

    /*计算前5秒，该任务cpu利用率*/
    stats->cpuPercent = task->lastTickCnt * 100 / (5000 * TC_SYSTICK_HZ / 1000);

    TC_EXIT_CRITICAL();

    return 1;
}

#endif
