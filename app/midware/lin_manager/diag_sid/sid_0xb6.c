/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $B6 保存配置处理模块（SaveConfiguration）
 *
 * @file    sid_0xb6.c
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
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "store_manager.h"
#endif

/**
 * @brief  SID $B6 保存配置（SaveConfiguration）处理函数
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   将当前运行时配置持久化存储到Flash中：
 *         - 保存lin_configured_NAD到系统配置参数区（SYSTEM_CFG_PARAM）
 *         - 保存lin_configuration_RAM（帧ID配置表）到系统ID配置参数区（SYSTEM_ID_CFG_PARAM）
 *         保存完成后发送正响应。
 * @retval None (通过 lin_diag_positive_notify 返回)
 */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diag_save_configuration(uint8_t *ptr, uint16_t length)
{
    (void)length;
    /* save nad */
    g_sys_cfgs.nad = lin_configured_NAD;
    store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);

    store_system_data_set(SYSTEM_ID_CFG_PARAM, lin_configuration_RAM, LIN_SIZE_OF_CFG);

    lin_diag_positive_notify(ptr[0], NULL, 0);
}
