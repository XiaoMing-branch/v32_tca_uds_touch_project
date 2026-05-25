/**
 *****************************************************************************
 * @brief  LIN休眠/唤醒管理模块源文件。
 *         处理低功耗模式切换(DEEPSLEEP_MODE)、弱函数定义（测量值清除和LED颜色恢复）。
 * @file   lin_wakeup.c
 * @author AE/FAE team
 * @date   09/Jan/2024
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
#include "lin.h"
#include "pal_pmu.h"
#include "lin_wakeup.h"

extern uint8_t g_lighting_init;
extern void meas_manager_value_clear(void);
extern void led_color_wakeup_recovery_handle(void);

/**
 * @brief  清除测量管理器的采样值（弱函数，可被用户重写）
 * @note   默认空实现，在唤醒后调用，用于清除休眠前的测量数据
 * @retval 无
 */
__attribute__((weak)) void meas_manager_value_clear(void)
{
    //do noting
}

/**
 * @brief  唤醒后恢复LED颜色状态（弱函数，可被用户重写）
 * @note   默认空实现，在唤醒后调用，用于恢复休眠前的LED显示效果
 * @retval 无
 */
__attribute__((weak)) void led_color_wakeup_recovery_handle(void)
{
    //do noting
}

/**
 * @brief  设置LIN休眠模式并执行低功耗进入/退出流程
 * @param  mode - 休眠模式选择
 * @note   支持模式：
 *         - DEEPSLEEP_MODE: 深度睡眠，约580uA
 *         - SLEEPWALK_MODE: 浅睡眠，约23uA
 * @retval 无
 */
static void lin_sleep_mode_set(sleep_mode_e mode)
{
    /* low power enter */
    pmu_lpm_enter(mode);  /* 进入低功耗模式，MCU停止运行直到唤醒事件 */

    /* low power exit */
    pmu_lpm_exit();  /* 唤醒后退出低功耗模式，恢复系统时钟 */
}

/**
 * @brief  初始化系统低功耗管理模块
 * @note   调用PMU底层初始化，配置低功耗相关寄存器
 * @retval 无
 */
void system_low_power_init(void)
{
    pmu_lpm_init();
}

/**
 * @brief  进入休眠模式的主入口函数
 * @note   当lin_goto_sleep_flg置位时触发：
 *         1. 清零休眠标志和一致性测试唤醒计数
 *         2. 调用lin_sleep_mode_set进入DEEPSLEEP_MODE
 *         3. 唤醒后清除测量值并恢复LED颜色
 * @retval 无
 */
#ifdef CFG_LIN_CONFORM_TEST
    extern volatile uint8_t bus_wake_flag;
    extern uint32_t bus_wake_cnts;
#endif
void sleep_mode_enter(void)
{
    if (lin_goto_sleep_flg == 1)
    {
        lin_goto_sleep_flg = 0;

#ifdef CFG_LIN_CONFORM_TEST
        bus_wake_flag = 0;
        bus_wake_cnts = 0;
#endif

        lin_sleep_mode_set(DEEPSLEEP_MODE);  /* 进入深度睡眠（包含唤醒后的退出） */

        /* meas_manager_value_clear */
        meas_manager_value_clear();  /* 唤醒后清除测量采样值 */
        /* led color restore */
        led_color_wakeup_recovery_handle();  /* 唤醒后恢复LED颜色状态 */
    }
}
