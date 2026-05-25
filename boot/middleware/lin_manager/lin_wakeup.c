/**
 *****************************************************************************
 * @brief   lin wakeup source file.
 *
 * @file    lin_wakeup.c
 * @author  AE/FAE team
 * @date    09/Jan/2024
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
#include "pal_gpio.h"
#include "lin_wakeup.h"

#ifdef CFG_LIN_CONFORM_TEST
#include "pal_gpio.h"
static uint32_t bus_wakeup_cnt = 0;
uint8_t lin_slave_wakeup_flag = 0;
#define SYSTEM_SLEEP_MODE           SLEEPWALK_MODE
#else
#define SYSTEM_SLEEP_MODE           DEEPSLEEP_MODE
#endif

extern void lin_goto_idle_state(void);
extern void meas_manager_value_clear(void);
extern void led_disp_wakeup_recovery_handle(void);

/**
 * @brief  清除测量管理器的数据（弱定义）
 * @param  无
 * @note   弱符号定义，可由外部模块重写
 * @retval 无
 */
__attribute__((weak)) void meas_manager_value_clear(void)
{
    //do noting
}

/**
 * @brief  LED显示唤醒恢复处理（弱定义）
 * @param  无
 * @note   弱符号定义，可由外部模块重写，用于唤醒后恢复LED显示状态
 * @retval 无
 */
__attribute__((weak)) void led_disp_wakeup_recovery_handle(void)
{
    //do noting
}

#ifdef CFG_LIN_CONFORM_TEST
/**
 * @brief  LIN GPIO唤醒回调函数（一致性测试）
 * @param  gpio_pin - 触发中断的GPIO引脚号
 * @note   设置从机唤醒标志并复位唤醒计数器，仅在CFG_LIN_CONFORM_TEST使能时编译
 * @retval 无
 */
__attribute__((weak)) void lin_gpio_callback(uint32_t gpio_pin)
{
    if (lin_slave_wakeup_flag == 0 && GPIO_PIN_0 != gpio_pin)
    {
        return;
    }

    lin_slave_wakeup_flag = 1;
    bus_wakeup_cnt = 0;
}

/**
 * @brief  从机唤醒主机处理（一致性测试）
 * @param  无
 * @note   唤醒后按特定间隔发送LIN总线Break信号以唤醒主机
 * @retval 无
 */
void lin_slave_wakeup_master_handle(void)
{
    if (lin_slave_wakeup_flag == 0)
    {
        return;
    }

    bus_wakeup_cnt++;

    switch (bus_wakeup_cnt)
    {
        case 1:
        case 19:
        case 35:
            ll_lin_ctrl_brk_tx(LL_SCI_BUS_1, 8);
            break;

        case 200:
            bus_wakeup_cnt = 0;
            break;

        default:
            break;
    }
}

/**
 * @brief  初始化GPIO唤醒（一致性测试）
 * @param  mode - 睡眠模式（EEPSLEEP_MODE=580uA, SLEEPWALK_MODE=23uA）
 * @note   配置GPIO_PIN_0为下拉输入、上升沿触发唤醒
 * @retval 无
 */
static void lin_gpio_wakeup_init(sleep_mode_e mode)
{
    gpio_config_t gpio_config =
    {
        .gpio_pin = GPIO_PIN_0,
        .mode = GPIO_MODE_IN_PP,
        .pull_mode = GPIO_PULL_DOWN,
        .pull_down_type = GPIO_PULLDOWN_SW_ONLY,
        .afio = AFIO_MUX_1,
        .trigger_flag = GPIO_TRIGGER_RISING_EDGE,
    };

    pal_gpio_init(&gpio_config, lin_gpio_callback);
}
#endif

/**
 * @brief  设置系统进入睡眠模式
 * @param  mode - 睡眠模式选择
 * @note   进入低功耗前关闭LIN主机引脚，退出后恢复空闲状态
 * @retval 无
 */
static void lin_sleep_mode_set(sleep_mode_e mode)
{
#ifdef CFG_LIN_CONFORM_TEST
    lin_slave_wakeup_flag = 0;
#endif
#if CFG_SUPPORT_LIN_MASTER
#ifdef  __TCPL03X__
    pal_gpio_output(GPIO_PIN_5, false);
#endif
#endif
    /* low power enter */
    pmu_lpm_enter(mode);

#ifdef CFG_LIN_CONFORM_TEST
    lin_goto_idle_state();
#endif

    /* low power exit */
    pmu_lpm_exit();
}

/**
 * @brief  系统低功耗初始化
 * @param  无
 * @note   初始化PMU低功耗管理，一致性测试时额外初始化GPIO唤醒
 * @retval 无
 */
void system_low_power_init(void)
{
    pmu_lpm_init();

#ifdef CFG_LIN_CONFORM_TEST
    lin_gpio_wakeup_init(SLEEPWALK_MODE);
#endif
}

/**
 * @brief  进入睡眠模式入口
 * @param  无
 * @note   检查睡眠标志后进入睡眠，唤醒后清除测量数据并恢复LED显示
 * @retval 无
 */
void sleep_mode_enter(void)
{
#ifdef CFG_LIN_CONFORM_TEST
    lin_slave_wakeup_master_handle();
#endif

    if (lin_goto_sleep_flg == 1)
    {
        lin_goto_sleep_flg = 0;

        lin_sleep_mode_set(SYSTEM_SLEEP_MODE);
        /* meas_manager_value_clear */
        meas_manager_value_clear();
        /* led color restore */
        led_disp_wakeup_recovery_handle();
    }
}
