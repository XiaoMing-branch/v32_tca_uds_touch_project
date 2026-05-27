/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   LIN诊断配置服务 - SID $B5 目标复位（TargetReset）/ SNPD从机节点定位处理源文件
 *
 * @file    sid_0xb5.c
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
#define LOG_DIAG(...)  //do{log_debug("[DIAG_0xB5] " __VA_ARGS__);}while(0) /*!< 诊断日志宏，默认被注释，启用时可输出SID $B5调试信息 */

#if LIN_PROTOCOL == PROTOCOL_J2602
/**
 * @brief  SID $B5 目标复位处理函数（J2602协议）
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   J2602协议下的目标复位操作。遍历所有包含错误信号的响应帧，
 *         将lin_pFrameBuf中对应的error signal位置为0x01（表示"Reset"）。
 *         错误信号位通过lin_response_error_byte_offset[]和
 *         lin_response_error_bit_offset[]数组定位。
 *         完成错误标志设置后，检查NAD是否为广播地址：
 *         - 非广播NAD：发送正响应
 *         - 广播NAD：不响应（重置从节点响应计数）
 * @retval None (通过 lin_diag_positive_notify 返回正响应，广播NAD时静默退出)
 */
void lin_diag_target_reset(uint8_t *ptr, uint16_t length)
{
    uint8_t nad;                      /*!< 从节点地址，从响应缓冲区获取 */
    uint16_t byte_offset_temp;        /*!< 错误信号字节偏移临时变量 */
    uint8_t bit_offset_temp, i;       /*!< 错误信号位偏移 + 循环索引 */

    /* 遍历所有包含错误信号的响应帧，将错误信号设为Reset状态 */
    for (i = 0; i < num_frame_have_esignal; i++)
    {
        /* 获取当前帧中错误信号的字节偏移和位偏移 */
        byte_offset_temp = lin_response_error_byte_offset[i];
        bit_offset_temp = lin_response_error_bit_offset[i];
        /* 将error signal位置为0x01（表示"Reset"），保留其他位不变 */
        lin_pFrameBuf[byte_offset_temp] = (uint8_t)((lin_pFrameBuf[byte_offset_temp] & (~(0x07U << bit_offset_temp))) |
                                          (0x01U << bit_offset_temp));
    }

    /* 获取当前节点的NAD，判断是否需要发送响应 */
    nad = lin_lld_response_buffer[1];

    /* 非广播NAD才发送正响应，广播NAD不响应以避免总线冲突 */
    if (LD_BROADCAST != nad)
    {
        lin_diag_positive_notify(ptr[0], NULL, 0);
    }
    else
    {
        /* 广播NAD：重置从节点响应计数，不发送任何响应 */
        tl_slaveresp_cnt = 0;
    }
}
#else
/**
 * @brief  SNPD从机节点定位诊断处理弱函数（默认空实现）
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   使用__attribute__((weak))声明的弱函数，默认空实现。
 *         SNPD（Slave Node Position Detection）用于从机节点自动寻址定位。
 *         用户可在应用层重新定义此函数以实现具体的SNPD寻址逻辑，
 *         例如根据物理位置自动分配NAD。
 *         非J2602协议下使用（J2602协议使用lin_diag_target_reset）。
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
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   将SID $B5请求委托给lin_snpd_diag_handle()弱函数处理。
 *         SNPD（Slave Node Position Detection）是LIN自定位协议，
 *         允许从机根据其在总线上的物理位置自动获取NAD。
 *         用户可在应用层重定义lin_snpd_diag_handle实现自定义SNPD逻辑，
 *         例如基于GPIO电平或时序检测判断节点位置。
 * @retval None
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_diag_snpd(uint8_t *ptr, uint16_t length)
{
    lin_snpd_diag_handle(ptr, length);
}
#endif /* End (LIN_PROTOCOL = = PROTOCOL_J2602) */
