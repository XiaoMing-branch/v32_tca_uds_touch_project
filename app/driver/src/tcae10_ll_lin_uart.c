/**
 *****************************************************************************
 * @brief   sci driver source file.
 *
 * @file   tcae10_ll_lin_uart.c
 * @author AE/FAE team
 * @date   23/11/2022
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2022 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */
#include <stdio.h>
#include "tcae10.h"
#include "system_tcae10.h"
#include "tcae10_ll_lin_uart.h"
#include "tcae10_ll_gpio.h"

/*******************************************************************************
* Definitions:
******************************************************************************/

/*******************************************************************************
* Variables:
******************************************************************************/

uint32_t uartCount = 0;
uint32_t uartStatus = 0;
uint8_t  uartTxEmpty = 1;

/*******************************************************************************
* Prototypes:
******************************************************************************/

/*******************************************************************************
* Code:
******************************************************************************/

/**
 * @brief   反初始化LIN SCI1 UART外设
 * @note    复位LIN_SCI1外设寄存器
 * @retval  None
 */
void ll_lin_sci_uart_deinit(void)
{
    CRG_CONFIG_UNLOCK();
    CRG->LIN_SCI1_CLKRST_CTRL_F.RST_LIN_SCI1 = 1;              /* 复位LIN SCI1 */
    __NOP();
    __NOP();
    CRG->LIN_SCI1_CLKRST_CTRL_F.RST_LIN_SCI1 = 0;              /* 释放复位 */
    __NOP();
    __NOP();
    CRG_CONFIG_LOCK();
}

/**
 * @brief   设置LIN SCI工作模式（LIN模式或UART模式）
 * @param   mode - sci_mode_LIN（LIN模式）或sci_mode_UART（UART模式）
 * @note    配置CTRL_UART寄存器中的MODE位，同时配置停止位和多处理器模式
 * @retval  None
 */
void ll_lin_sci_uart_mode(sci_mode_t mode)
{
    LIN_SCI->CTRL_UART_F.MODE = mode;                           /* 设置模式：0-LIN，1-UART */
    /* set uart communication crc bit mode */
    /* set uart stop bit*/
    LIN_SCI->CTRL_UART_F.STP_BIT_SEL = 0;                       /* 设置停止位为1位 */
    /* set uart MP mode */
    LIN_SCI->CTRL_UART_F.MP_MODE_EN = 1;                        /* 使能多处理器模式 */
    LIN_SCI->CTRL_UART_F.MP_RX_ADDR_WR_EN = 1;                  /* 使能接收地址写入 */
    LIN_SCI->CTRL_UART_F.MP_TX_ADDR_DATA_SEL = 1;               /* 发送地址/数据选择 */
    /* set uart MP address */
    LIN_SCI->RX_CFG_F.MP_SLAVE_ADDR = 0xAA;                     /* 设置从机地址 */
    LIN_SCI->RX_CFG_F.MP_SLAVE_ADDR_MSK = 1;                    /* 使能地址掩码 */
}

/**
 * @brief   配置LIN SCI UART的IO引脚
 * @note    当前为空实现，使用默认引脚映射
 * @retval  None
 */
void ll_lin_sci_uart_io_config(void)
{
}

/**
 * @brief   获取LIN SCI UART外设时钟频率
 * @return  LIN SCI模块的实际工作时钟频率（Hz）
 * @note    根据CRG分频配置计算：fclk = hclk / (div + 1)
 */
static uint32_t lin_sci_uart_clock_get(void)
{
    uint32_t reg_val = 0;
    uint8_t status = 0;
    (void)(&status);

    reg_val = CRG->LIN_SCI1_CLKRST_CTRL;
    reg_val &= CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_MASK; /* 读取分频值 */
    reg_val >>= CRG_LIN_SCI1_CLKRST_CTRL_FCLK_DIV_LIN_SCI1_SHIFT;

    return ((SystemGetHClkFreq()) / (reg_val + 1));             /* 计算实际时钟频率 */
}

/**
 * @brief   设置SCI波特率分频寄存器
 * @param   dlh  - 分频系数高4位
 * @param   dll  - 分频系数低8位
 * @param   frac - 小数分频系数（4位）
 * @note    波特率 = fclk / (16 * (DLH:DLL + frac/16))
 * @retval  None
 */
static void sci_uart_divided(unsigned int dlh, unsigned int dll, unsigned int frac)
{
    uint32_t reg_val = 0;
    uint8_t status = 0;
    (void)(&status);

    dlh &= 0x0f;
    dll &= 0xff;
    frac &= 0x0f;
    reg_val |= (frac << 12 | dlh << 8 | dll);                   /* 组合波特率配置值 */
    LIN_SCI1->BAUD_CFG = reg_val;                               /* 写入波特率配置寄存器 */
}

/**
 * @brief   设置LIN SCI UART波特率
 * @param   baudrate - 目标波特率值
 * @note    根据外设时钟频率自动计算分频系数，支持小数分频以实现精确波特率
 * @retval  None
 */
