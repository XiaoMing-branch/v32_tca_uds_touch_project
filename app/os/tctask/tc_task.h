/**
*****************************************************************************
* @brief  tc task header
* @file   tc_task.h
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

#ifndef TC_TASK_H__
#define TC_TASK_H__

/*任务优先等级，数值越小，优先级越高*/
#define TC_TASK_PRIO_LOW                3
#define TC_TASK_PRIO_MID                2
#define TC_TASK_PRIO_HIGH               1
#define TC_TASK_PRIO_CRITICAL           0

/*系统保留的消息类型*/
#define MSG_TASK_INIT                   0               //task被创建时接收到此消息
#define MSG_TASK_DESTROY                1               //task被销毁时接收到此消息
#define MSG_TASK_TIMER                  2               //定时器消息
#define MSG_TASK_QUEUE                  3               //task监听的队列收到数据时接收到此消息
#define MSG_TASK_BITFLAG                4               //task标记位消息

/*用户可以使用的消息起始值*/
#define MSG_TASK_USER_BASE          1024

/*任务回调函数*/
typedef void (*TC_TASK_CALLBACK)(uint32_t msg, void *param);

/*消息类型*/
typedef struct
{
    struct T_TcListHead list;       //所有消息用链表管理
    uint16_t taskId;
    uint32_t msg;
    void *param;
} T_TcMsg;

/*task类型*/
typedef struct
{
    uint16_t taskId;        //编码方式，将task优先级编码进taskId中
    uint16_t isUsed;        //0表示任务未被使用，1表示任务被使用
    volatile uint32_t bitFlag;               //32位的标记位，由用户自定义使用
    const char *name;
    TC_TASK_CALLBACK callback;

    /*统计任务运行状态*/
#if TC_GENERATE_RUN_TIME_STATS
    uint32_t lastTickCnt;           //上一次5秒占用的cpu时间片数
    uint32_t tickCntPer5S;      //每5秒占用cpu时间片数
#endif

} T_TcTask;

//所有任务描述数组
extern T_TcTask tcTasks[TC_TASK_PRIO_LOW + 1][TC_TASK_NUM];

extern T_TcTask *currentTask;       //当前执行任务

/*任务初始化，失败返回-1，成功返回1*/
int TcTaskInit(void);

/*任务创建，priority是TC_TASK_PRIO_XXX,callback不能为NULL，失败返回NULL*/
T_TcTask *TcTaskCreate(const char *name, TC_TASK_CALLBACK callback, void *param, uint32_t priority);

/*向任务发送消息，失败返回-1，成功返回1*/
int TcTaskSendMsg(T_TcTask *task, uint32_t msg, void *param);

/*清除指定任务的所有待处理消息*/
int TcTaskClrMsg(T_TcTask *task);

/*清空所有任务消息*/
void TcTaskClrAllMsg(void);

/*获取任务标记*/
#define TcTaskGetBitFlag(task)      ((task)->bitFlag)

/*设置任务标记，任务标记非线程安全的，需要用户自己保证*/
#define TcTaskSetBitFlag(task,bits)         do{			\
                                                (task)->bitFlag |= (bits);	\
                                              }while(0)

/*清除任务标记，任务标记非线程安全的，需要用户自己保证*/
#define TcTaskClrBitFlag(task,bits)         do{			\
                                                (task)->bitFlag &= ~(bits);	\
                                              }while(0)

/*销毁任务,失败返回-1，失败原因通常是申请不到内存发送MSG_TASK_DESTROY消息*/
int TcTaskDestroy(T_TcTask *task);
																						
/*强制销毁任务*/																							
void TcTaskForceDestroy(T_TcTask *task);

/*向所有任务广播处理消息*/
void TcTaskBroadcastHandleMsg(uint32_t msg, void *param);

/*任务循环运行*/
void TcTaskExec(void);

/*统计任务运行状态*/
#if TC_GENERATE_RUN_TIME_STATS

    /*清除所有任务的tickCntPer5S计数器，并将其值保存到lastTickCnt中*/
    void TcAllTaskClrTickCnt(void);

    /*获取任务的运行时状态，失败返回-1，成功返回1*/
    int TcGetTaskRunStats(T_TcTask *task, T_TcTaskStats *stats);
#endif

#endif
