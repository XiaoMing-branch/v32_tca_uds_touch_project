/**
 *****************************************************************************
 * @brief   lpm Source file.
 *
 * @file    tcae10_ll_lpm.c
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

#include "tcae10_ll_lpm.h"
#include "tcae10_ll_cortex.h"

/**
 * @brief   进入深度睡眠模式
 * @param   on_exit - 当设置为true时，从异常处理返回线程模式时自动进入睡眠（SLEEPONEXIT）
 *                    当设置为false时，立即执行WFI进入睡眠
 * @note    设置SLEEPDEEP位选择深度睡眠，通过SCR寄存器控制SLEEPONEXIT行为
 * @retval  None
 */
static void ll_lpm_deep_sleep_enter(bool on_exit)
{
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;          /* 设置SLEEPDEEP位，选择深度睡眠模式 */

    if (on_exit)
    {
        SCB->SCR |= (0x01 << 1);                /* 设置SLEEPONEXIT，异常退出后自动睡眠 */
    }
    else
    {
        SCB->SCR &= ~(0x01 << 1);               /* 清除SLEEPONEXIT */
    }

    __WFI();                                    /* 等待中断唤醒 */
}

/**
 * @brief   进入普通睡眠模式
 * @param   on_exit - 当设置为true时，从异常处理返回线程模式时自动进入睡眠（SLEEPONEXIT）
 *                    当设置为false时，立即执行WFI进入睡眠
 * @note    清除SLEEPDEEP位选择普通睡眠模式，通过SCR寄存器控制
 * @retval  None
 */
static void ll_lpm_normal_sleep_enter(bool on_exit)
{
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;         /* 清除SLEEPDEEP位，选择普通睡眠模式 */

    if (on_exit)
    {
        SCB->SCR |= (0x01 << 1);                /* 设置SLEEPONEXIT */
    }
    else
    {
        SCB->SCR &= ~(0x01 << 1);               /* 清除SLEEPONEXIT */
    }

    __WFI();                                    /* 等待中断唤醒 */
}

/**
 * @brief   获取当前睡眠模式
 * @param   None
 * @note    从ASYSCFG的SLEEP_MODE寄存器读取当前睡眠模式（低2位）
 * @retval  当前睡眠模式：0-IDLE, 1-SLEEP, 2-SLEEPWALK, 3-DEEPSLEEP
 */
uint8_t ll_sleep_mode_get(void)
{
    return (ASYSCFG->SLEEP_MODE & 0x3);          /* 读取睡眠模式，仅取低2位 */
}

/**
 * @brief   MCU进入低功耗模式（顶层入口）
 * @param   state   - 低功耗模式选择：IDLE_MODE/SLEEP_MODE/SLEEPWALK_MODE/DEEPSLEEP_MODE
 * @param   on_exit - 是否在异常退出后自动进入睡眠（SLEEPONEXIT）
 * @note    根据选择的模式配置ASYSCFG睡眠模式寄存器，并调用对应睡眠进入函数
 * @retval  None
 */
void ll_lpm_mcu_enter(sleep_mode_e state, bool on_exit)
{
    ASYSCFG_CONFIG_UNLOCK();

    switch (state)
    {
        case SLEEP_MODE_MAX:
            break;

        case IDLE_MODE:
            ASYSCFG->SLEEP_MODE = SLPMODE_IDLE;
            ll_lpm_normal_sleep_enter(on_exit);
            break;

        case SLEEPWALK_MODE:
            ASYSCFG->SLEEP_MODE = SLPMODE_SLEEPWALK;
            ll_lpm_deep_sleep_enter(on_exit);
            break;

        case SLEEP_MODE:
            ASYSCFG->SLEEP_MODE = SLPMODE_SLEEP;
            ll_lpm_deep_sleep_enter(on_exit);
            break;

        case DEEPSLEEP_MODE:
            ASYSCFG->SLEEP_MODE_F.SLP_MODE = SLPMODE_DEEPSLEEP;
            ll_lpm_deep_sleep_enter(on_exit);
            break;

        default:
            break;
    }

    ASYSCFG_CONFIG_LOCK();
}

/**
 * @brief   模拟前端（AFE）进入低功耗模式
 * @param   mode - 低功耗模式选择
 * @note    当前为空函数，预留AFE低功耗控制接口
 * @retval  None
 */
void ll_lpm_afe_enter(sleep_mode_e mode)
{
}

/**
 * @brief   配置GPIO进入低功耗状态
 * @param   None
 * @note    将LED引脚切换为GPIO模式，降低低功耗模式下引脚漏电
 * @retval  None
 */
void ll_pmu_gpio_lowpower(void)
{
    // /* config LED as led */
    PINMUX->LED0_CFG_F.LED0_SRC_SEL = 0;
    PINMUX->LED1_CFG_F.LED1_SRC_SEL = 0;
    PINMUX->LED2_CFG_F.LED2_SRC_SEL = 0;

    /* config gpio */
    // PINMUX->IO2_CFG_F.IO2_SRC_SEL = 0;
}

/**
 * @brief   使能或禁能LDO Dummy负载
 * @param   enable - true启用dummy负载，false关闭
 * @note    控制LDO15和LDO33的dummy负载开关和偏置电流选择
 * @retval  None
 */
void ll_pmu_ldo_dummy_enable(bool enable)
{
    ASYSCFG->LDO15_CTRL_F.LDO15_DL_SW_ENB = !enable;

    ASYSCFG->LDO15_CTRL_F.LDO15_DL_IBASE_SEL = 0;
    ASYSCFG->LDO33_CTRL_F.LDO33_DL_SW_ENB = !enable;
    ASYSCFG->LDO33_CTRL_F.LDO33_DL_IBASE_SEL = 0;
}
