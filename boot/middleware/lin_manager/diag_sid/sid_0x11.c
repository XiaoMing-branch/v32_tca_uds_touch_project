/**
 *****************************************************************************
 * @brief   lin dianosticiii source file.
 *
 * @file    diagnosticiii.c
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

#include "diagnosticIII.h"

extern void wdg_enable(bool enable);

/**
 * @brief  SID $11 ECU复位处理函数
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   支持硬件复位(子功能0x01): 先发送正响应，关闭看门狗，再执行NVIC系统复位
 *         其他子功能返回SFNS(子功能不支持)
 * @retval None
 */
void lin_diag_ecu_reset(uint8_t *ptr, uint16_t length)
{
    switch (ptr[1])
    {
        /* ISO14429 还支持好几种复位 to do */
        case 0x01:
            /* hardware reset  to do*/
            lin_diag_positive_notify(ptr[0], &ptr[1], 1);
            wdg_enable(false);
            NVIC_SystemReset();
            break;

        default :
            lin_diag_negative_notify(ptr[0], SFNS);
            break;
    }
}
