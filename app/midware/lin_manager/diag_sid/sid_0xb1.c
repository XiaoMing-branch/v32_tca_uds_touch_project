/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $B1 分配帧标识符（AssignFrameIdentifier）处理源文件
 *
 * @file    sid_0xb1.c
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

#if ((LIN_PROTOCOL == PROTOCOL_20) || (LIN_PROTOCOL == PROTOCOL_J2602) || (LIN_PROTOCOL == PROTOCOL_21))

/**
 * @brief  SID $B1 分配帧标识符（AssignFrameIdentifier）处理函数
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   LIN 2.0/J2602协议帧ID分配/删除操作。
 *         - PID=0x40时删除帧：在配置ROM中查找匹配的messageid，将RAM中对应项置0xFF。
 *         - 否则分配帧：校验PID奇偶位，检查帧ID是否已被其他消息占用。
 *         执行Supplier ID和Function ID匹配校验，匹配成功后将帧ID写入lin_configuration_RAM。
 * @retval None (通过 lin_diag_positive_notify 返回)
 */
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
void lin_diag_assign_frame_identifier(uint8_t *ptr, uint16_t length)
{
    (void)length;
    uint8_t id, pid, i;
    uint16_t supid, messageid;

    /* Get supplier and function indentification in request */
    supid = ((uint16_t)ptr[2] << 8) | ptr[1];
    messageid = ((uint16_t)ptr[4] << 8) | ptr[3];

    pid = ptr[5];

    if (pid == 0x40u)    //Delete frameid
    {
        i = 1u;
        while (lin_configuration_ROM[i] != 0xFFFFu)
        {
            if (lin_configuration_ROM[i] == messageid)
            {
                lin_configuration_RAM[i] = 0xFF;
                /* Send positive response */
                lin_diag_positive_notify(ptr[0], NULL, 0);
                break;
            }
            i++;
        }
        return;
    }

    id = lin_process_parity(pid, CHECK_PARITY);

    /* Check if id is already assign for the other frame */
    i = 1;
    while (lin_configuration_ROM[i] != 0xFFFFu)
    {
        if ((id == lin_configuration_RAM[i]) && (lin_configuration_ROM[i] != messageid))
        {
            id = 0xFF;
            break;
        }

        i++;
    }

    /* Check Supplier ID and Function ID */
    if (((supid != product_id.supplier_id) && (supid != (uint16_t)LD_ANY_SUPPLIER)) || (id == 0xFFu))
    {
        tl_slaveresp_cnt = 0;
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
        tl_service_status = LD_SERVICE_IDLE;
#endif /* End (_TL_FRAME_SUPPORT_ = = _TL_MULTI_FRAME_) */
        return;
    }

    /* Check if exist message id */
    i = 1;
    while (lin_configuration_ROM[i] != 0xFFFFu)
    {
        if (lin_configuration_ROM[i] == messageid)
        {
            lin_configuration_RAM[i] = id;
            /* Send positive response */
            lin_diag_positive_notify(ptr[0], NULL, 0);
            if (id == 0x00u)
            {
                lin_pFrameBuf[8] = 0xaa;
                lin_pFrameBuf[9] = 0xbb;
                lin_frame_flag_tbl[1] = 0;
            }
            break;
        }

        i++;
    }
}

#endif /* end of LIN_PROTOCOL == PROTOCOL_20 or LIN_PROTOCOL == PROTOCOL_J2602 */
