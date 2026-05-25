/**
 *****************************************************************************
 * @brief   monitor_task source file.
 *
 * @file    monitor_task.c
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
#include "app.h"
#include "lin_wakeup.h"
#include "pal_wdg.h"

#ifdef EMC_TEST
#include "colormixing.h"
extern uint8_t lin_receive_msg_timeout;

/**
 * @brief  EMC测试监控处理函数
 *         检测LIN接收超时，若超时次数超过10次则将LED通道0设为红色报警
 * @param  无
 * @retval 无
 */
static void emc_monitor_handle(void)
{
    if (lin_receive_msg_timeout++ > 10)
    {
        color_pwm_t color_pwm = {255, 0, 0, 0x0};
        cm_set_target_pwm(LED_CHANNEL_0, &color_pwm);

        cm_target_pwm_lighting(LED_CHANNEL_0);
        lin_receive_msg_timeout = 0;
    }
}
#endif

/**
 * @brief  监控任务处理函数
 *         喂看门狗、进入低功耗模式（可选）、EMC异常检测
 * @param  无
 * @retval 无
 */
static void task_monitor_handle(void)
{
#if CFG_SUPPORT_WDG_EN
    wdg_reload();
#endif

#if CFG_SUPPORT_LOWPOWER
    /* enter low power mode */
    sleep_mode_enter();
#endif

#ifdef EMC_TEST
    emc_monitor_handle();
#endif
}

/**
 * @brief  监控任务初始化函数
 *         初始化看门狗（3000ms超时）、初始化低功耗模式、创建监控任务
 * @param  无
 * @retval 无
 */
void monitor_task_init(void)
{
#if CFG_SUPPORT_WDG_EN
    wdg_init(3000); //3s
#endif
    /* close unused devices */
    system_low_power_init();

    OS_TASK_CREATE(task_monitor_handle, 10, 10, 1);
}
