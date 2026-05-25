/**
 *****************************************************************************
 * @brief   meas_task source file.
 *
 * @file    meas_task.c
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
#include "measure.h"
#include "logging.h"

#if CFG_SUPPORT_FAULT_DET
#include "monitor_manager.h"
#endif

#define LOG_MEAS(...)  do{log_debug("[MEAS] " __VA_ARGS__);}while(0)

task_handle_t task_meas_handle;
extern bool lin_task_busy_status(void);

#if CFG_SUPPORT_SINGAL_BIN
extern const sft_adpat_value_t sft_adpat_value;
#endif

#if CFG_SUPPORT_LED_CTRL
/**
 * @brief  PN电容测量任务处理函数
 *         轮询各LED通道的PN电容值，根据SNPD忙状态控制采样节奏，
 *         在静态采样完成后切换到LOOP模式以加快采集
 * @param  无
 * @retval 无
 */
static void task_meas_pn_handle(void)
{
    static led_channel_e mux_channel;
    mux_channel = meas_pn_acquire_channel_get();

    if (meas_pn_process_handle(mux_channel, !!CFG_SUPPORT_COLOR_COMP))
    {
        mux_channel++;

        if (mux_channel >= LED_CHANNEL_MAX)
        {
            mux_channel = LED_CHANNEL_0;
        }

#if CFG_SUPPORT_LIN_SNPD

        if (!lin_task_busy_status())
#endif
        {
            meas_pn_acquire_handle(mux_channel);

            if (meas_pn_static_sample_status_get(LED_CHANNEL_0) || meas_pn_static_sample_status_get(LED_CHANNEL_1))
            {
                OS_TASK_MODE_SET(task_meas_handle, OS_TASK_MODE_LOOP);
            }
            else
            {
                OS_TASK_MODE_SET(task_meas_handle, OS_TASK_MODE_TIME_SLICE);
            }
        }
    }
}
#endif

/**
 * @brief  安全测量任务处理函数
 *         执行安全相关测量（VBAT/TEMP），以及故障检测处理
 * @note   若SNPD忙则跳过本次测量周期
 * @param  无
 * @retval 无
 */
static void task_meas_safty_handle(void)
{
#if CFG_SUPPORT_LIN_SNPD

    if (!lin_task_busy_status())
#endif
    {
        meas_safty_handle();
    }

#if CFG_SUPPORT_FAULT_DET
    monitor_detect_handle();
#endif
}

/**
 * @brief  测量任务初始化函数
 *         初始化所有LED通道的PN测量管理器，配置典型PN值，
 *         初始化安全测量（VBAT/TEMP）计算参数和故障检测模块，
 *         创建PN测量任务和安全测量任务
 * @param  无
 * @retval 无
 */
void meas_task_init(void)
{
    for (led_channel_e channel = LED_CHANNEL_0; channel < LED_CHANNEL_MAX ; channel++)
    {
        meas_manager_init(channel);
#if CFG_SUPPORT_SINGAL_BIN
        meas_typical_pn_init(channel, (sft_adpat_value_t *)&sft_adpat_value);
#endif
    }

    meas_safty_calucate_func(MEAS_VOLT_VBAT);
    meas_safty_calucate_func(MEAS_VOLT_TEMP);

#if CFG_SUPPORT_FAULT_DET
    monitor_manager_init();
#endif

#if CFG_SUPPORT_LED_CTRL
    task_meas_handle = OS_TASK_CREATE(task_meas_pn_handle, 20, 20, 1);
    OS_TASK_CREATE(task_meas_safty_handle, 30, 30, 1);
#endif
}
