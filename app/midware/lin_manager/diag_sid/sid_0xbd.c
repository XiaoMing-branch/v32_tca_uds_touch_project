/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $BD SoC寄存器写入处理模块
 *
 * @file    sid_0xbd.c
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
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "utilities.h"
#include "pal_store.h"
#endif

/**
 * @brief  SID $BD 写入SoC寄存器值
 *         从UDS请求报文中提取寄存器地址（ptr[2..5]）和待写入值（ptr[6..9]），
 *         两者均为大端序；分别经 endian_swap_func() 转换为小端序后，
 *         调用 pal_store_reg_rw() 写入SoC寄存器；
 *         写操作完成后将报文中的原始数据（大端序）通过 lin_diag_positive_notify() 回显
 * @param  ptr    - UDS请求报文指针
 *         - ptr[0]：SID $BD
 *         - ptr[2..5]：4字节寄存器地址（大端序，高字节在前）
 *         - ptr[6..9]：4字节待写入值（大端序，高字节在前）
 * @param  length - 报文长度（本函数未使用，通过(void)length抑制编译器警告）
 * @note   endian_swap_func() 大端小端转换说明：
 *         - SoC（Cortex-M 内核）采用小端序（Little-Endian），低地址存放低字节
 *         - UDS/LIN 总线传输遵循大端序（Big-Endian，即网络字节序），高字节在前
 *         - 地址转换：报文 ptr[2..5] = [0x12,0x34,0x56,0x78]（大端序表示地址0x12345678）
 *           → endian_swap_func() 反转后得到小端序 [0x78,0x56,0x34,0x12] 供SoC使用
 *         - 值的转换同理：报文 ptr[6..9] 的大端序值反转后写入寄存器
 *         - 正响应直接回显 ptr[2..9] 的原始大端序数据，无需再次转换
 * @retval None（通过 lin_diag_positive_notify 回显8字节数据：地址+值）
 */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void soc_reg_write(uint8_t *ptr, uint16_t length)
{
    (void)length;                                                                       /*!< 抑制未使用形参的编译器警告 */
    l_u32 addr = 0;                                                                     /*!< SoC寄存器地址（小端序），从报文中提取的大端序地址转换而来 */
    (void)(&addr);
    l_u32 tt = 0;                                                                       /*!< 待写入寄存器的值（小端序），从报文中提取的大端序值转换而来 */
    (void)(&tt);

    /* 将报文中的大端序地址（ptr[2..5]）转换为小端序后存入addr */
    addr = endian_swap_func((uint8_t *)&ptr[2], sizeof(uint32_t));
    /* 将报文中的大端序待写入值（ptr[6..9]）转换为小端序后存入tt */
    tt   = endian_swap_func((uint8_t *)&ptr[6], sizeof(uint32_t));

/* PRQA S 0310 1 #0310 - Cast between different object pointer types is intentional for hardware register access, types have compatible size and alignment. */
    /* 调用寄存器读写接口写入SoC寄存器（true表示写操作），以小端序地址和值写入 */
    pal_store_reg_rw(true, addr, (uint32_t *)&tt);

    /* 发送SID $BD正响应，回显报文中的原始大端序数据（8字节：地址+值），无需再次转换 */
    lin_diag_positive_notify(ptr[0], &ptr[2], 8);
}
