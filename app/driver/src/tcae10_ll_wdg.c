/**
 *****************************************************************************
 * @brief   wdg driver source file.
 *
 * @file    tcae10_ll_wdg.c
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

#include "tcae10_ll_wdg.h"
#include "tcae10_ll_cortex.h"

/** @defgroup IWDG_LOCK_Definitions
  * @{
  */
#if 1
    #define WDG_UNLCOK()    (IWDG->LOCK=0XAAAA5555)  /* 解锁IWDG寄存器 */
    #define WDG_LCOK()      (IWDG->LOCK=0X12345678)  /* 锁定IWDG寄存器 */
#else
    #define WDG_UNLCOK()
    #define WDG_LCOK()
#endif

static ISR_FUNC_CALLBACK wdg_callback = NULL;

/**
 * @brief   初始化独立看门狗（IWDG）
 * @param   config   - 看门狗配置参数指针（包含最大计数值、溢出复位使能、中断配置）
 * @param   callback - 看门狗中断回调函数指针
 * @note    先调用ll_wdg_deinit()复位，使能IWDG时钟，配置计数值和中断掩码，
 *         写CCR触发计数器加载
 * @retval  None
 */
void ll_wdg_init(wdg_config_t *config, ISR_FUNC_CALLBACK callback)
{
    ll_wdg_deinit();

    CRG_CONFIG_UNLOCK();
    CRG->IWDG_CLKRST_CTRL_F.PCLK_EN_IWDG = 1; /* 使能IWDG外设时钟 */
    CRG_CONFIG_LOCK();

    WDG_UNLCOK();
    IWDG->CTRL_F.DBG_STOP_EN = 1;             /* 调试模式下停止计数 */
    IWDG->CTRL_F.RST_EN = (config->reset_on_overflow) ? 1 : 0; /* 溢出是否产生复位 */
    IWDG->CNT_MAX_F.CNT_MAX = (config->max_count & 0xFFFFF);   /* 设置最大计数值（20位） */
    IWDG->IRQ_F.IMR = !config->isr_cfg.isr_enable;             /* 配置中断掩码 */
    IWDG->CCR_F.CCR = 1;                     /* 写CCR触发计数器重载 */
    WDG_LCOK();


    // if (config->isr_cfg.isr_enable)
    // {
    //     if (NULL != callback)
    //     {
    //         wdg_callback = callback;
    //     }
    //     NVIC_ClearPendingIRQ(IWDG_IRQn);
    //     NVIC_SetPriority(IWDG_IRQn, config->isr_cfg.priority);
    // }
}

/**
 * @brief   反初始化独立看门狗（IWDG）
 * @param   None
 * @note    禁能NVIC中断，复位IWDG控制寄存器，关闭时钟
 * @retval  None
 */
void ll_wdg_deinit(void)
{
    NVIC_DisableIRQ(IWDG_IRQn);
    wdg_callback = NULL;

    WDG_UNLCOK();
    IWDG->CTRL = 0;                          /* 复位控制寄存器 */
    IWDG->CNT_MAX_F.CNT_MAX = 0;             /* 清空最大计数值 */
    WDG_LCOK();

    CRG_CONFIG_UNLOCK();
    CRG->IWDG_CLKRST_CTRL_F.PCLK_EN_IWDG = 0; /* 关闭IWDG时钟 */
    CRG_CONFIG_LOCK();
}

/**
 * @brief   使能或禁能看门狗中断（NVIC层）
 * @param   enable - true使能IWDG中断，false禁能
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
 * @brief   使能或禁能看门狗计数
 * @param   enable - true启动看门狗计数，false停止
 * @retval  None
 */
void ll_wdg_enable(bool enable)
{
    WDG_UNLCOK();
    IWDG->CTRL_F.EN = (enable) ? 1 : 0;     /* 设置使能位 */
    WDG_LCOK();
}

/**
 * @brief   重载看门狗计数器（喂狗）
 * @note    写入CCR寄存器触发计数器重新加载，防止看门狗复位
 * @retval  None
 */
void ll_wdg_reload(void)
{
    WDG_UNLCOK();
    IWDG->CCR_F.CCR = 1;                    /* 触发计数器重载 */
    WDG_LCOK();
}

/**
 * @brief   清除看门狗中断标志
 * @note    写ICR寄存器清除中断状态
 * @retval  None
 */
void ll_wdg_interrupt_clear(void)
{
    WDG_UNLCOK();
    IWDG->IRQ_F.ICR = 1;                    /* 清除中断标志 */
    WDG_LCOK();
}

/**
 * @brief   IWDG中断处理函数
 * @note    （注释掉）- 清除中断并重载看门狗，调用用户回调
 * @retval  None
 */
// void IWDG_IRQHandler(void)
// {
//     ll_wdg_interrupt_clear();
//     ll_wdg_reload();

//     if (NULL != wdg_callback)
//     {
//         wdg_callback(0);
//     }
// }
