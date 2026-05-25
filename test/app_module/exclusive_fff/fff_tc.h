#ifndef __FFF_TC_H__
#define __FFF_TC_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fff_tc_conf.h"
#include "fff_tc_usermsg.h"
#include "fff_tc_port.h"

//Task*********************************************************************************

#define MSG_TASK_INIT                   0      //任务初始化消息
#define MSG_TASK_TIMER                  1      //定时任务消息，1ms产生一次
#define MSG_TASK_BITFLAG                2      //task标记位消息
#define MSG_TASK_USER_BASE              3      //用户自定义消息

/*任务优先等级，实际未实现*/
#define TC_TASK_PRIO_LOW                3
#define TC_TASK_PRIO_MID                2
#define TC_TASK_PRIO_HIGH               1
#define TC_TASK_PRIO_CRITICAL           0

/*任务回调函数*/
typedef void (*TC_TASK_CALLBACK)(uint32_t msg, void *param);

/*task类型*/
typedef struct
{
    uint8_t isUsed;     //是否被使用
    uint8_t msgNum;     //有几个待处理消息
    struct
    {
        uint32_t msg;       //消息
        void *param;        //算法参数
    } msgs[TC_MSG_NUM];
    TC_TASK_CALLBACK callback;    //回调函数
    volatile uint32_t bitFlag;  //32位的标记位，由用户自定义使用
} T_TcTask;

extern T_TcTask *currentTask;       //当前执行任务

/*任务创建，priority是TC_TASK_PRIO_XXX,callback不能为NULL，para和priority未使用，失败返回NULL*/
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
#define TcTaskSetBitFlag(task,bits)         do{(task)->bitFlag |= (bits);}while(0)

/*清除任务标记，任务标记非线程安全的，需要用户自己保证*/
#define TcTaskClrBitFlag(task,bits)         do{(task)->bitFlag &= ~(bits);}while(0)

/*向所有任务广播处理消息*/
void TcTaskBroadcastHandleMsg(uint32_t msg, void *param);

/*强制销毁任务*/
void TcTaskForceDestroy(T_TcTask *task);

/*任务循环运行*/
void TcTaskExec(void);

//定时器*********************************************************************************

/*定时器种类*/
#define TC_TIMER_TYPE_ONESHOT       0       //单次
#define TC_TIMER_TYPE_CIRCLE        1       //周期

/*timer类型*/
typedef struct
{
    uint8_t isUsed;                     //是否被使用
    uint8_t type;                       //定时器种类，单次，周期
    uint8_t state;                     //定时器状态，初始状态，运行中状态，停止状态
    T_TcTask *task;                     //当本字段非空时，会向任务发送MSG_TASK_TIMER消息
    uint32_t period;                    //周期性定时器使用，单位ms
    uint32_t startt;                    //本次定时器起始时间
} T_TcTimer;

/*创建定时器，task表示到期时向其发送MSG_TASK_TIMER消息，可以为NULL，callback和param未使用*/
T_TcTimer *TcTimerCreate(uint8_t type, uint32_t periodMs, void *callback, T_TcTask *task, void *param);

/*打开定时器*/
void TcTimerStart(T_TcTimer *ptimer);

/*关闭定时器*/
void TcTimerStop(T_TcTimer *ptimer);

/*修改超时时间，并重新计时*/
void TcTimerChgPeriod(T_TcTimer *ptimer, uint32_t periodMs);

/*定时计数器，复位*/
void TcTimerCntReset(T_TcTimer *ptimer);

#endif
