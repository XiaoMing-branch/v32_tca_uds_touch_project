#ifndef __TC_H__
#define __TC_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "tc_conf.h"
#include "tc_port.h"

//*****************************************************************************
//  任务管理模块
//*****************************************************************************

/**
 * @brief  系统保留消息类型
 */
#define MSG_TASK_INIT                   0      /**< 任务初始化消息 */
#define MSG_TASK_TIMER                  1      /**< 定时器到期消息 */
#define MSG_TASK_BITFLAG                2      /**< 标记位消息 */
#define MSG_TASK_USER_BASE              3      /**< 用户自定义消息起始值 */

/**
 * @brief  任务优先级定义（Litetask未实际实现多级优先级）
 */
#define TC_TASK_PRIO_LOW                3
#define TC_TASK_PRIO_MID                2
#define TC_TASK_PRIO_HIGH               1
#define TC_TASK_PRIO_CRITICAL           0

/**
 * @brief  任务回调函数类型
 * @param  msg   - 消息ID
 * @param  param - 消息参数
 */
typedef void (*TC_TASK_CALLBACK)(uint32_t msg, void *param);

/**
 * @brief  任务控制块结构体（Litetask简化版）
 * @note   使用消息数组而非链表管理消息，每个任务固定TC_MSG_NUM个消息槽
 *         msgNum记录当前待处理消息数量，超出则消息发送失败
 */
typedef struct
{
    uint8_t isUsed;         /**< 使用标志（0-未使用，1-使用中） */
    uint8_t msgNum;         /**< 当前待处理消息数量 */
    struct
    {
        uint32_t msg;       /**< 消息类型 */
        void *param;        /**< 消息参数 */
    } msgs[TC_MSG_NUM];     /**< 消息数组，固定大小 */
    TC_TASK_CALLBACK callback;    /**< 任务回调函数 */
    volatile uint32_t bitFlag;    /**< 32位用户自定义标记位 */
} T_TcTask;

/**< 当前正在执行的任务指针 */
extern T_TcTask *currentTask;

/**
 * @brief  创建任务
 * @param  name     - 任务名称（未使用）
 * @param  callback - 回调函数（不能为NULL）
 * @param  param    - 传给MSG_TASK_INIT的参数
 * @param  priority - 优先级（Litetask未使用）
 * @retval T_TcTask* - 成功；NULL - 失败
 */
T_TcTask *TcTaskCreate(const char *name, TC_TASK_CALLBACK callback, void *param, uint32_t priority);

/**
 * @brief  向任务发送消息
 * @param  task  - 目标任务
 * @param  msg   - 消息ID
 * @param  param - 消息参数
 * @retval 1 - 成功；-1 - 失败（消息槽已满）
 */
int TcTaskSendMsg(T_TcTask *task, uint32_t msg, void *param);

/**
 * @brief  清除任务所有待处理消息
 * @param  task - 目标任务
 * @retval 1 - 成功
 */
int TcTaskClrMsg(T_TcTask *task);

/**
 * @brief  清空所有任务消息
 */
void TcTaskClrAllMsg(void);

/**
 * @brief  获取任务标记位
 * @param  task - 任务指针
 */
#define TcTaskGetBitFlag(task)      ((task)->bitFlag)

/**
 * @brief  设置任务标记位（非线程安全）
 * @param  task - 任务指针
 * @param  bits - 位掩码
 */
#define TcTaskSetBitFlag(task,bits)         do{(task)->bitFlag |= (bits);}while(0)

/**
 * @brief  清除任务标记位（非线程安全）
 * @param  task - 任务指针
 * @param  bits - 位掩码
 */
#define TcTaskClrBitFlag(task,bits)         do{(task)->bitFlag &= ~(bits);}while(0)

/**
 * @brief  向所有任务广播消息（同步执行）
 * @param  msg   - 消息类型
 * @param  param - 消息参数
 */
void TcTaskBroadcastHandleMsg(uint32_t msg, void *param);

/**
 * @brief  强制销毁任务
 * @param  task - 目标任务
 */
void TcTaskForceDestroy(T_TcTask *task);

/**
 * @brief  任务调度执行函数
 * @note   按顺序遍历所有任务，先处理定时器消息，再处理任务消息，最后处理标记位
 */
void TcTaskExec(void);

//*****************************************************************************
//  软件定时器模块
//*****************************************************************************

/**
 * @brief  定时器类型
 */
#define TC_TIMER_TYPE_ONESHOT       0       /**< 单次定时器 */
#define TC_TIMER_TYPE_CIRCLE        1       /**< 周期定时器 */

/**
 * @brief  定时器控制块（Litetask简化版）
 * @note   不支持回调函数callback，到期时仅向task发送MSG_TASK_TIMER消息
 */
typedef struct
{
    uint8_t isUsed;         /**< 使用标志 */
    uint8_t type;           /**< 定时器类型（ONESHOT/CIRCLE） */
    uint8_t state;          /**< 定时器状态（INIT/RUN/STOP） */
    T_TcTask *task;         /**< 关联任务（到期发送MSG_TASK_TIMER消息） */
    uint32_t period;        /**< 定时周期，单位ms */
    uint32_t startt;        /**< 起始时间戳 */
} T_TcTimer;

/**
 * @brief  创建定时器
 * @param  type     - 定时器类型
 * @param  periodMs - 周期（ms）
 * @param  callback - Litetask未使用
 * @param  task     - 关联任务（可NULL）
 * @param  param    - Litetask未使用
 * @retval T_TcTimer* - 成功；NULL - 失败
 */
T_TcTimer *TcTimerCreate(uint8_t type, uint32_t periodMs, void *callback, T_TcTask *task, void *param);

/**
 * @brief  启动定时器
 * @param  ptimer - 定时器指针
 */
void TcTimerStart(T_TcTimer *ptimer);

/**
 * @brief  停止定时器
 * @param  ptimer - 定时器指针
 */
void TcTimerStop(T_TcTimer *ptimer);

/**
 * @brief  修改周期并重新计时
 * @param  ptimer   - 定时器指针
 * @param  periodMs - 新周期（ms）
 */
void TcTimerChgPeriod(T_TcTimer *ptimer, uint32_t periodMs);

/**
 * @brief  复位定时器计数器
 * @param  ptimer - 定时器指针
 */
void TcTimerCntReset(T_TcTimer *ptimer);

#endif
