/**
 *****************************************************************************
 * @brief   SID $AD/$AE/$AF LIN一致性测试命令处理源文件
 *
 * @file    sid_0xct.c
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
#include "fff_utilities.h"
#include "fff_pal_store.h"
#else
#include "diagnosticIII.h"
#include "utilities.h"
#include "pal_store.h"
#endif

/**
 * @brief  0xAD命令 - LIN一致性测试响应（返回固定测试数据）
 *         按照LIN一致性测试规范，返回预定义的8字节测试数据序列
 * @param  ptr - UDS请求/响应报文指针
 * @param  length - 报文长度
 * @note   固定响应: 0x0B, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
 *         MasterReq 01 10 0B AD 21 22 23 24 (standard check AB)
 *         MasterReq 01 21 25 26 27 28 29 2A (standard check EF)
 *         SlaveResp 01 10 08 ED 0B 32 33 34 (standard check 54)
 *         SlaveResp 01 21 35 36 37 FF FF FF (standard check 3B)
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
 * @brief  0xAE命令 - LIN一致性测试响应（返回固定标识字节）
 *         按照LIN一致性测试规范，返回0x0B作为响应标识
 * @param  ptr - UDS请求/响应报文指针
 * @param  length - 报文长度
 * @note   固定响应: 0x0B
 *         MasterReq 01 10 0B AE 11 12 13 14 (standard check EA)
 *         MasterReq 01 21 15 16 17 18 19 1A (standard check 50)
 *         SlaveResp 01 02 EE 0B FF FF FF FF (standard check 03)
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
 * @brief  0xAF命令 - LIN一致性测试响应（返回长度标识字节）
 *         按照LIN一致性测试规范，返回0x06作为响应长度标识
 * @param  ptr - UDS请求/响应报文指针
 * @param  length - 报文长度
 * @note   固定响应: 0x06
 *         MasterReq 01 06 AF 01 02 03 04 05 (standard check 3A)
 *         SlaveResp 01 02 EF 06 FF FF FF FF (standard check 07)
 * @retval None
 */
#ifdef CFG_LIN_CONFORM_TEST
void diag_0xaf_command(uint8_t *ptr, uint16_t length)
{
    ptr[1] = 06;

    lin_diag_positive_notify(ptr[0], &ptr[2], 2);
}
#endif
