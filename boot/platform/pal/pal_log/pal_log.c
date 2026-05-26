/**
 *****************************************************************************
 * @brief   pal log source file.
 *
 * @file    pal_log.c
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

#include "pal_log.h"

/**
 * @brief  日志打印一个字符(通过串口发送)
 * @param  ch - 待打印的 ASCII 字符
 * @retval 无返回值
 */
void pal_log_print(uint8_t ch)
{
    ll_sci_transmit(LL_SCI_BUS_0, (uint8_t *)&ch, 1);
}

/**
 * @brief  日志模块初始化(配置并初始化串口)
 * @param  baudrate - 串口通信波特率，如 115200
 * @retval 无返回值
 */
void pal_log_init(uint32_t baudrate)
{
    sci_config_t config =
    {
        .baudrate = baudrate,
        .mode = SCI_MODE_UART,
    };

    ll_sci_init(LL_SCI_BUS_0, &config, NULL);
}

/**
 * @brief  日志模块去初始化(释放串口资源)
 * @param  无
 * @retval 无返回值
 */
void pal_log_deinit(void)
{
    ll_sci_deinit(LL_SCI_BUS_0);
}
