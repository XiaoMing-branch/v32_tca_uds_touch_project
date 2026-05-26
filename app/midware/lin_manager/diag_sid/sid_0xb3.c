/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $B3 条件改变节点地址处理模块（ConditionalChangeNAD）
 *
 * @file    sid_0xb3.c
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

#if LIN_PROTOCOL != PROTOCOL_J2602
/**
 * @brief  SID $B3 条件改变节点地址（ConditionalChangeNAD）处理函数
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   仅当product_id的指定字节经过（XOR Invert & AND Mask）运算结果为0时，
 *         才将NAD改为新值。匹配条件支持：
 *         - Byte 1~2：Supplier ID
 *         - Byte 3~4：Function ID
 *         - Byte 5：Variant
 *         ID=0表示执行条件匹配（LIN规范定义）。
 * @retval None (通过 lin_diag_positive_notify 返回)
 */
/* PRQA S 3673 4 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diag_conditional_change_nad(uint8_t *ptr, uint16_t length)
{
    (void)length;
    uint8_t id, byte, mask, invert;

    id      = ptr[1];
    byte    = ptr[2];
    mask    = ptr[3];
    invert  = ptr[4];

    /* Possible positive ID */
    if (id == 0u)
    {
        if ((byte > 0u) && (byte < 6u))
        {
            /*Byte 1: Supplier ID LSB; Byte 2: Supplier ID MSB*/
            if (byte < 3u)
            {
                byte = (uint8_t)(product_id.supplier_id >> ((byte - 1u) * 8u));
            }
            /*Byte 3: Function ID LSB; Byte 4: Function ID MSB*/
            else if (byte < 5u)
            {
                byte = (uint8_t)product_id.function_id >> ((byte - 3u) * 8u);
            }
            /* Byte 5: Variant */
            else
            {
                byte = product_id.variant;
            }

            /* Do a bitwise XOR with Invert and Do a bitwise AND with Mask */
            byte = (byte ^ invert)&mask;

            /* If the final result is zero, then give positive response*/
            if (byte == 0u)
            {
                lin_diag_positive_notify(ptr[0], NULL, 0);
                /* If the final result is zero then change the NAD to New NAD */
                lin_configured_NAD = ptr[5];
            }
        }
    }
}
#endif
