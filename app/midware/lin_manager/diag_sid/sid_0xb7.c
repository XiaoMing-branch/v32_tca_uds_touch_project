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
#include "fff_lin_precfg.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "lin_precfg.h"
#endif

#if LIN_PROTOCOL == PROTOCOL_21
/**
 * @brief  SID $B7 分配帧ID范围（AssignFrameIDRange）处理函数（LIN 2.1）
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   报文长度须为6字节。从start_index开始分配4个连续帧ID：
 *         - 0x00：取消分配该帧（设为0xFF）
 *         - 0xFF：保持该帧之前分配的值不变
 *         - 其他值：通过lin_process_parity()计算奇偶校验后的帧ID并分配
 *         若PID奇偶校验失败则返回GENERAL_REJECT负响应。
 * @retval None (通过 lin_diag_positive_notify / lin_diag_negative_notify 返回)
 */
/* PRQA S 2889 2 #3257 - Multiple return statements for logical clarity and efficiency */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diag_assign_frame_id_range(uint8_t *ptr, uint16_t length)
{
    uint8_t start_index;
    uint8_t i, j;
    uint8_t fid;

    if (length != 6u)
    {
        lin_diag_negative_notify(ptr[0], GENERAL_REJECT);
        return;
    }

    start_index = ptr[1];

    i = 5u;
    j = start_index + 4u;
/* PRQA S 3387 1 #3265 - Increment or decrement operation is safe with no unintended side effects */
    for (; j > start_index; j--)
    {
        if ((ptr[i] != 0xFFu) && (j > lin_cfg.lin_cfg_frame_num))
        {
            return;
        }
	i--;
    }

    /* Store PIDs */
    i = 2u; 
    j = start_index + 1u;
/* PRQA S 3387 1 #3265 - Increment or decrement operation is safe with no unintended side effects */
    for (; i < length; i++)
    {
        switch (ptr[i])
        {
            case 0x00:
                /* Unassign frame */
                lin_configuration_RAM[j] = 0xFF;
                break;

            case 0xFF:
                /* keep the previous assigned value of this frame */
                break;

            default:
                /* Calculate frame ID & Assign ID to frame */
                fid = lin_process_parity(ptr[i], CHECK_PARITY);
                if (0xFFu == fid)
                {
                    lin_diag_negative_notify(ptr[0], GENERAL_REJECT);
                    return;
                }
                else
                {
                    lin_configuration_RAM[j] = fid;
                }
                break;
        }
	j++;
    } /* End of for statement */

    lin_diag_positive_notify(ptr[0], NULL, 0);
}
#endif
