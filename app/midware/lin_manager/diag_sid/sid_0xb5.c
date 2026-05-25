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
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#endif

/* PRQA S 1534 1 #3261 - Unused macro defined for future extension and configuration compatibility */
#define LOG_DIAG(...)  //do{log_debug("[DIAG_0xB5] " __VA_ARGS__);}while(0)

#if LIN_PROTOCOL == PROTOCOL_J2602
/**
 * @brief  SID $B5 目标复位处理函数（J2602协议）
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   遍历所有包含错误信号的响应帧，将lin_pFrameBuf中对应的error signal位置为0x01（表示"Reset"），
 *         通过lin_response_error_byte_offset[]/bit_offset[]定位错误信号位。
 *         若非广播NAD则发送正响应，广播NAD不响应。
 * @retval None (通过 lin_diag_positive_notify 返回)
 */
void lin_diag_target_reset(uint8_t *ptr, uint16_t length)
{
    uint8_t nad;
    uint16_t byte_offset_temp;
    uint8_t bit_offset_temp, i;

    /* Set the reset flag within the J2602 Status Byte */
    /* Set error signal equal to error in response */
    for (i = 0; i < num_frame_have_esignal; i++)
    {
        /* Get pointer to Byte and bit offset values in each frame that contains the error signal */
        byte_offset_temp = lin_response_error_byte_offset[i];
        bit_offset_temp = lin_response_error_bit_offset[i];
        /* Set error signal to 0x01 means "Reset" */
        lin_pFrameBuf[byte_offset_temp] = (uint8_t)((lin_pFrameBuf[byte_offset_temp] & (~(0x07U << bit_offset_temp))) |
                                          (0x01U << bit_offset_temp));
    }

    /* Create positive response */

    /* Get NAD of node */
    nad = lin_lld_response_buffer[1];

    if (LD_BROADCAST != nad)
    {
        lin_diag_positive_notify(ptr[0], NULL, 0);
    }
    else
    {
        tl_slaveresp_cnt = 0;
    }
}
#else
/**
 * @brief  SNP（从机节点定位）诊断处理弱函数
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   __attribute__((weak))弱函数，默认空实现。
 *         用户可在其他文件中重定义此函数以实现SNPD协议特定的从机节点定位处理逻辑。
 * @retval None
 */
/* PRQA S 3673 4 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1505 3 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 2071 1 #3269 - Language extension used for compiler and hardware optimization */
__attribute__((weak))  void lin_snpd_diag_handle(uint8_t *ptr, uint16_t length)
{
    (void)ptr;
    (void)length;
}

/**
 * @brief  SID $B5 SNPD从机节点位置诊断处理函数（非J2602协议）
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   将请求委托给lin_snpd_diag_handle()弱函数处理。
 *         用户可在应用层重定义lin_snpd_diag_handle实现自定义SNPD逻辑。
 * @retval None
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_snpd(uint8_t *ptr, uint16_t length)
{
    lin_snpd_diag_handle(ptr, length);
}
#endif /* End (LIN_PROTOCOL = = PROTOCOL_J2602) */
