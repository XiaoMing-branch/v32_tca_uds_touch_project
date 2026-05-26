/**
 *****************************************************************************
 * @brief   SID $14 清除诊断信息处理模块（ClearDiagnosticInformation）
 *
 * @file    sid_0x14.c
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
#include "diagnosticIII.h"
#endif


/**
 * @brief  SID $14 清除诊断信息处理函数（存根实现）
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   当前为存根实现，直接返回正响应，未执行实际的DTC清除操作。
 * @retval None (通过 lin_diag_positive_notify 返回)
 */
void clear_dtc_info_handle(uint8_t *ptr, uint16_t length)
{
    lin_diag_positive_notify(ptr[0], NULL, 0);
}
