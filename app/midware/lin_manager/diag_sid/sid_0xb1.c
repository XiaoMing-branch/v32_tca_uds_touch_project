/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   LIN诊断配置服务 - SID $B1 分配帧标识符（AssignFrameIdentifier）处理源文件
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
 * @param  ptr - UDS请求报文指针，包含Supplier ID、Function ID、Message ID及PID
 * @param  length - 报文长度
 * @note   LIN 2.0/J2602协议帧ID分配/删除操作。
 *         - PID=0x40时删除帧：在配置ROM中查找匹配的messageid，将RAM中对应项置0xFF。
 *         - 否则分配帧：校验PID奇偶位，检查帧ID是否已被其他消息占用。
 *         执行Supplier ID和Function ID匹配校验，匹配成功后将帧ID写入lin_configuration_RAM。
 *         - 当分配的ID为0x00时，额外设置lin_pFrameBuf[8~9]标志位以备后续处理。
 * @retval None (通过 lin_diag_positive_notify 返回响应)
 */
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
void lin_diag_assign_frame_identifier(uint8_t *ptr, uint16_t length)
{
    (void)length;
    uint8_t id, pid, i;                 /*!< id=校验后的帧ID, pid=原始PID, i=循环索引 */
    uint16_t supid, messageid;          /*!< supid=供应商ID, messageid=消息标识符 */

    /* 从请求报文中提取Supplier ID和Message ID */
    supid = ((uint16_t)ptr[2] << 8) | ptr[1];
    messageid = ((uint16_t)ptr[4] << 8) | ptr[3];

    pid = ptr[5];

    /* PID=0x40表示删除帧操作 */
    if (pid == 0x40u)    //Delete frameid
    {
        i = 1u;
        /* 遍历配置ROM，查找匹配的Message ID */
        while (lin_configuration_ROM[i] != 0xFFFFu)
        {
            /* 找到匹配的Message ID后，将RAM中对应帧ID置0xFF（删除） */
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

    /* 非删除操作：校验PID的奇偶位，获取有效帧ID */
    id = lin_process_parity(pid, CHECK_PARITY);

    /* 检查该帧ID是否已被其他消息占用 */
    i = 1;
    while (lin_configuration_ROM[i] != 0xFFFFu)
    {
        /* 若帧ID已分配给其他Message ID（非当前消息），则将ID置0xFF表示冲突 */
        if ((id == lin_configuration_RAM[i]) && (lin_configuration_ROM[i] != messageid))
        {
            id = 0xFF;
            break;
        }

        i++;
    }

    /* 校验Supplier ID和Function ID是否匹配，或帧ID冲突 */
    if (((supid != product_id.supplier_id) && (supid != (uint16_t)LD_ANY_SUPPLIER)) || (id == 0xFFu))
    {
        /* Supplier ID不匹配或帧ID冲突：重置从节点响应计数，静默退出 */
        tl_slaveresp_cnt = 0;
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
        tl_service_status = LD_SERVICE_IDLE;
#endif /* End (_TL_FRAME_SUPPORT_ = = _TL_MULTI_FRAME_) */
        return;
    }

    /* 重新遍历配置ROM，查找匹配的Message ID以分配帧ID */
    i = 1;
    while (lin_configuration_ROM[i] != 0xFFFFu)
    {
        /* 找到匹配的Message ID，将校验后的帧ID写入RAM配置 */
        if (lin_configuration_ROM[i] == messageid)
        {
            lin_configuration_RAM[i] = id;
            /* Send positive response */
            lin_diag_positive_notify(ptr[0], NULL, 0);
            /* 当分配的帧ID为0x00时，设置帧缓冲标志供后续逻辑使用 */
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
