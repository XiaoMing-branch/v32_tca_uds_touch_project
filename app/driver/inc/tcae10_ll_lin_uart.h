/**
  ******************************************************************************
  * @brief  LIN SCI Driver header file.  
  *
  * @file   tcae10_ll_lin_uart.h
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


#ifndef __TCAE10_LL_LINUART_H__
#define __TCAE10_LL_LINUART_H__


#include <stdint.h>
#include <stdbool.h>

#include "tcae10.h"


/** @addtogroup TCPL02_DRIVER
  * @{
  */


/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @brief  发送模式选择（0: 轮询，1: 中断）
 */
#define TX_INTERRUPT    0   /* 0 if TX uses polling, 1 interrupt driven. */


/**
  * @brief  SCI_MODE_ENUM enumeration definition
  */
typedef enum 
{
    sci_mode_LIN           = 0,
    sci_mode_UART          = 1,
}sci_mode_t;
/**
  * @}
  */



/** @defgroup LIN_SCI_FIFO
  * @{
  */
/** @brief  清除发送FIFO */
#define tx_fifo_clr()       (LIN_SCI1->CTRL_F.TX_FIFO_CLR = 1)
/** @brief  清除接收FIFO */
#define rx_fifo_clr()       (LIN_SCI1->CTRL_F.RX_FIFO_CLR=1)
/** @brief  中止发送状态 */
#define tx_state_abort()    (LIN_SCI1->CTRL_F.TX_ABORT=1)
/** @brief  中止接收状态 */
#define rx_state_abort()    (LIN_SCI1->CTRL_F.RX_ABORT=1)
/** @brief  触发发送Break信号 */
#define break_tx_trig()     (LIN_SCI1->CTRL_F.BRK_TX_TRIG=1)
/**
  * @}
  */




/** @defgroup LIN_SCI_INT
  * @{
  */    
#define LIN_SCI1_INT_RX_VALID             LIN_SCI1_IMR_RX_FIFO_VLD_INT_MSK_MASK
#define LIN_SCI1_INT_SHORT_TO_GND         LIN_SCI1_IMR_SHORT_GND_DET_INT_MSK_MASK    //Short to GND detected interrupt
#define LIN_SCI1_INT_RX_DATA_DONE         LIN_SCI1_IMR_RX_DATA_DONE_INT_MSK_MASK     //Receive all data done(when rx_num_mode =1)  
#define LIN_SCI1_INT_SLV_TX_BRK_DONE      LIN_SCI1_IMR_SLV_TX_BRK_DONE_INT_MSK_MASK  //slave tx break done 
#define LIN_SCI1_INT_TX_1BYTE_DONE        LIN_SCI1_IMR_TX_1BYTE_DONE_INT_MSK_MASK    //tx 1byte done 
#define LIN_SCI1_INT_MP_MODE_ADDR         LIN_SCI1_IMR_MP_MODE_ADDR_INT_MSK_MASK     //multi_processor mode address received 
#define LIN_SCI1_INT_TX_RX_CONF           LIN_SCI1_IMR_TX_RX_CONF_INT_MSK_MASK       //tx rx conflict   
#define LIN_SCI1_INT_TX_PID_DONE          LIN_SCI1_IMR_TX_PID_DONE_INT_MSK_MASK      //tx pid done 
#define LIN_SCI1_INT_SYNC_VAL_ERR         LIN_SCI1_IMR_SYNC_VAL_ERR_INT_MSK_MASK     //Sync value check error 
#define LIN_SCI1_INT_SYNC_DET             LIN_SCI1_IMR_SYNC_DET_INT_MSK_MASK         //Sync bits detect 
#define LIN_SCI1_INT_BRK_DET              LIN_SCI1_IMR_BRK_DET_INT_MSK_MASK         //Break bits detect   
#define LIN_SCI1_INT_TX_FIFO_EMPTY        LIN_SCI1_IMR_TX_FIFO_EMPTY_INT_MSK_MASK    //Transmit fifo empty 
#define LIN_SCI1_INT_TX_DONE              LIN_SCI1_IMR_TX_DONE_INT_MSK_MASK          //Transmit done(when tx_num_mode =1) 
#define LIN_SCI1_INT_RX_FIFO_OVF_ERR      LIN_SCI1_IMR_RX_FIFO_OVF_ERR_INT_MSK_MASK  //Receive fifo overflow 
#define LIN_SCI1_INT_RX_FIFO_FULL         LIN_SCI1_IMR_RX_FIFO_FULL_INT_MSK_MASK     //Receive fifo full 
#define LIN_SCI1_INT_RX_STP_ERR           LIN_SCI1_IMR_RX_STP_ERR_INT_MSK_MASK       //Receive no stop bit error 
#define LIN_SCI1_INT_RX_CHKSUM_ERR        LIN_SCI1_IMR_RX_CHKSUM_ERR_INT_MSK_MASK    //Receive checksum error 
#define LIN_SCI1_INT_RX_PTY_CHK_ERR       LIN_SCI1_IMR_RX_PTY_CHK_ERR_INT_MSK_MASK   //Receive parity check error (in LIN parity check only in PID).
#define LIN_SCI1_INT_RX_PID_DONE          LIN_SCI1_IMR_RX_PID_DONE_INT_MSK_MASK      //Receive pid done 
#define LIN_SCI1_INT_RX_DONE              LIN_SCI1_IMR_RX_DONE_INT_MSK_MASK          //Receive all data and checksum done(when rx_num_mode =1) 
#define LIN_SCI1_INT_RX_1BYTE_DONE        LIN_SCI1_IMR_RX_1BYTE_DONE_INT_MSK_MASK    //Receive each byte data done 
/**                                                                                
  * @}                                                                             
  */
  
  
  
