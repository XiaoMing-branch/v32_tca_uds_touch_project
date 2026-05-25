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
#include "utilities.h"

/**
 * @brief  SID $2F IOControlByIdentifier处理(Boot版本)
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   Bootloader侧仅返回正响应，不执行实际IO控制
 *         APP侧实现具体的IO控制逻辑(如LED点亮/熄灭)
 * @retval None
 */
void lin_diag_io_control_by_identifier(uint8_t *ptr, uint16_t length)
{
    lin_diag_positive_notify(ptr[0], NULL, 0);
}