void ll_lin_sci_uart_setbaudrate(uint32_t baudrate)
{
    uint32_t  clk;

    uint32_t div;
    uint8_t frac = 0;
    clk = lin_sci_uart_clock_get();                             /* 获取外设时钟频率 */
    /* Fck/(16*Baud_Rate) */
    div = clk >> 4;                                             /* 除以16 */
    frac = div % baudrate;                                      /* 计算余数 */
    frac = (frac << 4) / baudrate;                              /* 计算小数分频 */
    div  = div / baudrate;                                      /* 整数分频 */
    sci_uart_divided(((div >> 8) & 0xff), ((div >> 0) & 0xff), frac);  /* 写分频寄存器 */
}

/**
 * @brief   配置LIN RX接收滤波器时间
 * @param   filter - 滤波器时间常数
 * @note    用于滤除LIN总线上的噪声毛刺，同时配置RX延迟
 * @retval  None
 */
void ll_lin_sci_uart_rxfilter_cfg(uint16_t filter)
{
    LIN_SCI->RX_FILTER_CFG_F.RX_FILTER_TIM = filter;            /* 设置接收滤波时间 */
    TEST->TEST_LIN_CTRL_F.LIN_RX_DELAY = 0;                    /* 配置LIN RX延迟 */
}

/**
 * @brief   初始化LIN SCI UART（设置为UART模式）
 * @note    写入CTRL_UART寄存器MODE位为1（UART模式）
 * @retval  None
 */
void ll_lin_sci_uart_init(void)
{
    LIN_SCI->CTRL_UART_F.MODE = 1;                              /* 设置为UART模式 */
}

/**
 * @brief   使能或禁能LIN SCI外设
 * @param   en - TRUE使能，FALSE禁能
 * @note    清除TX/RX FIFO，配置TX/RX使能，设置全局使能
 * @retval  None
 */
void ll_lin_sci_uart_enable(boolean_t en)
{
    /* 清除TX FIFO和RX FIFO */
    LIN_SCI->CTRL_F.TX_FIFO_CLR = 1;
    LIN_SCI->CTRL_F.RX_FIFO_CLR = 1;
    LIN_SCI->CTRL_F.TX_NUM_MODE = 0;
    LIN_SCI->CTRL_F.RX_NUM_MODE = 0;
    LIN_SCI->CTRL_F.TX_EN = 1;                                   /* 使能发送 */
    LIN_SCI->CTRL_F.RX_EN = 1;                                   /* 使能接收 */
    LIN_SCI->CTRL_F.GLB_EN = 1;                                  /* 全局使能 */
    LIN_SCI->CTRL_F.TX_NUM = 0;
    LIN_SCI->CTRL_F.RX_NUM = 0;
}

/**
 * @brief   通过LIN SCI UART发送一个字节
 * @param   data - 待发送的字节数据
 * @note    阻塞等待TX FIFO非满后写入数据
 * @retval  None
 */
void ll_lin_sci_uart_send_byte(uint8_t data)
{
    LIN_SCI->TX_DATA_F.TX_DATA = data;                           /* 写入发送数据 */
    while (LIN_SCI->STATUS_F.TX_FIFO_FULL);                     /* 等待TX FIFO非满 */
}

/**
 * @brief   通过LIN SCI UART发送多个字节
 * @param   pu8Buffer - 指向待发送数据的指针
 * @param   u32Len    - 数据长度
 * @retval  None
 */
void ll_lin_sci_uart_send_bytes(uint8_t *pu8Buffer, uint32_t u32Len)
{
    while (u32Len != 0)
    {
        ll_lin_sci_uart_send_byte(*pu8Buffer);                  /* 逐字节发送 */
        pu8Buffer++;
        u32Len--;
    }
}

/**
 * @brief   选择LIN SCI TX检测点位置
 * @param   sel - 0：在TX位中间检测；1：在TX位最后一个FCLK检测
 * @retval  None
 */
void ll_lin_sci_tx_check_point_select(uint8_t sel)
{
    LIN_SCI->TX_CFG_F.CHK_PT_SEL = sel;                         /* 设置检测点位置 */
}

/**
 * @brief   使能LIN SCI中断
 * @param   lin_sci_int - 要使能的中断标志位
 * @note    写IMR清除对应位使能中断
 * @retval  None
 */
void ll_lin_sci_interrupt_enable(uint32_t lin_sci_int)
{
    LIN_SCI->IMR &= ~(lin_sci_int);                              /* 清除IMR对应位，使能中断 */
}

/**
 * @brief   禁能LIN SCI中断
 * @param   lin_sci_int - 要禁能的中断标志位
 * @retval  None
 */
void ll_lin_sci_interrupt_disable(uint32_t lin_sci_int)
{
    LIN_SCI->IMR |= lin_sci_int;                                 /* 设置IMR对应位，禁能中断 */
}

/**
 * @brief   获取LIN SCI中断状态
 * @param   lin_sci_int - 要查询的中断标志位
 * @retval  true - 中断已发生，false - 无中断
 */
bool ll_lin_sci_interrupt_status_get(uint32_t lin_sci_int)
{
    return ((LIN_SCI->ISR & lin_sci_int));                      /* 读取ISR寄存器 */
}

/**
 * @brief   清除LIN SCI中断标志
 * @param   lin_sci_int - 要清除的中断标志位
 * @retval  None
 */
void ll_lin_sci_interrupt_clear(uint32_t lin_sci_int)
{
    LIN_SCI->ICR |= lin_sci_int;                                 /* 写ICR清除中断标志 */
}
