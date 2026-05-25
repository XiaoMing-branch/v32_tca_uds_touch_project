/**
*****************************************************************************
* @brief  tc timer source
* @file   tc_timer.c
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
 * @brief  定时器ISR运行标志
 * @note   0：定时器在主循环中运行（默认，不使用临界区保护）
 *         1：定时器在ISR中运行（使用临界区保护）
 *         当前默认为0，即定时器在主循环的TcTaskExec中处理
 */
#define TIMER_RUN_IN_ISR       0

#if !TIMER_RUN_IN_ISR

    #undef TC_ENTER_CRITICAL
    #undef TC_EXIT_CRITICAL

    #define  TC_ENTER_CRITICAL()  do{		\
 		                                  cpu_sr = cpu_sr;	\
 		                                }while(0)
    #define  TC_EXIT_CRITICAL()   {}

#endif

/**< 定时器缓冲区数组 */
static T_TcTimer tcTimers[TC_TIMER_NUM];

/**< 空闲定时器链表头 */
struct T_TcListHead timerFreeList;
/**< 运行中定时器链表头 */
struct T_TcListHead timerRunList;

/**< 底层定时器执行函数（遍历运行链表检查超时） */
static void TcTimerExecLow(void);

/**
 * @brief  定时器模块初始化
 * @note   清空定时器数组，初始化空闲链表和运行链表
 *         将所有定时器节点添加到空闲链表中等待分配
 * @retval 1  - 初始化成功
 */
int TcTimerInit(void)
{
    int i;

    memset(tcTimers, 0x0, sizeof(tcTimers));
    TcListInit(&timerFreeList);
    TcListInit(&timerRunList);

    /*将所有定时器节点，添加到空闲链表中*/
    for (i = 0; i < TC_TIMER_NUM; i++)
    {
        TcListAdd(&tcTimers[i].list, &timerFreeList);
    }

    return 1;
}

/**
 * @brief  创建软件定时器
 * @param  type       - 定时器类型（TC_TIMER_TYPE_ONESHOT / TC_TIMER_TYPE_CIRCLE）
 * @param  periodTick - 定时周期（单位：tick）
 * @param  callback   - 到期回调函数（可NULL）
 * @param  task       - 关联任务（可NULL，非NULL时到期发送MSG_TASK_TIMER消息）
 * @param  param      - 用户参数
 * @note   从空闲链表中取出一个定时器节点，初始化后添加到运行链表尾部
 *         创建后定时器处于TC_STATE_INIT状态，需调用TcTimerStart启动
 * @retval T_TcTimer* - 创建成功
 * @retval NULL       - 创建失败（空闲链表无可用节点）
 */
T_TcTimer *TcTimerCreate(uint16_t type, uint32_t periodTick, TC_TIMER_CALLBACK callback, T_TcTask *task, void *param)
{
    T_TcTimer *ptmr = NULL;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    if (TcListEmpty(&timerFreeList))
    {
        TC_EXIT_CRITICAL();
        return NULL;
    }

    /*取出一个定时器节点*/
    ptmr = TcListFirstEntry(&timerFreeList, T_TcTimer, list);
    TcListDel(&ptmr->list);

    TC_EXIT_CRITICAL();

    memset(ptmr, 0x0, sizeof(*ptmr));
    ptmr->type = type;
    ptmr->state = TC_STATE_INIT;
    ptmr->period = periodTick;
    ptmr->param = param;
    ptmr->callback = callback;
    ptmr->task = task;

    TC_ENTER_CRITICAL();

    //添加到定时器链表中
    TcListAddTail(&ptmr->list, &timerRunList);

    TC_EXIT_CRITICAL();

    return ptmr;
}

/**
 * @brief  启动定时器
 * @param  ptimer - 定时器指针
 * @note   记录当前系统时间到startt，将状态设为TC_STATE_RUN
 */
void TcTimerStart(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->startt = TcTimeGet();
    ptimer->state = TC_STATE_RUN;

    TC_EXIT_CRITICAL();
}

