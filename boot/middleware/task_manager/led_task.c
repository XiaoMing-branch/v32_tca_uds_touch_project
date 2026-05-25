/**
 *****************************************************************************
 * @brief   led task source file.
 *
 * @file    led_task.c
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

#include "osal.h"
#include "led_disp.h"
#include "colormixing.h"
#include "led_control.h"
/**
 * @brief  LED任务处理函数
 *         遍历所有LED通道，依次调用PWM点亮输出
 * @param  无
 * @retval 无
 */
static void task_led_handle(void)
{
    for (led_channel_e channel = LED_CHANNEL_0; channel < LED_CHANNEL_MAX; channel++)
    {
        cm_target_pwm_lighting(channel);
    }
}

/**
 * @brief  LED任务初始化函数
 *         初始化LED显示模块，创建LED处理任务
 * @param  无
 * @retval 无
 */
void led_task_init(void)
{
    led_disp_init();
    OS_TASK_CREATE(task_led_handle, 1, 1, 1);
}
