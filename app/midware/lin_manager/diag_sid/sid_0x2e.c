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

/**
 * @brief  SID $2E 通过标识符写数据处理函数
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   存根实现，直接返回正响应。未执行实际的数据写入操作。
 *         后续可根据DID（数据标识符）实现对应参数写入逻辑。
 * @retval None (通过 lin_diag_positive_notify 返回)
 */
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_write_by_identifier(uint8_t *ptr, uint16_t length)
{
    (void)length;
    lin_diag_positive_notify(ptr[0], NULL, 0);
}
