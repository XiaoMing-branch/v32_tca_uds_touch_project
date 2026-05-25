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
#include "fff_pal_store.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "utilities.h"
#include "pal_store.h"
#endif

/********************************************************
** \brief   soc_reg_read
** \param   uint8_t*                    ptr
** \param   uint16_t                    length
** \retval  None
*********************************************************/
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void soc_reg_read(uint8_t *ptr, uint16_t length)
{
    (void)length;
    uint32_t addr = 0;
    (void)(&addr);
    uint32_t tt = 0;

    addr = endian_swap_func((uint8_t *)&ptr[2], sizeof(uint32_t));

    pal_store_reg_rw(false, addr, &tt);

/* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    endian_swap_func((uint8_t *)&tt, sizeof(uint32_t));
    lin_diag_positive_notify(ptr[0], (uint8_t *)&tt, sizeof(uint32_t));
}
