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
#include "fff_utilities.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "utilities.h"
#endif

/********************************************************
** \brief   lin_diag_tester_present
** \param   uint8_t*                    ptr
** \param   uint16_t                    length
** \retval  None
*********************************************************/
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_tester_present(uint8_t *ptr, uint16_t length)
{
    (void)length;
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

