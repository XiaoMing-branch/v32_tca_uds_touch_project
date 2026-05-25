/**
  ******************************************************************************
  * @brief  SPI Driver source file.
  *
  * @file   spi.c
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



#include "tcae10_ll_spi.h"
#include "tcae10_ll_gpio.h"


/**
 * @brief   反初始化SPI外设，复位所有寄存器
 * @param   None
 * @note    复位SPI外设时钟，将所有寄存器恢复默认值
 * @retval  None
 */
void ll_spi_deinit(void)
{
    CRG_CONFIG_UNLOCK();
    CRG->SPI_CLKRST_CTRL_F.RST_SPI = 1;                     /* 复位SPI外设 */
    __NOP();
    __NOP();
    CRG->SPI_CLKRST_CTRL_F.RST_SPI = 0;                     /* 释放复位 */
    __NOP();
    __NOP();
    CRG_CONFIG_LOCK();
}



/**
 * @brief   使能或禁能SPI从模式输出（3线模式使用）
 * @param   state - ENABLE使能从机输出，DISABLE禁能
 * @note    从机输出默认使能，在3线模式下可通过此函数禁用输出
 * @retval  None
 */
void ll_spi_slaveoutenable(FunctionalState state)
{
    SPI->CR0_F.IO_DIS = (state ? 0 : 1);                    /* 设置IO_DIS位控制从机输出 */
}



/**
 * @brief   使能或禁能SPI主模式输入（发送期间禁能接收）
 * @param   state - ENABLE使能主设备数据输入，DISABLE禁能
 * @note    主设备数据输入默认使能
 * @retval  None
 */
void ll_spi_masterinenable(FunctionalState state)
{
    SPI->CR0_F.IO_DIS = (state ? 0 : 1);                    /* 设置IO_DIS位控制主设备输入 */
}



/**
 * @brief   初始化SPI外设
 * @param   config - 指向SPI配置结构体的指针（包含模式、线数、数据长度、时钟极性/相位等）
 * @note    先复位SPI，再配置CR0寄存器中各参数：主从模式、线数、数据长度、
 *         帧格式、CSN极性、CRC、字节序、时钟相位/极性、回环模式等
 * @retval  None
 */
void ll_spi_init(SPI_COnfig_t *config)
{
    CRG_CONFIG_UNLOCK();
    CRG->SPI_CLKRST_CTRL_F.RST_SPI = 1;
    __NOP();
    __NOP();
    CRG->SPI_CLKRST_CTRL_F.RST_SPI = 0;
    __NOP();
    __NOP();

    SPI->CR0_F.SLV_MODE = config->SPI_Mode;
    SPI->CR0_F.WIRE_MODE_4 = config->SPI_WireMode;

    SPI->CR0_F.DATA_SIZE_DIS = config->SPI_DataLengthSelect;

    if (config->SPI_DataLengthSelect == SPI_DataLengthSelect_Data)
    {
        SPI->CR0_F.DATA_SIZE = config->SPI_DataLength;
    }
    else if (config->SPI_DataLengthSelect == SPI_DataLengthSelect_Instruction)
    {
        SPI->CR0_F.INS_SIZE = config->SPI_InstructionLength;
    }

    SPI->CR0_F.L_FRAME_EN = config->LongFrame_Enable;
    SPI->CR0_F.CSN_POL_SEL = config->SPI_CSNPolarity;
    SPI->CR0_F.CRC_EN = config->CRC_Enable;
    SPI->CR0_F.LSB_FIRST = config->SPI_Encoding;
    SPI->CR0_F.SPH = config->SPI_ClockPhase;
    SPI->CR0_F.SPO = config->SPI_ClockPolarity;
    SPI->CR0_F.LP_BACK_EN = config->LoopBack_Enable;
    SPI->CPSR_F.FCLK_DIV = config->ClockPrescale;               /* 设置时钟分频系数 */

}



/**
 * @brief   配置SPI相关的GPIO引脚复用功能
 * @param   w_mode - SPI接线模式，可取@ref SPI_WIREMODE_ENUM中的值
 * @note    3线模式配置CSN/CLK/DATA引脚，4线模式配置CSN/CLK/TXD/RXD引脚
 * @retval  None
 */
