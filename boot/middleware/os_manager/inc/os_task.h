/**
 *****************************************************************************
 * @brief   os task header file.
 *
 * @file    os_task.h
 * @author  AE/FAE team
 * @date    2024.01.01
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */

#ifndef __OS_TASK_H__
#define __OS_TASK_H__

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SCH_MAX_TASKS       10

typedef void (*pfun)(void);

typedef uint8_t task_handle_t;
/** @brief 操作系统状态枚举 */
typedef enum
{
    OS_STATUS_OPS_ERROR,      /**< 操作错误（任务不存在等） */
    OS_STATUS_OK,             /**< 操作成功 */
    OS_STATUS_DELE_ERROR,     /**< 删除错误（任务不存在） */
    OS_STATUS_TASK_OVERFLOW,  /**< 任务槽溢出 */
} os_status_e;

/** @brief 任务运行模式枚举 */
typedef enum
{
    OS_TASK_MODE_PRE_EMPTIVE, /**< 抢占式模式（由更新函数直接执行，不经过运行队列） */
    OS_TASK_MODE_TIME_SLICE,  /**< 时间片轮转模式 */
    OS_TASK_MODE_LOOP,        /**< 后台循环模式（每个tick都会执行） */
} os_task_mode_e;

/**
 * @brief 任务控制块结构体
 */
typedef struct task_t
{
    void (*ptask)();        /**< 任务函数指针 */
    uint32_t delay;         /**< 初始延迟（单位：系统tick数） */
    uint32_t period;        /**< 运行周期（0表示仅执行一次） */
    uint8_t  runme;         /**< 待执行次数计数器，os_task_update每触发一次+1 */
    uint8_t  op;            /**< 运行模式：0-抢占式 1-时间片轮转 2-后台循环 */
    struct task_t *next;    /**< 链表下一节点指针（预留） */
} task_t;

/**
 * @brief  任务调度更新
 *         每个系统tick调用一次，递减延迟、标记就绪任务
 */
void os_task_update(void);

/**
 * @brief  创建任务
 * @param  ptask  - 任务函数指针
 * @param  delay  - 初始延迟（tick数）
 * @param  period - 运行周期（0=单次）
 * @param  op     - 运行模式
 * @retval 任务句柄，SCH_MAX_TASKS表示失败
 */
task_handle_t os_task_create(pfun ptask, uint32_t delay, uint32_t period, uint8_t op);

/**
 * @brief  删除任务
 * @param  handle - 任务句柄
 * @retval 删除状态
 */
os_status_e os_task_delete(task_handle_t handle);

/**
 * @brief  设置任务运行模式
 * @param  handle - 任务句柄
 * @param  mode   - 运行模式
 * @retval 设置状态
 */
os_status_e os_task_mode_set(task_handle_t handle, os_task_mode_e mode);

/**
 * @brief  执行所有就绪任务
 */
void os_task_run(void);

/**
 * @brief  断言处理（打印信息后死循环）
 * @param file - 文件名
 * @param line - 行号
 */
void os_assert(uint8_t *file, uint32_t line);

#ifdef __cplusplus
}
#endif
#endif /* __OS_TASK_H__ */
