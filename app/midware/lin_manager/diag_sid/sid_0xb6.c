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
 * @param  ptr    - UDS请求报文指针，ptr[0]为SID，本函数未使用其他数据字节
 * @param  length - 报文长度（本函数未使用，通过(void)length抑制编译器警告）
 * @note   将当前运行时配置持久化存储到Flash中：
 *         - 步骤1：将当前运行的 lin_configured_NAD 写入全局系统配置结构体 g_sys_cfgs.nad
 *         - 步骤2：调用 store_system_data_set() 将 g_sys_cfgs 写入 Flash 的 SYSTEM_CFG_PARAM 区域
 *         - 步骤3：调用 store_system_data_set() 将 lin_configuration_RAM（帧ID配置表）写入
 *           Flash 的 SYSTEM_ID_CFG_PARAM 区域
 *         - 步骤4：发送 $B6 正响应以通知上位机保存操作已完成
 * @attention 本函数仅存储配置参数，不涉及Flash擦除操作，由 store_system_data_set() 内部管理
 * @retval None（正/负响应通过 lin_diag_positive_notify 或 lin_diag_negative_notify 发送）
 */
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_diag_save_configuration(uint8_t *ptr, uint16_t length)
{
    (void)length;                                                                       /*!< 抑制未使用形参的编译器警告 */
    /* save nad */
    g_sys_cfgs.nad = lin_configured_NAD;                                                /*!< 将当前运行的NAD值写入系统配置结构体 */
    store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);   /*!< 将系统配置参数（含NAD）持久化存储到Flash的SYSTEM_CFG_PARAM区域 */

    store_system_data_set(SYSTEM_ID_CFG_PARAM, lin_configuration_RAM, LIN_SIZE_OF_CFG); /*!< 将LIN帧ID配置表持久化存储到Flash的SYSTEM_ID_CFG_PARAM区域 */

    lin_diag_positive_notify(ptr[0], NULL, 0);                                           /*!< 发送SID $B6正响应，通知上位机配置保存已完成 */
}
