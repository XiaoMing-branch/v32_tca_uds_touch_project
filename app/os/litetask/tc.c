#include "tcae10.h"
#include "tc.h"

/*定时器状态*/
#define TC_STATE_INIT       0       //初始状态
#define TC_STATE_RUN        1       //运行状态
#define TC_STATE_STOP       2       //停止状态

static T_TcTask tcTasks[TC_TASK_NUM];   //任务缓冲区
T_TcTask *currentTask = NULL;       //当前执行任务

static T_TcTimer tcTimers[TC_TIMER_NUM];    //定时器缓冲区

/*底层运行定时器*/
static void TcTimerExec(void);

/*任务创建，priority是TC_TASK_PRIO_XXX,callback不能为NULL，para和priority未使用，失败返回NULL*/
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

/*向任务发送消息,失败返回-1，成功返回1*/
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

/*清除指定任务的所有待处理消息*/
int TcTaskClrMsg(T_TcTask *task)
{
    task->msgNum = 0;

    return 1;
}

/*清空所有任务消息*/
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

/*向所有任务广播处理消息*/
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

/*强制销毁任务*/
void TcTaskForceDestroy(T_TcTask *task)
{
    task->isUsed = 0;
}

/*任务循环运行*/
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

/*创建定时器，task表示到期时向其发送MSG_TASK_TIMER消息，可以为NULL，callback和param未使用*/
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

/*打开定时器*/
void TcTimerStart(T_TcTimer *ptimer)
{
    ptimer->startt = TcTimeGet();
    ptimer->state = TC_STATE_RUN;
}

/*关闭定时器*/
void TcTimerStop(T_TcTimer *ptimer)
{
    ptimer->state = TC_STATE_STOP;
}

/*修改超时时间，并重新计时*/
void TcTimerChgPeriod(T_TcTimer *ptimer, uint32_t periodMs)
{
    ptimer->period = periodMs;
    ptimer->startt = TcTimeGet();
}

/*定时计数器，复位*/
void TcTimerCntReset(T_TcTimer *ptimer)
{
    ptimer->startt = TcTimeGet();
}

/*底层运行定时器*/
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
