/**
 *****************************************************************************
 * @brief   wdg driver source file.
 *
 * @file    tcpl03x_ll_wdg.c
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

#include "tcpl03x_ll_wdg.h"
#include "tcpl03x_ll_cortex.h"

static ISR_FUNC_CALLBACK wdg_callback = NULL;

/**
 * @brief   初始化看门狗
 *
 * @param   config      看门狗配置结构体指针，包含时钟、中断和溢出行为等配置参数
 * @param   callback    看门狗中断回调函数指针（仅在使能中断时有效）
 *
 * @retval  None
 */
void ll_wdg_init(wdg_config_t *config, ISR_FUNC_CALLBACK callback)
{
    ll_wdg_deinit();

    CRG_CONFIG_UNLOCK();
    CRG->IWDG_CLKRST_CTRL_F.PCLK_EN_IWDG = 1; /* enable wdg pclk*/
    CRG_CONFIG_LOCK();

    WDG_UNLCOK();
    IWDG->CTRL_F.DBG_STOP_EN = 1;
    IWDG->CTRL_F.RST_EN = (config->reset_on_overflow) ? 1 : 0;
    IWDG->CNT_MAX_F.CNT_MAX = (config->max_count & 0xFFFFF);
    IWDG->IRQ_F.IMR = !config->isr_cfg.isr_enable;
    IWDG->CCR_F.CCR = 1;
    WDG_LCOK();

    if (config->isr_cfg.isr_enable)
    {
        wdg_callback = callback;

        NVIC_ClearPendingIRQ(IWDG_IRQn);
        NVIC_SetPriority(IWDG_IRQn, config->isr_cfg.priority);
    }
}

/**
 * @brief   反初始化看门狗
 *
 * @retval  None
 */
void ll_wdg_deinit(void)
{
    NVIC_DisableIRQ(IWDG_IRQn);
    wdg_callback = NULL;

    WDG_UNLCOK();
    IWDG->CTRL = 0;
    IWDG->CNT_MAX_F.CNT_MAX = 0;
    WDG_LCOK();

    CRG_CONFIG_UNLOCK();
    CRG->IWDG_CLKRST_CTRL_F.PCLK_EN_IWDG = 0;
    CRG_CONFIG_LOCK();
}

/**
 * @brief   使能或禁止看门狗中断
 *
 * @param   enable  true  - 使能看门狗中断（使能 NVIC）
 *                  false - 禁止看门狗中断（禁止 NVIC）
 *
 * @retval  None
 */
void ll_wdg_isr_enable(bool enable)
{
    NVIC_ClearPendingIRQ(IWDG_IRQn);

    if (enable)
    {
        NVIC_EnableIRQ(IWDG_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(IWDG_IRQn);
    }
}

/**
 * @brief   使能或禁止看门狗计数器
 *
 * @param   enable  true  - 启动看门狗计数
 *                  false - 停止看门狗计数
 *
 * @retval  None
 */
void ll_wdg_enable(bool enable)
{
    WDG_UNLCOK();
    IWDG->CTRL_F.EN = (enable) ? 1 : 0;
    WDG_LCOK();
}

/**
 * @brief   刷新看门狗计数器（喂狗）
 *
 * @note    向 CCR 寄存器写入 1 以重载计数器，防止看门狗溢出复位/中断
 *
 * @retval  None
 */
void ll_wdg_reload(void)
{
    WDG_UNLCOK();
    IWDG->CCR_F.CCR = 1;
    WDG_LCOK();
}

/**
 * @brief   清除看门狗中断标志
 *
 * @note    向 ICR 寄存器写入 1 以清除挂起的中断标志位
 *
 * @retval  None
 */
void ll_wdg_interrupt_clear(void)
{
    WDG_UNLCOK();
    IWDG->IRQ_F.ICR = 1;
    WDG_LCOK();
}

/**
 * @brief   看门狗中断服务函数
 *
 * @note    该函数由硬件中断向量自动调用，执行以下操作：
 *          - 清除看门狗中断标志
 *          - 刷新看门狗计数器
 *          - 调用用户注册的回调函数（若已注册）
 *
 * @retval  None
 */
void IWDG_IRQHandler(void)
{
    ll_wdg_interrupt_clear();
    ll_wdg_reload();

    if (NULL != wdg_callback)
    {
        wdg_callback(0);
    }
}
