/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
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

#include "test_config.h"
#ifdef ENABLE_TEST_MODE
#include "fff_diagnosticIII.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#endif

#if LIN_PROTOCOL != PROTOCOL_J2602
/********************************************************
** \brief   lin_diag_conditional_change_nad
** \param   uint8_t*                    ptr
** \param   uint16_t                    length
** \retval  None
*********************************************************/
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
