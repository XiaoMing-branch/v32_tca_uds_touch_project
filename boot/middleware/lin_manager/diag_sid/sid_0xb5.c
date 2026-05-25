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

#define LOG_DIAG(...)  //do{log_debug("[DIAG_0xB5] " __VA_ARGS__);}while(0)

#if LIN_PROTOCOL == PROTOCOL_J2602
/**
 * @brief  SID $B5 SNPD - J2602 Target Reset目标节点复位
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   遍历所有包含错误信号(Error Signal)的帧，将错误标志位置为0x01(表示"Reset")
 *         非广播NAD: 发送正响应
 *         广播NAD: 静默处理(不发送响应)
 * @retval None
 */
void lin_diag_target_reset(uint8_t *ptr, uint16_t length)
{
    //uint8_t *signal_data_ptr;
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
 * @brief  SNPD诊断处理弱函数(可被用户重写)
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   weak属性允许用户在APP层覆盖此函数实现自定义SNPD逻辑
 *         默认实现为空函数体
 * @retval None
 */
__attribute__((weak))  void lin_snpd_diag_handle(uint8_t *ptr, uint16_t length)
{

}

/**
 * @brief  SID $B5 SNPD诊断入口(非J2602协议)
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   直接转发到lin_snpd_diag_handle弱函数
 *         SNPD用于供应商自定义诊断服务
 * @retval None
 */
void lin_diag_snpd(uint8_t *ptr, uint16_t length)
{
    lin_snpd_diag_handle(ptr, length);
}
#endif /* End (LIN_PROTOCOL == PROTOCOL_J2602) */
