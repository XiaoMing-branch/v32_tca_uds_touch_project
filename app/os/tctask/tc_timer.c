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

/*timer接口是否可以运行在中断ISR中*/
#define TIMER_RUN_IN_ISR       0

#if !TIMER_RUN_IN_ISR

    #undef TC_ENTER_CRITICAL
    #undef TC_EXIT_CRITICAL

    #define  TC_ENTER_CRITICAL()  do{		\
		                                  cpu_sr = cpu_sr;	\
		                                }while(0)
    #define  TC_EXIT_CRITICAL()   {}

#endif

/*定时器缓冲区*/
static T_TcTimer tcTimers[TC_TIMER_NUM];

/*空闲定时器链表*/
struct T_TcListHead timerFreeList;
/*运行中定时器链表*/
struct T_TcListHead timerRunList;

/*底层运行定时器*/
static void TcTimerExecLow(void);

/*定时器初始化，失败返回-1，成功返回1*/
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

/*创建定时器，callback为到期回调函数，task表示到期时向其发送MSG_TASK_TIMER消息*/
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

/*打开定时器*/
void TcTimerStart(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->startt = TcTimeGet();
    ptimer->state = TC_STATE_RUN;

    TC_EXIT_CRITICAL();
}

/*关闭定时器*/
void TcTimerStop(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->state = TC_STATE_STOP;

    TC_EXIT_CRITICAL();
}

/*修改超时时间，并重新计时*/
void TcTimerChgPeriod(T_TcTimer *ptimer, uint32_t periodTick)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->period = periodTick;
    ptimer->startt = TcTimeGet();

    TC_EXIT_CRITICAL();
}

/*定时计数器，复位*/
void TcTimerCntReset(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    ptimer->startt = TcTimeGet();

    TC_EXIT_CRITICAL();
}

/*销毁定时器，定时器销毁后，只能重新create*/
void TcTimerDestroy(T_TcTimer *ptimer)
{
    TC_CPU_SR  cpu_sr = 0u;     /*开关临界区用*/

    TC_ENTER_CRITICAL();

    /*将定时器移到空闲链表中*/
    TcListDel(&ptimer->list);
    TcListAddTail(&ptimer->list, &timerFreeList);

    TC_EXIT_CRITICAL();
}

/*底层运行定时器*/
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

//定时器任务处理函数
void TcTimerExec(void)
{
    static uint32_t lastTick = 0;

    if (lastTick != TcSystick)
    {
        TcTimerExecLow();
        lastTick = TcSystick;
    }
}