/**
 * @brief  停止定时器
 * @param  ptimer - 定时器指针
 * @note   将状态设为TC_STATE_STOP，定时器暂停计时
 *         可通过TcTimerStart重新启动
 */
void TcTimerStop(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->state = TC_STATE_STOP;

    TC_EXIT_CRITICAL();
}

/**
 * @brief  修改定时器周期并重新计时
 * @param  ptimer     - 定时器指针
 * @param  periodTick - 新的定时周期（tick）
 * @note   同时更新period和startt，相当于重置定时器从头计时
 */
void TcTimerChgPeriod(T_TcTimer *ptimer, uint32_t periodTick)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->period = periodTick;
    ptimer->startt = TcTimeGet();

    TC_EXIT_CRITICAL();
}

/**
 * @brief  复位定时器计数器
 * @param  ptimer - 定时器指针
 * @note   将startt更新为当前时间，定时器重新开始计时
 *         不改变类型、状态和周期
 */
void TcTimerCntReset(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->startt = TcTimeGet();

    TC_EXIT_CRITICAL();
}

/**
 * @brief  销毁定时器
 * @param  ptimer - 定时器指针
 * @note   将定时器从运行链表中移除，添加到空闲链表尾部
 *         销毁后如需使用必须重新调用TcTimerCreate创建
 */
void TcTimerDestroy(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    /*将定时器移到空闲链表中*/
    TcListDel(&ptimer->list);
    TcListAddTail(&ptimer->list, &timerFreeList);

    TC_EXIT_CRITICAL();
}

/**
 * @brief  底层定时器执行函数
 * @note   遍历运行链表中的所有定时器，检查是否有定时器超时
 *         超时处理流程：
 *         1. 调用callback回调函数
 *         2. 若关联了task，向其发送MSG_TASK_TIMER消息
 *            （非ISR模式下直接调用task回调，不经过消息队列）
 *         3. 周期定时器（TYPE_CIRCLE）用beginT更新startt自动重载
 *         4. 单次定时器（TYPE_ONESHOT）状态设为TC_STATE_STOP
 */
static void TcTimerExecLow(void)
{
    T_TcTimer *pos;
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    //遍历定时器链表
    TC_ENTER_CRITICAL();
    TcListForEachEntry(pos, &timerRunList, list)
    {
        TC_EXIT_CRITICAL();
        if (pos->state == TC_STATE_RUN)     //只处理运行中的定时器
        {
            if (TcTimeGet() - pos->startt >= pos->period) //超时
            {
                uint32_t beginT = TcTimeGet();
                if (pos->callback != NULL)          /*回调函数*/
                {
                    (*pos->callback)(pos, pos->param);
                }
                if (pos->task != NULL)                  /*向窗口发送通知消息*/
                {
#if TIMER_RUN_IN_ISR
                    TcTaskSendMsg(pos->task, MSG_TASK_TIMER, pos->param);
#else
                    if (pos->task->isUsed != 0)
                    {
                        //任务是有效的
                        currentTask = pos->task;
                        currentTask->callback(MSG_TASK_TIMER, pos->param);
                        currentTask = NULL;
                    }
#endif
                }

                if (pos->type == TC_TIMER_TYPE_CIRCLE)      //周期性定时器
                {
                    pos->startt = beginT;
                }
                else        //单周期定时器
                {
                    pos->state = TC_STATE_STOP;
                }
            }
        }
        TC_ENTER_CRITICAL();
    }
    TC_EXIT_CRITICAL();
}

/**
 * @brief  定时器调度执行函数
 * @note   每tick调用一次，使用lastTick变量确保同一tick内只执行一次
 *         实际调用TcTimerExecLow遍历并处理所有超时定时器
 *         由TcTaskExec在消息调度循环中调用
 */
void TcTimerExec(void)
{
    static uint32_t lastTick = 0;

    if (lastTick != TcSystick)
    {
        TcTimerExecLow();
        lastTick = TcSystick;
    }
}
