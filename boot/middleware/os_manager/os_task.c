/**
 *****************************************************************************
 * @brief   os task source file.
 *
 * @file    os_task.c
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

#include <stdio.h>
#include "os_task.h"
#include "logging.h"

static task_t sch_task_group[SCH_MAX_TASKS];

/**
 * @brief  操作系统任务调度更新函数
 *         遍历所有任务槽，递减延迟计数器，将到期任务标记为可运行（runme++）。
 *         LOOP模式任务在每个tick都会被标记为可运行。
 *         单次执行任务（period=0）执行后自动清除任务指针。
 * @param  无
 * @retval 无
 */
void os_task_update(void)
{
    for (task_handle_t handle = 0; handle < SCH_MAX_TASKS; handle++)
    {
        if (sch_task_group[handle].ptask)
        {
            if (sch_task_group[handle].delay)
            {
                sch_task_group[handle].delay --;
            }

            if (sch_task_group[handle].delay == 0 || sch_task_group[handle].op == OS_TASK_MODE_LOOP)
            {
                if (sch_task_group[handle].op)
                {
                    sch_task_group[handle].runme += 1;
                }
                else
                {
                    (*sch_task_group[handle].ptask)();

                    if (sch_task_group[handle].period == 0)
                    {
                        sch_task_group[handle].ptask = 0;
                    }
                }

                if (sch_task_group[handle].period)
                {
                    sch_task_group[handle].delay = sch_task_group[handle].period;
                }
            }
        }
    }
}

/**
 * @brief  创建操作系统任务
 *         关中断，查找空闲任务槽，填入任务函数指针、延迟、周期和运行模式。
 *         若任务槽已满返回SCH_MAX_TASKS。
 * @param  ptask  - 任务函数指针
 * @param  delay  - 初始延迟（单位：系统tick数）
 * @param  period - 运行周期（0表示仅执行一次）
 * @param  op     - 运行模式（OS_TASK_MODE_xx）
 * @retval 任务句柄（0~SCH_MAX_TASKS-1），SCH_MAX_TASKS表示创建失败
 */
task_handle_t os_task_create(pfun ptask, uint32_t delay, uint32_t period, uint8_t op)
{
    task_handle_t handle = 0;

    __disable_irq();

    while ((sch_task_group[handle].ptask != NULL)
           && (handle < SCH_MAX_TASKS))
    {
        handle ++;
    }

    if (handle == SCH_MAX_TASKS)
    {
        __enable_irq();
        return SCH_MAX_TASKS;
    }

    sch_task_group[handle].ptask = ptask;
    sch_task_group[handle].delay = delay;
    sch_task_group[handle].period = period;
    sch_task_group[handle].runme = 0;
    sch_task_group[handle].op = op;

    __enable_irq();

    return handle;
}

/**
 * @brief  删除操作系统任务
 *         关中断，清空指定句柄对应的任务槽（置零ptask/delay/period/runme）
 * @param  handle - 要删除的任务句柄
 * @retval OS_STATUS_OK        - 删除成功
 * @retval OS_STATUS_DELE_ERROR - 任务不存在（ptask为空）
 */
os_status_e os_task_delete(task_handle_t handle)
{
    os_status_e status;

    __disable_irq();

    if (sch_task_group[handle].ptask == 0)
    {
        status  = OS_STATUS_DELE_ERROR;
    }
    else
    {
        status = OS_STATUS_OK;
    }

    sch_task_group[handle].ptask  = NULL;
    sch_task_group[handle].delay  = 0;
    sch_task_group[handle].period = 0;
    sch_task_group[handle].runme  = 0;

    __enable_irq();

    return status;
}

/**
 * @brief  设置任务运行模式
 *         更改指定任务的调度模式（LOOP/TIME_SLICE/PRE_EMPTIVE）
 * @param  handle - 任务句柄
 * @param  mode   - 目标运行模式
 * @retval OS_STATUS_OK        - 设置成功
 * @retval OS_STATUS_OPS_ERROR - 任务不存在
 */
os_status_e os_task_mode_set(task_handle_t handle, os_task_mode_e mode)
{
    os_status_e status;

    if (sch_task_group[handle].op != mode)
    {

        __disable_irq();

        if (sch_task_group[handle].ptask == 0)
        {
            status  = OS_STATUS_OPS_ERROR;
        }
        else
        {
            status = OS_STATUS_OK;
        }

        sch_task_group[handle].op  = mode;

        __enable_irq();
    }

    return status;
}

/**
 * @brief  执行就绪任务
 *         遍历所有任务槽，执行runme>0且op不为PRE_EMPTIVE(0)的任务，
 *         每执行一次runme减1。单次执行任务（period=0）执行后自动删除。
 * @param  无
 * @retval 无
 */
void os_task_run(void)
{
    uint8_t handle;

    for (handle = 0; handle < SCH_MAX_TASKS; handle++)
    {
        if ((sch_task_group[handle].runme > 0)
            && (sch_task_group[handle].op))
        {
            (*sch_task_group[handle].ptask)();
            sch_task_group[handle].runme -= 1;

            if (sch_task_group[handle].period == 0)
            {
                os_task_delete(handle);
            }
        }
    }
}

/**
 * @brief  操作系统断言处理函数
 *         打印断言失败的文件名和行号，然后进入死循环
 * @param file - 断言失败的文件名字符串指针
 * @param line - 断言失败的行号
 * @retval 无（函数不会返回）
 */
void os_assert(uint8_t *file, uint32_t line)
{
    log_warn("[OS] assert file:%s line:%d\r\n", file, line);

    while (1);
}