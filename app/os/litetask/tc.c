#include "tcae10.h"
#include "tc.h"

/**
 * @brief  定时器状态宏
 */
#define TC_STATE_INIT       0       /**< 初始状态 */
#define TC_STATE_RUN        1       /**< 运行状态 */
#define TC_STATE_STOP       2       /**< 停止状态 */

/**< 任务缓冲区 */
static T_TcTask tcTasks[TC_TASK_NUM];

/**< 当前执行任务 */
T_TcTask *currentTask = NULL;

/**< 定时器缓冲区 */
static T_TcTimer tcTimers[TC_TIMER_NUM];

/**< 底层定时器执行 */
static void TcTimerExec(void);

/**
 * @brief  创建任务（Litetask简化版）
 * @param  name     - 任务名称（未使用）
 * @param  callback - 回调函数（不能为NULL）
 * @param  param    - 传给MSG_TASK_INIT的参数
 * @param  priority - 优先级（Litetask未实现多级优先级，此参数被忽略）
 * @note   在tcTasks数组中查找空闲槽位，创建后自动发送MSG_TASK_INIT消息
 *         若初始化消息发送失败则回滚
 * @retval T_TcTask* - 创建成功
 * @retval NULL      - 创建失败（槽位已满或MSG_TASK_INIT发送失败）
 */
