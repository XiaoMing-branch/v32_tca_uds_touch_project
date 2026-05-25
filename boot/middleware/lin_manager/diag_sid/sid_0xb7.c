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
#include "lin_precfg.h"

#if LIN_PROTOCOL == PROTOCOL_21
/**
 * @brief  SID $B7 AssignFrameIDRange分配帧ID范围(LIN 2.1)
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   请求格式: ptr[1]=起始索引, ptr[2~5]=4个PID值
 *         检查帧索引是否超出lin_cfg最大帧数(0xFF值跳过)
 *         PID值0x00=取消分配(设为0xFF), 0xFF=保持原值, 其他=计算校验位后分配
 *         报文长度不为6时返回GENERAL_REJECT
 * @retval None
 */
void lin_diag_assign_frame_id_range(uint8_t *ptr, uint16_t length)
{
    uint8_t start_index;
    uint8_t i, j;

    if (length != 6)
    {
        lin_diag_negative_notify(ptr[0], GENERAL_REJECT);
        return;
    }

    start_index = ptr[1];

    for (i = 5, j = start_index + 4; j > start_index; i--, j--)
    {
        if (ptr[i] != 0xFF && j > lin_cfg.lin_cfg_frame_num)
        {
            lin_diag_negative_notify(ptr[0], GENERAL_REJECT);
            return;
        }
    }

    /* Store PIDs */
    for (i = 2, j = start_index + 1; i < length; i++, j++)
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
                lin_configuration_RAM[j] = lin_process_parity(ptr[i], CHECK_PARITY);
                break;
        }
    } /* End of for statement */

    lin_diag_positive_notify(ptr[0], NULL, 0);
}
#endif
