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

//所有任务描述数组
T_TcTask tcTasks[TC_TASK_PRIO_LOW + 1][TC_TASK_NUM];

//消息缓冲区
static T_TcMsg tcMsgs[TC_MSG_NUM];

//消息内存池，用来管理消息节点的分配释放
static T_TcMem *tcMsgsMem = NULL;

/*待处理消息列表*/
static struct T_TcListHead tcMsgsTable[TC_TASK_PRIO_LOW + 1];

T_TcTask *currentTask = NULL;       //当前执行任务

/*处理任务标记位*/
static void TcTaskBitFlagRun(void);

/*任务初始化，失败返回-1，成功返回1*/
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

/*任务创建，priority是TC_TASK_PRIO_XXX,callback不能为NULL*/
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

/*向任务发送消息,失败返回-1，成功返回1*/
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

/*清除指定任务的所有待处理消息*/
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

/*清空所有任务消息*/
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

/*销毁任务,失败返回-1，失败原因通常是申请不到内存发送MSG_TASK_DESTROY消息*/
int TcTaskDestroy(T_TcTask *task)
{
    if (TcTaskSendMsg(task, MSG_TASK_DESTROY, 0) < 0)
    {
        return -1;
    }

    return 1;
}

/*强制销毁任务*/
void TcTaskForceDestroy(T_TcTask *task)
{
    task->isUsed = 0;
}

/*向所有任务广播处理消息*/
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

/*任务循环运行*/
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

/*处理任务标记位*/
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

/*统计任务运行状态*/
#if TC_GENERATE_RUN_TIME_STATS

/*清除所有任务的tickCntPer5S计数器，并将其值保存到lastTickCnt中*/
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

/*获取任务的运行时状态，失败返回-1，成功返回1*/
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
