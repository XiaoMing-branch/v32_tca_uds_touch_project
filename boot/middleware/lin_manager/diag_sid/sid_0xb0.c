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

#if LIN_PROTOCOL == PROTOCOL_J2602
/**
 * @brief  J2602协议下根据DNN重新计算报文ID
 * @param  dnn - 新的节点NAD值(DNN)
 * @param  frame_id_change - 帧ID变化步长(4/8/16取决于可配置帧数量)
 * @note   遍历所有帧配置，对非广播帧(<=0x37)重新计算PID = id_origin + dnn<<2
 *         对广播帧(0x38~0x3B)根据DNN值调整偏移
 * @retval 0 - 成功; 1 - 失败
 */
static l_bool ld_change_msg_id(uint8_t dnn, uint8_t frame_id_change)
{
    uint8_t i;
    uint8_t id_origin;

    /* If new DNN is greater than current DNN */
    for (i = lin_num_of_frms; i > 0U; i--)
    {
        /* For non-broadcast frame identifiers  less than or equal to 0x37 */
        if (lin_configuration_RAM[i] <= 0x37U)
        {
            /* get id with dnn equal 0 */
            id_origin = (uint8_t)(lin_configuration_RAM[i] % frame_id_change);
            lin_configuration_RAM[i] = (uint8_t)(id_origin + (uint8_t)(dnn << 2));
        }
        /* For broad cast message ID */
        else if ((lin_configuration_RAM[i] <= 0x3BU) && (lin_configuration_RAM[i] >= 0x38U))
        {
            if ((dnn  >= 8U) &&
                ((lin_configuration_RAM[i] == 0x38U) || (lin_configuration_RAM[i] == 0x3AU)))
            {
                lin_configuration_RAM[i] += 1U;
            }
            else if ((dnn  < 8U) &&
                     ((lin_configuration_RAM[i] == 0x39U) || (lin_configuration_RAM[i] == 0x3BU)))
            {
                lin_configuration_RAM[i] -= 1U;
            }
        }
    }

    return (l_bool)0U;
}

/**
 * @brief  J2602协议下根据DNN重新配置报文ID(含边界检查)
 * @param  dnn - 新的节点NAD值(DNN = ptr[5] - 0x60)
 * @note   根据可配置帧数量分档处理:
 *         >16帧: 仅支持NAD=0x60(不做任何变更)
 *         9~16帧: 仅支持DNN=0/4/8, 步长16
 *         5~8帧: 仅支持DNN为偶数, 步长8
 *         1~4帧: 步长4, 支持0~13
 *         先计算非广播帧数量，排除LIN_FRM_UNCD类型帧
 * @retval 0 - 成功; 1 - 失败
 */
static l_bool ld_reconfig_msg_ID(uint8_t dnn)
{
    l_bool ret_val = 1U;
    int8_t i = 0;
    /* Get number of configurable_frames not calculate id 0x3C and 0x3D */
    uint8_t number_of_configurable_frames = lin_num_of_frms - 2;

    for (i = lin_num_of_frms - 3; i >= 0; i--)
    {
        if (lin_frame_tbl[i].frm_type == LIN_FRM_UNCD)
        {
            break;
        }
        else
        {
            number_of_configurable_frames--;
        }
    }

    if (dnn <= 0xDU)
    {
        /* number of configurable frames greater than 16 */
        if (number_of_configurable_frames > 16U)
        {
            /* Only 0x60 is valid NAD */
            /* Do nothing */
        }
        /* number of configurable frames is from 9 - 16 */
        else if (number_of_configurable_frames > 8U)
        {
            /* Only NAD 0x60, 0x64, 0x68 are valid, 0x6C and 0x6D not valid */
            if ((dnn == 0U) || (dnn == 4U) || (dnn == 8U))
            {
                ret_val = ld_change_msg_id(dnn, 16U);
            }
        }
        /* number of configurable frames is from 5 - 8 */
        else if (number_of_configurable_frames > 4U)
        {
            /* Check to verify if dnn is 0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C */
            if ((dnn % 2U) == 0U)
            {
                ret_val = ld_change_msg_id(dnn, 8U);
            }
        }
        /* number of configurable frames is from 1 - 4 */
        else
        {
            ret_val = ld_change_msg_id(dnn, 4U);
        }
    }

    return ret_val;
}
#endif

/**
 * @brief  SID $B0 AssignNAD分配节点地址
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   请求格式: ptr[1-2]=SupplierID, ptr[3-4]=FunctionID, ptr[5]=新NAD
 *         验证SupplierID和FunctionID与本机匹配(或通配符ANY)
 *         J2602模式下: 新NAD必须在0x60~0x6D范围, 并调用ld_reconfig_msg_ID重算帧ID
 *         非J2602模式: 直接回复正响应
 *         NAD实际在验证通过后更新: lin_configured_NAD = ptr[5]
 * @retval None
 */
void lin_diag_assign_nad(uint8_t *ptr, uint16_t length)
{
    uint16_t supplierIdLsb;
    uint16_t supplierIdMsb;
    uint16_t functionIdLsb;
    uint16_t functionIdMsb;

    if (6U == length)
    {
        /* Get Supplier ID and Function ID*/
        supplierIdLsb = ptr[1];
        supplierIdMsb = ptr[2];
        functionIdLsb = ptr[3];
        functionIdMsb = ptr[4];

        /*Check if Supplier ID and Function ID match, then send positive response */
        if (((supplierIdMsb << 8 | supplierIdLsb) == product_id.supplier_id) ||
            ((supplierIdMsb << 8 | supplierIdLsb) == LD_ANY_SUPPLIER))
        {
            if (((functionIdMsb << 8 | functionIdLsb) == product_id.function_id) ||
                ((functionIdMsb << 8 | functionIdLsb) == LD_ANY_FUNCTION))
            {
                lin_configured_NAD = lin_initial_NAD; /* use lin_initial_NAD for response*/
#if LIN_PROTOCOL != PROTOCOL_J2602
                lin_diag_positive_notify(ptr[0], NULL, 0);
#else

                if ((ptr[5] >= 0x60) && (ptr[5] <= 0x6D) && (0 == ld_reconfig_msg_ID(ptr[5] - 0x60)))
                {
                    lin_diag_positive_notify(ptr[0], NULL, 0);
                }
                else
                {
                    lin_diag_negative_notify(ptr[0], ROOR)
                }

#endif
                lin_configured_NAD = ptr[5];
            }
        }
    }
}
