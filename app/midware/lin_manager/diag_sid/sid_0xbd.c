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
 * @brief  0xBD SID - 写入SoC寄存器值
 *         从请求报文中提取寄存器地址和待写入值，通过pal_store_reg_rw()写入寄存器
 * @param  ptr - UDS请求报文指针（ptr[2..5]为大端序地址, ptr[6..9]为大端序值）
 * @param  length - 报文长度（本函数未使用）
 * @note   地址和值均需经endian_swap_func()转换为小端序后传入pal_store_reg_rw()
 *         写操作完成后将地址和值原样返回
 * @retval None
 */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void soc_reg_write(uint8_t *ptr, uint16_t length)
{
    (void)length;
    l_u32 addr = 0;
    (void)(&addr);
    l_u32 tt = 0;
    (void)(&tt);

    addr = endian_swap_func((uint8_t *)&ptr[2], sizeof(uint32_t));
    tt   = endian_swap_func((uint8_t *)&ptr[6], sizeof(uint32_t));

/* PRQA S 0310 1 #0310 - Cast between different object pointer types is intentional for hardware register access, types have compatible size and alignment. */
    pal_store_reg_rw(true, addr, (uint32_t *)&tt);

    lin_diag_positive_notify(ptr[0], &ptr[2], 8);
}
