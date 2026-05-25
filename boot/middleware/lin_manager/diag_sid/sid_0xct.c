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
#include "pal_store.h"

/**
 * @brief  LIN一致性测试服务 $AD命令
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   LIN一致性测试用 - 返回固定数据模式
 *         MasterReq: 01 10 0B AD 21 22 23 24 / 01 21 25 26 27 28 29 2A
 *         SlaveResp: 01 10 08 ED 0B 32 33 34 / 01 21 35 36 37 FF FF FF
 * @retval None
 */
#ifdef CFG_LIN_CONFORM_TEST
void diag_0xad_command(uint8_t *ptr, uint16_t length)
{
    ptr[1] = 0x0B;
    ptr[2] = 0x32;
    ptr[3] = 0x33;
    ptr[4] = 0x34;
    ptr[5] = 0x35;
    ptr[6] = 0x36;
    ptr[7] = 0x37;

    lin_diag_positive_notify(ptr[0], &ptr[1], 9);
}
#endif

/**
 * @brief  LIN一致性测试服务 $AE命令
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   LIN一致性测试用 - 返回固定数据模式
 *         MasterReq: 01 10 0B AE 11 12 13 14 / 01 21 15 16 17 18 19 1A
 *         SlaveResp: 01 02 EE 0B FF FF FF FF
 * @retval None
 */
#ifdef CFG_LIN_CONFORM_TEST
void diag_0xae_command(uint8_t *ptr, uint16_t length)
{
    ptr[1] = 0x0B;

    lin_diag_positive_notify(ptr[0], &ptr[2], 2);
}
#endif

/**
 * @brief  LIN一致性测试服务 $AF命令
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   LIN一致性测试用 - 返回固定数据模式
 *         MasterReq: 01 06 AF 01 02 03 04 05
 *         SlaveResp: 01 02 EF 06 FF FF FF FF
 * @retval None
 */
#ifdef CFG_LIN_CONFORM_TEST
void diag_0xaf_command(uint8_t *ptr, uint16_t length)
{
    ptr[1] = 06;

    lin_diag_positive_notify(ptr[0], &ptr[2], 2);
}
#endif
