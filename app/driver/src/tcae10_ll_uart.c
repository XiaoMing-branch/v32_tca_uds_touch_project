/**
  ******************************************************************************
  * @brief   uart source file.
  *
  * @file   uart.c
  * @author
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



#include <stdio.h>
#include "tcae10_ll_uart.h"
#include "tcae10_ll_gpio.h"
#include "tcae10_ll_cortex.h"
#include "system_tcae10.h"

/**
 * @brief   反初始化调试UART（Print UART）
 * @param   None
 * @note    复位Print UART外设寄存器，兼容Keil和GCC编译器
 * @retval  None
 */
void ll_uart_deinit(void)
{
    CRG_CONFIG_UNLOCK();
    CRG->PRINT_UART_CLKRST_CTRL_F.RST_PRINT_UART = 1;  /* 复位Print UART */
#if defined (__ARMCC_VERSION)      /* Keil uVision */
    __NOP();
    __NOP();
    __NOP();
#elif defined (__GNUC__)    /* GNU GCC*/
    __NOP();
    __NOP();
    __NOP();
#endif
    CRG->PRINT_UART_CLKRST_CTRL_F.RST_PRINT_UART = 0;  /* 释放复位 */
}



/**
 * @brief   初始化调试UART（Print UART）
 * @param   baud - UART波特率
 * @note    使能Print UART时钟，根据系统时钟计算分频系数设置波特率
 * @retval  None
 */
void ll_uart_init(uint32_t baud)
{
    uint16_t prescale;

    CRG_CONFIG_UNLOCK();
    CRG->PRINT_UART_CLKRST_CTRL_F.PCLK_EN_PRINT_UART = 1;  /* 使能Print UART PCLK */
    CRG_CONFIG_LOCK();

    prescale = (SystemGetHClkFreq() / baud) - 1;            /* 计算波特率分频系数 */
    PRINT_UART->PRESCALE_F.PRESCALE = prescale ;            /* 设置分频寄存器 */
}

/**
 * @brief   通过UART发送一个字节（调试输出）
 * @param   data - 待发送的字节数据
 * @note    阻塞等待发送完成
 * @retval  None
 */
void ll_uart_sendbyte(uint8_t data)
{
    PRINT_UART->TX_DATA_F.TX_DATA = data;           /* 写入发送数据寄存器 */

    while (PRINT_UART->STATUS_F.TX_BUSY == 1);       /* 等待发送完成 */
}

/**
 * @brief   通过UART发送数据包（调试输出）
 * @param   data   - 指向待发送数据的指针
 * @param   length - 数据包长度
 * @note    循环调用ll_uart_sendbyte逐字节发送
 * @retval  None
 */
void ll_uart_senddata(uint8_t *data, uint16_t length)
{
    while (length-- > 0)
    {
        ll_uart_sendbyte(*data);
        data++;
    }
}

/**
 * @brief   设置调试UART的TX输出引脚
 * @param   pin - UART打印引脚选择，可取GPIO2/3/5/6
 * @note    选择GPIO6时需额外使能PWM和ADC相关外设
 * @retval  None
 */
void ll_uart_set_printpin(UART_PrintPin_t pin)
{
    switch (pin)
    {
    case UART_PRINT_GPIO2:
        ll_gpio_afio_config(GPIO_PIN_2, AFIO_MUX_1);
        break;
    case UART_PRINT_GPIO3:
        ll_gpio_afio_config(GPIO_PIN_3, AFIO_MUX_1);
        break;
    case UART_PRINT_GPIO5:
        ll_gpio_afio_config(GPIO_PIN_5, (gpio_afio_mux_e)GPIO5_SOFTWARE_INPUT_FUNCTION_LIN1_UART_1W);   //J1036 tx & rx
        break;
    case UART_PRINT_GPIO6:
        ll_gpio_afio_config(GPIO_PIN_6, AFIO_MUX_2); //needs an external pull-up resister (2K)
		CRG_CONFIG_UNLOCK();
		CRG->PWM_CLKRST_CTRL_F.PCLK_EN_PWM = ENABLE;
		CRG->PWM_CLKRST_CTRL_F.FCLK_EN_PWM = ENABLE;
		PWM->LED_CTRL_F.LED_LDO5V_EN = 1;
		CRG->ADC_CLKRST_CTRL_F.PCLK_EN_ADC = ENABLE;
		CRG->ADC_CLKRST_CTRL_F.FCLK_EN_ADC = ENABLE;	
		ADC->CTRL0_F.VREFBUF_EN = 1;
		CRG_CONFIG_LOCK();
        break;
    }
}
