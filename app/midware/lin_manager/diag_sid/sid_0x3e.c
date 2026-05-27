/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $3E 诊断仪在线处理模块（TesterPresent）
 *
 * 本模块实现了UDS协议SID $3E（TesterPresent）服务的处理逻辑。
 * 该服务用于诊断仪周期性发送在线请求以保持诊断会话激活状态，
 * 防止ECU因会话超时而自动退出诊断模式。支持两种子功能：
 * 子功能0x00（需要正响应）和子功能0x80（抑制正响应）。
 * 对于不支持的其他子功能值，返回SFNS（子功能不支持）负响应。
 *
 * @file    sid_0x3e.c
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
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "utilities.h"
#endif

/**
 * @brief  SID $3E 诊断仪在线处理函数（TesterPresent）
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   子功能0x00 = 需要响应，返回正响应。
 *         子功能0x80 = 抑制正响应（suppressPosRspMsgIndicationBit=1），仅刷新会话定时器。
 *         其他子功能返回SFNS负响应。用于保持诊断会话激活状态。
 * @retval None (通过 lin_diag_positive_notify / lin_diag_negative_notify 返回)
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_tester_present(uint8_t *ptr, uint16_t length)
{
    (void)length;
    switch (ptr[1]) /*!< 根据请求报文首个数据字节（子功能字节）选择对应的诊断仪在线处理方式 */
    {
        case 0x00u : /*!< 子功能0x00：需要正响应模式（supportPosRspMsgIndicationBit=0），发送正响应确认诊断仪在线 */ //supportPosRspMsgIndicationBit=0
            lin_diag_positive_notify(ptr[0], &ptr[1], 1); /*!< 发送正响应报文（SID + 子功能），保持诊断会话激活状态 */
            break;

        case 0x80u : /*!< 子功能0x80：抑制正响应模式（supportPosRspMsgIndicationBit=1），仅刷新会话定时器，不发送响应报文 */ //supportPosRspMsgIndicationBit=1
            break; /*!< 不发送任何响应，仅通过接收此请求维持诊断会话不超时 */

        default : /*!< 未定义或不支持的子功能值，统一返回SFNS负响应 */
            lin_diag_negative_notify(ptr[0], SFNS); /*!< 发送SFNS（子功能不支持）负响应码 */ //sub-functionNotSupported
            break;
    }
}

