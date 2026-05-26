/**
 *****************************************************************************
 * @brief   interrupt Source file.
 *
 * @file    tcpl03x_ll_cortex.c
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

#include "tcpl03x_ll_def.h"
#include "tcpl03x_ll_cortex.h"

/**
 * @brief   NMI中断处理函数，系统复位
 *
 * @param   None
 *
 * @retval  None
 *********************************************************/
void NMI_Handler(void)
{
    NVIC_SystemReset();                         /* 系统复位 */
}

/**
 * @brief   硬件错误中断处理函数，系统复位
 *
 * @param   None
 *
 * @retval  None
 *********************************************************/
void HardFault_Handler(void)
{
    NVIC_SystemReset();                         /* 系统复位 */
}

/**
 * @brief   使能NVIC中断
 *
 * @param   irq     中断号
 * @param   level   中断优先级
 * @param   en      使能标志（true为使能，false为禁能）
 *
 * @retval  None
 *
 * @note    Cortex-M0/M0+不支持已使能中断或异常的优先级动态更改，
 *          请在使能中断前设置好优先级
 *********************************************************/
void EnableNvic(uint32_t irq, uint8_t level, bool en)
{
    IRQn_Type enIrq = (IRQn_Type)irq;           /* 转换中断号类型 */
    NVIC_DisableIRQ(enIrq);                      /* 先禁能中断，避免配置冲突 */
    NVIC_ClearPendingIRQ(enIrq);                 /* 清除中断挂起状态 */
    NVIC_SetPriority(enIrq, level);              /* 设置中断优先级 */

    if (true == en)
    {
        NVIC_EnableIRQ(enIrq);                   /* 使能中断 */
    }
}
