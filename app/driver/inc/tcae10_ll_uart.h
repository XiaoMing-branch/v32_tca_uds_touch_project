/**
  ******************************************************************************
  * @brief  uart header file.
  *
  * @file   tcae10_ll_uart.h
  * @author AE/FAE team
  * @date
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
  *
  ******************************************************************************
  */


#ifndef __TCAE10_LL_UART_H__
#define __TCAE10_LL_UART_H__


#include <stdint.h>
#include "tcae10.h"


/** @addtogroup TCAE01_DRIVER
  * @{
  */



/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    UART_PRINT_GPIO2 = 0,
    UART_PRINT_GPIO3,
    UART_PRINT_GPIO5,
    UART_PRINT_GPIO6
} UART_PrintPin_t;

/**
 * @brief  去初始化UART打印模块
 */
void ll_uart_deinit(void);
/**
 * @brief  初始化UART打印模块
 * @param baud - 波特率
 */
void ll_uart_init(uint32_t baud);
/**
 * @brief  UART发送单字节
 * @param data - 要发送的字节数据
 */
void ll_uart_sendbyte(uint8_t data);
/**
 * @brief  UART发送多字节数据
 * @param data - 数据缓冲区
 * @param length - 发送长度（字节）
 */
void ll_uart_senddata(uint8_t *data, uint16_t length);
/**
 * @brief  设置UART打印输出引脚
 * @param pin - 打印引脚选择 @ref UART_PrintPin_t
 */
void ll_uart_set_printpin(UART_PrintPin_t pin);

#ifdef __cplusplus
}
#endif
/**
  * @}
  */
#endif /* __TCAE10_LL_UART_H__ */