void ll_spi_ioconfig(SPI_WireMode_t w_mode)
{
    if (w_mode == SPI_WireMode_3Wires)
    {
        ll_gpio_afio_config(GPIO_PIN_3, (gpio_afio_mux_e)GPIO3_SOFTWARE_INPUT_FUNCTION_SPI_CSN);
        ll_gpio_afio_config(GPIO_PIN_1, (gpio_afio_mux_e)GPIO1_SOFTWARE_INPUT_FUNCTION_SPI_CLK);
        ll_gpio_afio_config(GPIO_PIN_2, (gpio_afio_mux_e)GPIO2_SOFTWARE_INPUT_FUNCTION_SPI_DATA);
    }

    else if (w_mode == SPI_WireMode_4Wires)
    {
        ll_gpio_afio_config(GPIO_PIN_3, (gpio_afio_mux_e)GPIO3_SOFTWARE_INPUT_FUNCTION_SPI_CSN);
        ll_gpio_afio_config(GPIO_PIN_1, (gpio_afio_mux_e)GPIO1_SOFTWARE_INPUT_FUNCTION_SPI_CLK);
        ll_gpio_afio_config(GPIO_PIN_4, (gpio_afio_mux_e)GPIO4_SOFTWARE_INPUT_FUNCTION_SPI_TXD);
        ll_gpio_afio_config(GPIO_PIN_2, (gpio_afio_mux_e)GPIO2_SOFTWARE_INPUT_FUNCTION_SPI_RXD);
    }
}



/**
 * @brief   配置SPI FIFO阈值
 * @param   fifo      - 要配置的FIFO（TX或RX），可取@ref SPI_FIFOSELECT_ENUM
 * @param   threshold - 阈值等级，可取@ref SPI_FIFOTHRESHOLD_ENUM
 * @retval  None
 */
void ll_spi_fifoconfig(SPI_FifoSelect_t fifo, SPI_FifoThreshold_t threshold)
{
    if (fifo == SPI_FifoSelect_Rx)
    {
        SPI->CR1_F.RX_FIFO_INT_TH = threshold;                 /* 设置RX FIFO中断阈值 */
    }
    else if (fifo == SPI_FifoSelect_Tx)
    {
        SPI->CR1_F.TX_FIFO_INT_TH = threshold;                 /* 设置TX FIFO中断阈值 */
    }
}



/**
 * @brief   使能或禁能SPI外设
 * @param   state - ENABLE使能SPI，DISABLE禁能
 * @retval  None
 */
void ll_spi_cmd(FunctionalState state)
{
    SPI->CR0_F.SPI_EN = (state ? 1 : 0);                    /* 设置SPI使能位 */
}



/**
 * @brief   通过SPI发送数据
 * @param   data   - 指向待发送数据的指针
 * @param   length - 数据长度（字为单位）
 * @note    阻塞等待TX FIFO非满后写入数据寄存器
 * @retval  None
 */
void ll_spi_senddata(const uint32_t *data, uint16_t length)
{
    while (length-- > 0)
    {
        SPI->WDR_F.TX_DATA = *data;                            /* 写入数据到发送寄存器 */
        while (SPI->SR_F.TX_FIFO_FULL_FLG != 0);               /* 等待TX FIFO非满 */
        data++;
    }

}



/**
 * @brief   通过SPI接收数据
 * @param   buffer - 指向接收缓冲区的指针
 * @param   length - 要接收的数据长度（字为单位）
 * @retval  None
 */
void ll_spi_receivedata(uint32_t *buffer, uint16_t length)
{
    for (uint8_t i = 0; i < length ; i++)
    {
        buffer[i] = SPI->RDR_F.RX_DATA;                        /* 从接收寄存器读取数据 */
    }
}



/**
 * @brief   获取SPI状态标志
 * @param   status_flag - 要查询的状态标志，可取@ref SPI_STATUS_ENUM中的值
 * @retval  true - 标志置位，false - 标志清除
 */
bool ll_spi_statusget(SPI_Status_t status_flag)
{
    return (SPI->SR & 1 << status_flag) ;                    /* 读取状态寄存器对应位 */
}



/**
 * @brief   使能SPI中断
 * @param   spi_int - 要使能的中断标志，可取@ref SPI_INTERRUPT_Definitions中的值或组合
 * @note    写IMR寄存器清除对应位以使能中断
 * @retval  None
 */
void ll_spi_interruptenable(uint16_t spi_int)
{
    SPI->IMR &= ~(spi_int);                                  /* 清除IMR对应位，使能中断 */
}



/**
 * @brief   禁能SPI中断
 * @param   spi_int - 要禁能的中断标志，可取@ref SPI_INTERRUPT_Definitions中的值或组合
 * @retval  None
 */
void ll_spi_interruptdisable(uint16_t spi_int)
{
    SPI->IMR |= spi_int;                                     /* 设置IMR对应位，禁能中断 */
}



/**
 * @brief   获取SPI中断标志状态
 * @param   spi_int - 要查询的中断标志，可取@ref SPI_INTERRUPT_Definitions中的值或组合
 * @retval  true - 中断已发生，false - 无中断
 */
bool ll_spi_interruptstatusget(uint16_t spi_int)
{
    return (SPI->ISR & spi_int) ;                            /* 读取ISR寄存器 */
}



/**
 * @brief   清除SPI中断标志
 * @param   spi_int - 要清除的中断标志，可取@ref SPI_INTERRUPT_Definitions中的值或组合
 * @retval  None
 */
void ll_spi_interruptclear(uint16_t spi_int)
{
    SPI->ICR |= spi_int;                                     /* 写ICR清除中断标志 */
}
