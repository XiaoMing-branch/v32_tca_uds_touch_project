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
#include "store_manager.h"

/**
 * @brief  SID $B6 SaveConfiguration保存配置到非易失存储
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   保存当前lin_configured_NAD到系统配置存储(SYSTEM_CFG_PARAM)
 *         保存帧ID配置数组到SYSTEM_ID_CFG_PARAM
 *         完成后回复正响应
 * @retval None
 */
void lin_diag_save_configuration(uint8_t *ptr, uint16_t length)
{
    /* save nad */
    g_sys_cfgs.nad = lin_configured_NAD;
    store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);

    store_system_data_set(SYSTEM_ID_CFG_PARAM, (uint8_t *)g_sys_cfgs.frame_id_cfg, LIN_SIZE_OF_CFG);

    lin_diag_positive_notify(ptr[0], NULL, 0);
}
