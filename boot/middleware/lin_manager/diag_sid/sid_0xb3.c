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

#if LIN_PROTOCOL != PROTOCOL_J2602
/**
 * @brief  SID $B3 ConditionalChangeNAD条件NAD变更(LIN 2.0非J2602)
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   请求格式: ptr[1]=ID, ptr[2]=Byte, ptr[3]=Mask, ptr[4]=Invert, ptr[5]=新NAD
 *         ID必须为0, Byte指定产品标识字节(1~5):
 *         1-2: SupplierID LSB/MSB, 3-4: FunctionID LSB/MSB, 5: Variant
 *         条件判断: (ProductByte ^ Invert) & Mask == 0 成立时分配新NAD
 * @retval None
 */
void lin_diag_conditional_change_nad(uint8_t *ptr, uint16_t length)
{
    uint8_t id, byte, mask, invert;

    id      = ptr[1];
    byte    = ptr[2];
    mask    = ptr[3];
    invert  = ptr[4];

    /* Possible positive ID */
    if (id == 0)
    {
        if (byte > 0 && byte < 6)
        {
            /*Byte 1: Supplier ID LSB; Byte 2: Supplier ID MSB*/
            if (byte > 0 && byte < 3)
            {
                byte = product_id.supplier_id >> ((byte - 1) * 8);
            }
            /*Byte 3: Function ID LSB; Byte 4: Function ID MSB*/
            else if (byte > 2 && byte < 5)
            {
                byte = product_id.function_id >> ((byte - 3) * 8);
            }
            /* Byte 5: Variant */
            else
            {
                byte = product_id.variant;
            }

            /* Do a bitwise XOR with Invert and Do a bitwise AND with Mask */
            byte = (byte ^ invert)&mask;

            /* If the final result is zero, then give positive response*/
            if (byte == 0)
            {
                lin_diag_positive_notify(ptr[0], NULL, 0);
                /* If the final result is zero then change the NAD to New NAD */
                lin_configured_NAD = ptr[5];
            }
        }
    }
}
#endif
