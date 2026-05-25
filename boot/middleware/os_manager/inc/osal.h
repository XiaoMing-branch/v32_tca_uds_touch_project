/**
 *****************************************************************************
 * @brief   osal header file.
 *
 * @file    osal.h
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

#ifndef __OSAL_H__
#define __OSAL_H__

#include "os_task.h"
#include "pal_systick.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @brief 创建并注册一个周期性/一次性任务 */
#define OS_TASK_CREATE(task_func, delay, period, op) \
    os_task_create(task_func, delay, period, op)

/** @brief 删除指定句柄的任务 */
#define OS_TASK_DELETE(handle) \
    os_task_delete(handle)

/** @brief 设置指定任务的运行模式 */
#define OS_TASK_MODE_SET(handle, mode) \
    os_task_mode_set(handle, mode)

/** @brief 执行所有就绪的任务 */
#define OS_TASK_RUN() \
    os_task_run()

/** @brief 获取当前系统tick计数值 */
#define OS_TICK_GET() \
    systick_count_get()

/** @brief 计算两个tick之间的差值 */
#define OS_TICK_DIFF(os_tick) \
    systick_diff(os_tick)

/** @brief 阻塞式延时（毫秒） */
#define OS_DELAY_MS(ms) \
    delay_ms(ms)

/** @brief 断言宏：条件为假时触发断言处理 */
#define OS_ASSERT(expr) (expr?(void)0U: os_assert((uint8_t *)__FILE__, __LINE__))

#ifdef __cplusplus
}
#endif
#endif /* __OSAL__ */