T_TcTask *TcTaskCreate(const char *name, TC_TASK_CALLBACK callback, void *param, uint32_t priority)
{
    T_TcTask *task = NULL;

    /*传入参数合法性检测*/
    if (!callback)
    {
        return NULL;
    }

    for (int i = 0; i < TC_TASK_NUM; i++)
    {
        if (!tcTasks[i].isUsed)
        {
            task = &tcTasks[i];
            task->isUsed = 1;
            task->bitFlag = 0;
            task->msgNum = 0;
            task->callback = callback;

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

    return task;
}

/**
 * @brief  向任务发送消息
 * @param  task  - 目标任务
 * @param  msg   - 消息ID
 * @param  param - 消息参数
 * @note   将消息写入任务的msgs数组，msgNum递增
 *         若消息槽满（msgNum >= TC_MSG_NUM）则返回失败
 * @retval 1  - 发送成功
 * @retval -1 - 发送失败（消息槽已满）
 */
int TcTaskSendMsg(T_TcTask *task, uint32_t msg, void *param)
{
    if (task->msgNum >= TC_MSG_NUM)
    {
        return -1;
    }

    task->msgs[task->msgNum].param = param;
    task->msgs[task->msgNum].msg = msg;
    ++task->msgNum;

    return 1;
}

/**
 * @brief  清除指定任务的所有待处理消息
 * @param  task - 目标任务
 * @note   直接将msgNum置0，不修改msgs数组内容
 * @retval 1 - 清除成功
 */
int TcTaskClrMsg(T_TcTask *task)
{
    task->msgNum = 0;

    return 1;
}

/**
 * @brief  清空所有使用中的任务的消息
 * @note   遍历所有任务，将处于使用中的任务的msgNum置0
 */
void TcTaskClrAllMsg(void)
{
    for (int i = 0; i < TC_TASK_NUM; i++)
    {
        if (tcTasks[i].isUsed)
        {
            tcTasks[i].msgNum = 0;

        }
    }
}

/**
 * @brief  向所有使用中的任务广播消息（同步执行）
 * @param  msg   - 消息类型
 * @param  param - 消息参数
 * @note   直接遍历所有任务调用其回调函数，不经过消息队列
 */
void TcTaskBroadcastHandleMsg(uint32_t msg, void *param)
{
    for (int i = 0; i < sizeof(tcTasks) / sizeof(tcTasks[0]); i++)
    {
        if (tcTasks[i].isUsed)       //有效任务
        {
            tcTasks[i].callback(msg, param);
        }
    }
}

/**
 * @brief  强制销毁任务
 * @param  task - 目标任务
 * @note   直接将任务的isUsed置0，不发送MSG_TASK_DESTROY
 */
void TcTaskForceDestroy(T_TcTask *task)
{
    task->isUsed = 0;
}

/**
 * @brief  任务调度执行函数（Litetask核心调度器）
 * @note   调度流程：
 *         1. 检查系统tick是否变化，若是则执行TcTimerExec处理定时器
 *         2. 按顺序遍历所有任务
 *         3. 对每个有待处理消息（msgNum>0）的任务，依次调用其回调处理所有消息
 *         4. 处理完毕后将msgNum置0
 *         5. 若任务bitFlag非0，发送MSG_TASK_BITFLAG消息
 *         Litetask采用简单轮询调度，不区分优先级
 */
void TcTaskExec(void)
{
    T_TcTask *task = NULL;
    static uint32_t lastTick = 0;

    if (lastTick != TcSystick)      //运行定时消息
    {
        lastTick = TcSystick;
        TcTimerExec();
    }

    for (int i = 0; i < TC_TASK_NUM; ++i)
    {
        if (tcTasks[i].isUsed)
        {
            task = &tcTasks[i];

            if (task->msgNum)               //处理任务消息
            {
                currentTask = task;
                for (int j = 0; j < task->msgNum; ++j)
                {
                    (*task->callback)(task->msgs[j].msg, task->msgs[j].param);
                }
                task->msgNum = 0;
                currentTask = NULL;
            }

            if (task->bitFlag)      //处理任务标记位
            {
                currentTask = task;
                currentTask->callback(MSG_TASK_BITFLAG, NULL);
                currentTask = NULL;
            }
        }
    }
}

/**
 * @brief  创建定时器（Litetask简化版）
 * @param  type     - TC_TIMER_TYPE_ONESHOT 或 TC_TIMER_TYPE_CIRCLE
 * @param  periodMs - 定时周期（单位ms）
 * @param  callback - Litetask未使用此参数
 * @param  task     - 关联任务（可NULL，到期发送MSG_TASK_TIMER消息）
 * @param  param    - Litetask未使用此参数
 * @note   从tcTimers数组中查找空闲槽位，初始化定时器
 *         创建后处于INIT状态，需调用TcTimerStart启动
 * @retval T_TcTimer* - 创建成功
 * @retval NULL       - 创建失败（无空闲槽位）
 */
T_TcTimer *TcTimerCreate(uint8_t type, uint32_t periodMs, void *callback, T_TcTask *task, void *param)
{
    T_TcTimer *ptmr = NULL;

    for (int i = 0; i < TC_TIMER_NUM; i++)
    {
        if (!tcTimers[i].isUsed)
        {
            ptmr = &tcTimers[i];

            ptmr->isUsed = 1;
            ptmr->type = type;
            ptmr->state = TC_STATE_INIT;
            ptmr->period = periodMs;
            ptmr->task = task;

            break;
        }
    }

    return ptmr;
}

/**
 * @brief  启动定时器
 * @param  ptimer - 定时器指针
 * @note   记录当前时间到startt，状态设为RUN
 */
void TcTimerStart(T_TcTimer *ptimer)
{
    ptimer->startt = TcTimeGet();
    ptimer->state = TC_STATE_RUN;
}

/**
 * @brief  停止定时器
 * @param  ptimer - 定时器指针
 * @note   状态设为STOP，计时暂停
 */
void TcTimerStop(T_TcTimer *ptimer)
{
    ptimer->state = TC_STATE_STOP;
}

/**
 * @brief  修改定时器周期并重新计时
 * @param  ptimer   - 定时器指针
 * @param  periodMs - 新周期（ms）
 * @note   同时更新period和startt
 */
void TcTimerChgPeriod(T_TcTimer *ptimer, uint32_t periodMs)
{
    ptimer->period = periodMs;
    ptimer->startt = TcTimeGet();
}

/**
 * @brief  复位定时器计数器
 * @param  ptimer - 定时器指针
 * @note   将startt更新为当前时间，从头开始计时
 */
void TcTimerCntReset(T_TcTimer *ptimer)
{
    ptimer->startt = TcTimeGet();
}

/**
 * @brief  底层定时器执行函数
 * @note   遍历所有定时器，检查超时：
 *         1. 超时条件：TcTimeGet() - startt >= period * TC_SYSTICK_HZ / 1000
 *         2. 若关联了task，直接调用其回调发送MSG_TASK_TIMER消息
 *         3. 周期定时器（CIRCLE）：用startt + period更新下次超时时间
 *         4. 单次定时器（ONESHOT）：状态设为STOP
 */
static void TcTimerExec(void)
{
    T_TcTimer *pos = NULL;

    for (int i = 0; i < TC_TIMER_NUM; ++i)      //遍历定时器有没有超时
    {
        if (tcTimers[i].isUsed)
        {
            pos = &tcTimers[i];
            if (pos->state == TC_STATE_RUN)     //只处理运行中的定时器
            {
                if (TcTimeGet() - pos->startt >= pos->period * TC_SYSTICK_HZ / 1000) //超时
                {
                    if (pos->task)                  /*向窗口发送通知消息*/
                    {
                        if (pos->task->isUsed)
                        {
                            //任务是有效的
                            currentTask = pos->task;
                            pos->task->callback(MSG_TASK_TIMER, NULL);
                            currentTask = NULL;
                        }
                    }

                    if (pos->type == TC_TIMER_TYPE_CIRCLE)      //周期性定时器
                    {
                        pos->startt = pos->startt + pos->period * TC_SYSTICK_HZ / 1000;
                    }
                    else        //单周期定时器
                    {
                        pos->state = TC_STATE_STOP;
                    }
                }
            }
        }
    }
}