/**
 * @brief  去初始化LIN/SCI/UART模块
 */
void ll_lin_sci_uart_deinit(void);
/**
 * @brief  配置LIN/SCI/UART IO引脚
 */
void ll_lin_sci_uart_io_config(void);
/**
 * @brief  使能/禁能LIN/SCI/UART模块
 * @param en - TRUE: 使能，FALSE: 禁能
 */
void ll_lin_sci_uart_enable(boolean_t en);
/**
 * @brief  LIN/SCI/UART发送单字节
 * @param data - 要发送的字节数据
 */
void ll_lin_sci_uart_send_byte(uint8_t data);
/**
 * @brief  LIN/SCI/UART发送多字节数据
 * @param pu8Buffer - 数据缓冲区指针
 * @param u32Len - 发送数据长度（字节）
 */
void ll_lin_sci_uart_send_bytes(uint8_t *pu8Buffer, uint32_t u32Len);
/**
 * @brief  设置LIN/SCI/UART波特率
 * @param baudrate - 目标波特率
 */
void ll_lin_sci_uart_setbaudrate(uint32_t baudrate);
/**
 * @brief  设置LIN/SCI/UART工作模式
 * @param mode - 工作模式（LIN或UART）@ref sci_mode_t
 */
void ll_lin_sci_uart_mode(sci_mode_t mode);
/**
 * @brief  配置LIN/SCI/UART接收过滤器
 * @param filter - 过滤器配置值
 */
void ll_lin_sci_uart_rxfilter_cfg(uint16_t filter);
/**
 * @brief  使能LIN/SCI/UART中断
 * @param lin_sci_int - 中断标志位
 */
void ll_lin_sci_uart_interrupt_enable(uint32_t lin_sci_int);
/**
 * @brief  禁能LIN/SCI/UART中断
 * @param lin_sci_int - 中断标志位
 */
void ll_lin_sci_uart_interrupt_disable(uint32_t lin_sci_int);
/**
 * @brief  获取LIN/SCI/UART中断状态
 * @param lin_sci_int - 中断标志位
 * @retval true: 中断已触发，false: 中断未触发
 */
bool ll_lin_sci_uart_interrupt_status_get(uint32_t lin_sci_int);
/**
 * @brief  清除LIN/SCI/UART中断标志
 * @param lin_sci_int - 中断标志位
 */
void ll_lin_sci_uart_interrupt_clear(uint32_t lin_sci_int);
  
  
#ifdef __cplusplus
}
#endif
/**
  * @}
  */
#endif /* __TCAE10_LL_LINUART_H__ */
