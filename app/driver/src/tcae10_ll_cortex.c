/**
 *****************************************************************************
 * @brief   interrupt Source file.
 *
 * @file    tcae10_ll_cortex.c
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

#include "tcae10_ll_def.h"
#include "tcae10_ll_cortex.h"

/**
 * @brief   NMI（不可屏蔽中断）处理函数
 * @note    TCAE10 MCU NMI异常入口，触发时执行系统复位
 * @retval  None
 */
void NMI_Handler(void)
{
//    while (1)
//    {
//        ;
//    }
    NVIC_SystemReset();
}

/**
 * @brief   硬故障（HardFault）处理函数
 * @note    ARM Cortex-M0+硬故障异常入口，触发时执行系统复位
 * @retval  None
 */
void HardFault_Handler(void)
{
//    while (1)
//    {
//        ;
//    }
    NVIC_SystemReset();
}

/**
 * @brief   配置并使能/禁能NVIC中断
 * @param   irq    - 中断号，对应IRQn_Type枚举
 * @param   level  - 中断优先级（0~3，Cortex-M0+仅支持4级优先级）
 * @param   en     - true使能中断，false禁能中断
 * @note    Cortex-M0/M0+不支持运行时动态更改已使能中断的优先级，
 *         必须在使能中断前设置好优先级
 * @retval  None
 */
void EnableNvic(uint32_t irq, uint8_t level, bool en)
{
    IRQn_Type enIrq = (IRQn_Type)irq;
    NVIC_DisableIRQ(enIrq);
    NVIC_ClearPendingIRQ(enIrq);
    NVIC_SetPriority(enIrq, level);

    if (true == en)
    {
        NVIC_EnableIRQ(enIrq);
    }
}

/**
 * @brief   全局使能所有中断（除NMI和HardFault外）
 * @note    对应ARM CPSIE I指令，清除PRIMASK寄存器
 * @retval  None
 */
void interrupt_enable(void)
{
    __enable_irq();
}

/**
 * @brief   全局禁能所有中断（除NMI和HardFault外）
 * @note    对应ARM CPSID I指令，设置PRIMASK寄存器
 * @retval  None
 */
void interrupt_disable(void)
{
    __disable_irq();
}
