/**
*****************************************************************************
* @brief  tc timer header
* @file   tc_timer.h
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

#ifndef TC_TIMER_H__
#define TC_TIMER_H__

/*!!!Warning：中断ISR中禁止使用timer所有接口*/

/*定时器种类*/
#define TC_TIMER_TYPE_ONESHOT       0       //单次
#define TC_TIMER_TYPE_CIRCLE        1       //周期

/*定时器状态*/
#define TC_STATE_INIT       0       //初始状态
#define TC_STATE_RUN        1       //运行状态
#define TC_STATE_STOP       2       //停止状态

/*定时器回调函数*/
typedef void (*TC_TIMER_CALLBACK)(void *ptimer,void *param);

/*timer类型*/
typedef struct
{
    struct T_TcListHead list;           //所有定时器用链表管理
    uint16_t type;                      //定时器种类，单次，周期
    uint16_t state;                     //定时器状态，初始状态，运行中状态，停止状态
    T_TcTask *task;                     //当本字段非空时，会向任务发送MSG_TASK_TIMER消息
    uint32_t period;                    //周期性定时器使用，单位tick
    uint32_t startt;                    //本次定时器起始时间
    void * param;
    TC_TIMER_CALLBACK callback;
} T_TcTimer;

/*定时器初始化，失败返回-1，成功返回1*/
int TcTimerInit(void);

/*创建定时器，callback为到期回调函数，task表示到期时向其发送MSG_TASK_TIMER消息，都可以为NULL*/
T_TcTimer * TcTimerCreate(uint16_t type,uint32_t periodTick,TC_TIMER_CALLBACK callback,T_TcTask *task,void *param);

/*打开定时器*/
void TcTimerStart(T_TcTimer *ptimer);

/*关闭定时器*/
void TcTimerStop(T_TcTimer *ptimer);

/*修改超时时间，并重新计时*/
void TcTimerChgPeriod(T_TcTimer *ptimer,uint32_t periodTick);

/*定时计数器，复位*/
void TcTimerCntReset(T_TcTimer *ptimer);

/*销毁定时器，定时器销毁后，只能重新create*/
void TcTimerDestroy(T_TcTimer *ptimer);

/*运行定时器*/
void TcTimerExec(void);

#endif
