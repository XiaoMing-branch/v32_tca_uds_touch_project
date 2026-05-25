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

/**
 * @brief  软件定时器模块
 * @note   !!!Warning：ISR中断中禁止使用timer所有接口!!!
 *         定时器通过链表管理，支持单次和周期两种模式
 *         到期后callback回调，同时可向关联task发送MSG_TASK_TIMER消息
 */

/**
 * @brief  定时器类型定义
 */
#define TC_TIMER_TYPE_ONESHOT       0   /**< 单次定时器（到期一次后自动停止） */
#define TC_TIMER_TYPE_CIRCLE        1   /**< 周期定时器（到期后自动重载继续运行） */

/**
 * @brief  定时器状态定义
 */
#define TC_STATE_INIT       0           /**< 初始状态（创建后尚未启动） */
#define TC_STATE_RUN        1           /**< 运行状态（计时中） */
#define TC_STATE_STOP       2           /**< 停止状态（已停止或到期） */

/**
 * @brief  定时器回调函数类型
 * @param  ptimer - 触发的定时器指针
 * @param  param  - 用户参数
 */
typedef void (*TC_TIMER_CALLBACK)(void *ptimer,void *param);

/**
 * @brief  软件定时器结构体
 * @note   所有定时器节点通过双向链表管理（timerRunList/timerFreeList）
 *         type：TC_TIMER_TYPE_ONESHOT或TC_TIMER_TYPE_CIRCLE
 *         state：TC_STATE_INIT/RUN/STOP
 *         task非NULL时，到期会向该任务发送MSG_TASK_TIMER消息
 *         period和startt共同决定定时精度：TcTimeGet() - startt >= period 时触发
 */
typedef struct
{
    struct T_TcListHead list;       /**< 链表节点（用于串联空闲/运行中定时器） */
    uint16_t type;                  /**< 定时器类型（TC_TIMER_TYPE_ONESHOT / TC_TIMER_TYPE_CIRCLE） */
    uint16_t state;                 /**< 定时器状态（TC_STATE_INIT / TC_STATE_RUN / TC_STATE_STOP） */
    T_TcTask *task;                 /**< 关联任务（非NULL时到期发送MSG_TASK_TIMER消息） */
    uint32_t period;                /**< 定时周期，单位tick（由TcTimeGet提供时间基准） */
    uint32_t startt;                /**< 本次定时器起始时间戳 */
    void * param;                   /**< 用户参数 */
    TC_TIMER_CALLBACK callback;     /**< 到期回调函数 */
} T_TcTimer;

/**
 * @brief  定时器模块初始化
 * @note   清空定时器数组，初始化空闲链表和运行链表
 *         将所有定时器节点添加到空闲链表中
 * @retval 1  - 初始化成功
 * @retval -1 - 初始化失败
 */
int TcTimerInit(void);

/**
 * @brief  创建软件定时器
 * @param  type       - 定时器类型（TC_TIMER_TYPE_ONESHOT / TC_TIMER_TYPE_CIRCLE）
 * @param  periodTick - 定时周期（单位：tick）
 * @param  callback   - 到期回调函数（可NULL）
 * @param  task       - 关联任务（可NULL，非NULL时到期发送MSG_TASK_TIMER消息）
 * @param  param      - 用户参数
 * @note   从空闲链表中取一个定时器节点，初始化后添加到运行链表
 *         创建后定时器处于TC_STATE_INIT状态，需调用TcTimerStart启动
 * @retval T_TcTimer* - 创建成功
 * @retval NULL       - 创建失败（空闲链表为空）
 */
T_TcTimer * TcTimerCreate(uint16_t type,uint32_t periodTick,TC_TIMER_CALLBACK callback,T_TcTask *task,void *param);

/**
 * @brief  启动定时器
 * @param  ptimer - 定时器指针
 * @note   记录当前时间到startt，将状态设为TC_STATE_RUN
 */
void TcTimerStart(T_TcTimer *ptimer);

/**
 * @brief  停止定时器
 * @param  ptimer - 定时器指针
 * @note   将状态设为TC_STATE_STOP，定时器停止计时
 */
void TcTimerStop(T_TcTimer *ptimer);

/**
 * @brief  修改定时器周期并重新计时
 * @param  ptimer     - 定时器指针
 * @param  periodTick - 新的定时周期（tick）
 * @note   同时更新period和startt，相当于重启定时器
 */
void TcTimerChgPeriod(T_TcTimer *ptimer,uint32_t periodTick);

/**
 * @brief  复位定时器计数器
 * @param  ptimer - 定时器指针
 * @note   将startt更新为当前时间，定时器从头开始计时
 *         不改变定时器状态和周期
 */
void TcTimerCntReset(T_TcTimer *ptimer);

/**
 * @brief  销毁定时器
 * @param  ptimer - 定时器指针
 * @note   将定时器从运行链表中移除，添加到空闲链表
 *         销毁后如要使用需重新调用TcTimerCreate
 */
void TcTimerDestroy(T_TcTimer *ptimer);

/**
 * @brief  定时器调度执行函数
 * @note   遍历运行链表中的定时器，检查是否超时
 *         超时时调用callback回调，并向关联task发送MSG_TASK_TIMER消息
 *         周期定时器自动重载startt，单次定时器状态设为STOP
 */
void TcTimerExec(void);

#endif
