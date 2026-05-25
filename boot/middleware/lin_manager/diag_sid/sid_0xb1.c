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

#if ((LIN_PROTOCOL == PROTOCOL_20) || (LIN_PROTOCOL == PROTOCOL_J2602))

/**
 * @brief  SID $B1 AssignFrameIdentifier分配帧标识符(LIN 2.0/J2602)
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   请求格式: ptr[1-2]=SupplierID, ptr[3-4]=MessageID, ptr[5]=新PID
 *         验证SupplierID后，检查新ID是否已被其他帧占用(冲突检测)
 *         在lin_configuration_ROM中查找匹配的MessageID，更新对应lin_configuration_RAM
 * @retval None
 */
void lin_diag_assign_frame_identifier(uint8_t *ptr, uint16_t length)
{
    uint8_t id, pid, i;
    uint16_t supid, messageid;

    /* Get supplier and function indentification in request */
    supid = (uint16_t)(ptr[2] << 8) | ptr[1];
    messageid = (uint16_t)(ptr[4] << 8) | ptr[3];

    pid = ptr[5];
    id = lin_process_parity(pid, CHECK_PARITY);

    /* Check if id is already assign for the other frame */
    i = 1;

    while (lin_configuration_ROM[i] != 0xFFFF)
    {
        if ((id == lin_configuration_RAM[i]) && (lin_configuration_ROM[i] != messageid))
        {
            id = 0xFF;
            break;
        }

        i++;
    }

    /* Check Supplier ID and Function ID */
    if (((supid != product_id.supplier_id) && (supid != LD_ANY_SUPPLIER)) || (id == 0xFF))
    {
        tl_slaveresp_cnt = 0;
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
        tl_service_status = LD_SERVICE_IDLE;
#endif /* End (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_) */
        return;
    }

    /* Check if exist message id */
    i = 1;

    while (lin_configuration_ROM[i] != 0xFFFF)
    {
        if (lin_configuration_ROM[i] == messageid)
        {
            lin_configuration_RAM[i] = id;
            /* Send positive response */
            lin_diag_positive_notify(ptr[0], NULL, 0);
            break;
        }

        i++;
    }
}

#endif /* end of LIN_PROTOCOL == PROTOCOL_20 or LIN_PROTOCOL == PROTOCOL_J2602 */
