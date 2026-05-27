/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $11 ECU复位处理模块
 *
 * 本模块实现了UDS协议SID $11（ECUReset）服务的处理逻辑。
 * 支持硬件复位（子功能0x01），在复位前先发送正响应报文通知诊断仪，
 * 然后关闭看门狗以防止复位过程中看门狗超时误触发，最后调用NVIC_SystemReset()
 * 执行MCU硬件复位。对于不支持的子功能值，统一返回SFNS（子功能不支持）负响应。
 *
 * @file    sid_0x11.c
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

#include "test_config.h"
#ifdef ENABLE_TEST_MODE
#include "fff_diagnosticIII.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#endif

/** @brief 看门狗使能控制函数（外部声明），用于在ECU复位前禁用看门狗，防止复位过程中看门狗超时触发系统复位 */
extern void ll_wdg_enable(bool enable);

/**
 * @brief  SID $11 ECU复位处理函数
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   子功能0x01 = 硬件复位：先发送正响应，再调用NVIC_SystemReset()复位MCU。
 *         其他子功能返回SFNS（子功能不支持）负响应。
 * @retval None (通过 lin_diag_positive_notify / lin_diag_negative_notify 返回)
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_ecu_reset(uint8_t *ptr, uint16_t length)
{	
    (void)length;
    switch (ptr[1]) /*!< 根据请求报文首个数据字节（子功能字节）选择对应的复位类型进行处理 */
    {
        /* ISO14429 also supports several kinds of reset to do */
        case 0x01: /*!< 子功能0x01：硬件复位，执行MCU硬件复位流程 */
            /* hardware reset  to do*/
            lin_diag_positive_notify(ptr[0], &ptr[1], 1); /*!< 发送正响应报文（SID + 子功能），通知诊断仪即将执行复位操作 */
            ll_wdg_enable(false); /*!< 关闭看门狗定时器，防止复位过程中看门狗超时误触发系统复位 */
            NVIC_SystemReset(); /*!< 调用硬件复位函数立即复位MCU，重启微控制器 */
            break;

        default : /*!< 未定义或不支持的子功能值，统一返回SFNS负响应 */
            lin_diag_negative_notify(ptr[0], SFNS); /*!< 发送SFNS（子功能不支持）负响应码 */
            break;
    }
}
