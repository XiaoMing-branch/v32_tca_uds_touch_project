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
#include "fff_store_manager.h"
#include "fff_pal_store.h"
#include "fff_lin_precfg.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "store_manager.h"
#include "pal_store.h"
#include "lin_precfg.h"
#endif

static uint8_t fast_nad_write(uint8_t nad);

#if LIN_PROTOCOL == PROTOCOL_J2602
/**
 * @brief  J2602消息ID重新计算函数（根据新的DNN调整帧ID）
 * @param  dnn - 新的设备节点号（Device Node Number）
 * @param  frame_id_change - 帧ID变化步长（4/8/16取决于可配置帧数量）
 * @note   遍历所有LIN帧配置，将非广播帧ID重算：(原始ID % frame_id_change) + (dnn << 2)。
 *         广播帧（0x38~0x3B）根据DNN是否>=8进行±1调整。
 * @retval 0 - 成功
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
            if ((dnn >= 8U) &&
                ((lin_configuration_RAM[i] == 0x38U) || (lin_configuration_RAM[i] == 0x3AU)))
            {
                lin_configuration_RAM[i] += 1U;
            }
            else if ((dnn < 8U) &&
                     ((lin_configuration_RAM[i] == 0x39U) || (lin_configuration_RAM[i] == 0x3BU)))
            {
                lin_configuration_RAM[i] -= 1U;
            }
        }
    }

    return (l_bool)0U;
}

/**
 * @brief  J2602 NAD重配置时重新计算所有消息ID
 * @param  dnn - 设备节点号（NAD & 0x0F，范围0~13）
 * @note   根据可配置帧数量选择不同的步长：
 *         - >16帧：仅0x60 NAD有效，不做任何操作
 *         - 9~16帧：步长16（仅DNN=0/4/8有效）
 *         - 5~8帧：步长8（DNN必须为偶数）
 *         - 1~4帧：步长4（所有DNN有效）
 *         调用ld_change_msg_id()进行实际ID重算。
 * @retval 0 - 成功; 1 - 失败/DNN无效
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
 * @brief  SID $B0 分配节点地址（AssignNAD）处理函数
 * @param  NAD - 当前节点地址
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   仅当NAD匹配（初始NAD/广播/已配置NAD）且报文长度为6时执行。
 *         验证Supplier ID和Function ID是否匹配后，设置新的lin_configured_NAD。
 *         J2602协议下还需调用ld_reconfig_msg_ID()重新分配消息ID。
 *         新NAD通过fast_nad_write()写入Flash持久化存储。
 * @retval None (通过 lin_diag_positive_notify / lin_diag_negative_notify 返回)
 */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diagservice_assign_nad(uint8_t NAD, uint8_t *ptr, uint16_t length)
{
    uint16_t supplierIdLsb;
    uint16_t supplierIdMsb;
    uint16_t functionIdLsb;
    uint16_t functionIdMsb;

    if (((NAD == lin_initial_NAD) || (NAD == (uint8_t)LD_BROADCAST) || (NAD == lin_configured_NAD)) && (6U == length))
    {
        /* Get Supplier ID and Function ID*/
        supplierIdLsb = ptr[1];
        supplierIdMsb = ptr[2];
        functionIdLsb = ptr[3];
        functionIdMsb = ptr[4];

        /*Check if Supplier ID and Function ID match, then send positive response */
        if ((((supplierIdMsb << 8) | supplierIdLsb) == product_id.supplier_id) ||
            (((supplierIdMsb << 8) | supplierIdLsb) == (uint16_t)LD_ANY_SUPPLIER))
        {
            if ((((functionIdMsb << 8) | functionIdLsb) == product_id.function_id) ||
                (((functionIdMsb << 8) | functionIdLsb) == (uint16_t)LD_ANY_FUNCTION))
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
                    lin_diag_negative_notify(ptr[0], ROOR);
                }

#endif
                lin_configured_NAD = ptr[5];
                g_sys_cfgs.nad = lin_configured_NAD;

                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                fast_nad_write(lin_configured_NAD);

                //                store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
            }
        }
    }
}

/**
 * @brief  快速将NAD写入Flash存储（空扇区扫描方式）
 * @param  nad - 要写入的节点地址值
 * @note   以FLASH_SECTOR_SIZE为范围扫描每个对齐字，查找值为0xFF的空闲字节。
 *         若找到的空闲位置的前一个字节已经是相同NAD值，则跳过写入（去重）。
 *         否则将NAD写入该空闲位置。
 *         若整个扇区已满，返回0表示写入失败。
 * @retval 1 - 写入/跳过成功; 0 - 写入失败（扇区已满）
 */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
static uint8_t fast_nad_write(uint8_t nad)
{
    uint32_t alignbuf[2];
    uint8_t *rdbuf = (uint8_t *)alignbuf;

    for (uint32_t i = 0; i < (uint32_t)FLASH_SECTOR_SIZE; i += sizeof(alignbuf))
    {
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        pal_store_read(STORE_TYPE_SEL, FAST_LIN_NAD_ADDR + i, rdbuf, sizeof(alignbuf));
        for (uint32_t j = 0; j < sizeof(alignbuf); ++j)
        {
            if (rdbuf[j] == 0xFFu)
            {
                if ((j > 0u) && (rdbuf[j - 1u] == nad)) // No need to save duplicate nad
                {
                    return 1;
                }
                rdbuf[j] = nad;
                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                pal_store_write(STORE_TYPE_SEL, FAST_LIN_NAD_ADDR + i, rdbuf, sizeof(alignbuf));
                return 1;
            }
        }
    }

    return 0;
}
