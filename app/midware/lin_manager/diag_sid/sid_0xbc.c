/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $BC SoC寄存器读取处理模块
 *
 * @file    sid_0xbc.c
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
 * @brief  SID $BC 读取SoC寄存器值
 *         从UDS请求报文中提取寄存器地址（大端序），经 endian_swap_func() 转换为小端序后
 *         调用 pal_store_reg_rw() 读取SoC寄存器的值；读取结果同样经 endian_swap_func()
 *         原地转换回大端序后通过 lin_diag_positive_notify() 返回给上位机
 * @param  ptr    - UDS请求报文指针
 *         - ptr[0]：SID $BC
 *         - ptr[2..5]：4字节寄存器地址（大端序，高字节在前）
 * @param  length - 报文长度（本函数未使用，通过(void)length抑制编译器警告）
 * @note   endian_swap_func() 大端小端转换说明：
 *         - SoC（Cortex-M 内核）采用小端序（Little-Endian），即低地址存放低字节
 *         - UDS/LIN 总线传输遵循大端序（Big-Endian，即网络字节序），高字节在前
 *         - endian_swap_func() 将内存中 uint32_t 的字节序反转：
 *           - 输入（大端序）：[0x12, 0x34, 0x56, 0x78] → 值为 0x12345678
 *           - 输出（小端序）：[0x78, 0x56, 0x34, 0x12] → SoC 内存中正确表示
 *         - 读取结果的转换同理：SoC 返回小端序值，需转为大端序后再发回上位机
 * @retval None（通过 lin_diag_positive_notify 返回4字节寄存器值）
 */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void soc_reg_read(uint8_t *ptr, uint16_t length)
{
    (void)length;                                                                       /*!< 抑制未使用形参的编译器警告 */
    uint32_t addr = 0;                                                                  /*!< SoC寄存器地址（小端序），从报文中提取的大端序地址转换而来 */
    (void)(&addr);
    uint32_t tt = 0;                                                                    /*!< 存放读取到的寄存器值，初始为0 */

    /* 将报文中的大端序地址（ptr[2..5]）转换为小端序后存入addr */
    addr = endian_swap_func((uint8_t *)&ptr[2], sizeof(uint32_t));

    /* 调用寄存器读写接口读取SoC寄存器（false表示读操作），结果存入tt */
    pal_store_reg_rw(false, addr, &tt);

/* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    /* 将读取到的小端序寄存器值原地转换为大端序，以便通过UDS/LIN总线发送给上位机 */
    endian_swap_func((uint8_t *)&tt, sizeof(uint32_t));
    /* 发送SID $BC正响应，返回4字节大端序的寄存器值 */
    lin_diag_positive_notify(ptr[0], (uint8_t *)&tt, sizeof(uint32_t));
}
