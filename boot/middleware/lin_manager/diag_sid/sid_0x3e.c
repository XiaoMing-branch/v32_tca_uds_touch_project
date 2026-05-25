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
#include "utilities.h"

/**
 * @brief  SID $3E TesterPresent处理(Boot版本)
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   子功能0x00: 带正响应指示，回复正响应(保持会话激活)
 *         子功能0x80: 无响应指示，静默处理(不发送任何响应)
 *         其他子功能: 返回SFNS(子功能不支持)
 * @retval None
 */
void lin_diag_tester_present(uint8_t *ptr, uint16_t length)
{
    switch (ptr[1])
    {
        case 0x00u ://supportPosRspMsgIndicationBit=0
            lin_diag_positive_notify(ptr[0], &ptr[1], 1);
            break;

        case 0x80u ://supportPosRspMsgIndicationBit=1
            break;

        default :
            lin_diag_negative_notify(ptr[0], SFNS);//sub-functionNotSupported
            break;
    }
}

