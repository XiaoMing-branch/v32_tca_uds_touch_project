/**
  ******************************************************************************
  * @brief  SPI driver header file.  
  *
  * @file   tcae10_ll_spi.h
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



#ifndef __TCAE10_LL_SPI_H__
#define __TCAE10_LL_SPI_H__


#include <stdint.h>
#include <stdbool.h>
#include "tcae10.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup TCAE01_DRIVER
  * @{
  */


/** @defgroup SPI_FIFO_LENGTH_Definitions
  * @{
  */
#define SPI_FIFO_LENGTH_WORD    4
/**
  * @}
  */



/** @defgroup SPI_FIFO_CLEAR_Definitions
  * @{
  */
#define SPI_RX_FIFO_CLEAR()     SPI->CR1_F.RX_FIFO_CLR
#define SPI_TX_FIFO_CLEAR()     SPI->CR1_F.TX_FIFO_CLR
/**
  * @}
  */



/**
  * @brief  SPI_CSNPOLARITY_ENUM enumeration definition
  */
typedef enum
{
    SPI_CSNPolarity_ActiveLow = 0,
    SPI_CSNPolarity_ActiveHigh = 1,
} SPI_CSNPolarity_t;
/**
  * @}
  */




/**
  * @brief  SPI_WIREMODE_ENUM enumeration definition
  */
typedef enum
{
    SPI_WireMode_3Wires = 0,
    SPI_WireMode_4Wires = 1
} SPI_WireMode_t;
/**
  * @}
  */



/**
  * @brief  SPI_ENCODING_ENUM enumeration definition
  */
typedef enum
{
    SPI_Encoding_LittleEndian = 0,
    SPI_Encoding_BigEndian,
} SPI_Encoding_t;
/**
  * @}
  */



/**
  * @brief  SPI_CLOCKPOLARITY_ENUM enumeration definition
  */
typedef enum
{
    SPI_ClockPolarity_Positive,
    SPI_ClockPolarity_Negtive,
} SPI_ClockPolarity_t;
/**
  * @}
  */



/**
  * @brief  SPI_INSTRUCTIONLENGTH_ENUM enumeration definition
  */
typedef enum
{
    SPI_InstructionLength_1Byte = 0,
    SPI_InstructionLength_2Bytes,
} SPI_InstructionLength_t;
/**
  * @}
  */



/**
  * @brief  SPI_DATALENGTHSELECT_ENUM enumeration definition
  */
typedef enum
{
    SPI_DataLengthSelect_Data = 0,    /*!< Data size selected by data_size register.*/
    SPI_DataLengthSelect_Instruction, /*!< Data Size selected by instruction length register */
} SPI_DataLengthSelect_t;
/**
  * @}
  */



/**
  * @brief  SPI_MODE_ENUM enumeration definition
  */
typedef enum
{
    SPI_Mode_Master = 0,
    SPI_Mode_Slave,
} SPI_Mode_t;
/**
  * @}
  */



/**
  * @brief  SPI_DATALENGTH_ENUM enumeration definition
  */
typedef enum
{
    SPI_DataLength_1Byte = 0,   /*!< valid in both 4wire mode and 3 wire mode */
    SPI_DataLength_2Byte,       /*!< valid in both 4wire mode and 3 wire mode */
    SPI_DataLength_3Byte,       /*!< valid in both 4wire mode and 3 wire mode */
    SPI_DataLength_4Byte,       /*!< valid only in 3 wire mode*/
} SPI_DataLength_t;
/**
  * @}
  */



/**
  * @brief  SPI_FIFOTHRESHOLD_ENUM enumeration definition
  */
typedef enum
{
    SPI_FifoThreshold_1Word = 0,
    SPI_FifoThreshold_2Word,
    SPI_FifoThreshold_3Word,
    SPI_FifoThreshold_4Word,
} SPI_FifoThreshold_t;
/**
  * @}
  */



/**
  * @brief  SPI_FIFOSELECT_ENUM enumeration definition
  */
typedef enum
{
    SPI_FifoSelect_Tx = 0,
    SPI_FifoSelect_Rx
} SPI_FifoSelect_t;
/**
  * @}
  */



/**
  * @brief  SPI_STATUS_ENUM enumeration definition
  */
typedef enum
{
    SPI_Status_TxFifoEmpty = 0,
    SPI_Status_TxFifoFull,
    SPI_Status_RxFifoEmpty,
    SPI_Status_RxFifoFull,
    SPI_Status_Busy,
} SPI_Status_t;
/**
  * @}
  */



/**
  * @brief  SPI_CLOCKPHASE_ENUM enumeration definition
  */
typedef enum
{
    SPI_ClockPhase_Low=0,
    SPI_ClockPhase_High,
}SPI_ClockPhase_t;
/**
  * @}
  */




/** @defgroup SPI_INTERRUPT_Definitions
  * @{
  */
#define SPI_INT_RX_FIFO_OVERFLOW    SPI_IMR_RX_FIFO_OR_INT_MSK_MASK
#define SPI_INT_RX_FIFO_VALID       SPI_IMR_RX_FIFO_INT_MSK_MASK
#define SPI_INT_TX_FIFO_VALID       SPI_IMR_TX_FIFO_INT_MSK_MASK
#define SPI_INT_CMD_RECEIVED        SPI_IMR_INS_RX_FIN_INT_MSK_MASK    /*!< Only valid in 3 wire mode */
#define SPI_INT_CRC_ERROR           SPI_IMR_CRC_ERR_INT_MSK_MASK
#define SPI_INT_TX_CONFLICT         SPI_IMR_TX_CONF_ERR_INT_MSK_MASK    /*!< ONLY VALID IN 3 WIRE MODE */
/**
  * @}
  */



/** @defgroup  SPI_Configurations structure
  * @{
  */
typedef struct
{
    bool                        LongFrame_Enable;       /*!< Enable or disalbe long frame
                                                             this parameter is valid only in 4 wire mode*/
    
    SPI_CSNPolarity_t           SPI_CSNPolarity;        /*!< Specifies the polarity of CSN pin, 
                                                              this parameter can be any value of @ref SPI_CSNPOLARITY_ENUM*/
    
    bool                        CRC_Enable;             /*!< Enable or disalbe CRC 
                                                             this parameter can be true or false*/
    
    SPI_WireMode_t              SPI_WireMode;           /*!< Specifies the SPI wiring mode,
                                                             this parameter can be any value of @ref SPI_WIREMODE_ENUM*/
    
    SPI_Encoding_t              SPI_Encoding;           /*!< Specifies the SPI endian encoding,
                                                             this parameter can be any value of @ref SPI_ENCODING_ENUM */
    
    SPI_ClockPhase_t            SPI_ClockPhase;         /*!< Specifies the SPI clock phase. Only for Motorola SPI frame format.
                                                             This parameter can be any value of @ref SPI_CLOCKPHASE_ENUM */
    
    SPI_ClockPolarity_t         SPI_ClockPolarity;      /*!< Specifies SPI clock polarity.Only for Motorola SPI frame format.
                                                             This parameter can be any value of @ref SPI_CLOCKPOLARITY_ENUM */

    SPI_InstructionLength_t     SPI_InstructionLength;  /*!< Specifies the instruction length ,only valid in 3 wire mode
                                                             This parameter can be any value of @ref SPI_INSTRUCTIONLENGTH_ENUM */
    
    SPI_DataLengthSelect_t      SPI_DataLengthSelect;   /*!< Specifies the data length selection source
                                                             This parameter can be any value of @ref SPI_DATALENGTHSELECT_ENUM */

    SPI_DataLength_t            SPI_DataLength;         /*!< Specifies the data length,
                                                             This parameter can be any value of @ref SPI_DATALENGTH_ENUM */
                                                             
    bool                        SlaveOutput_Disable;    /*!< Disable the output in slave mode, Only valid in 4 wire mode
                                                             this parameter can be true or false*/
    
    SPI_Mode_t                  SPI_Mode;               /*!< Specifies the SPI wokring mode, 
                                                             this parameter can be any value of @ref SPI_MODE_ENUM*/
                                                              
    bool                        LoopBack_Enable;        /*!< Enables loopback mode, in loopback mode Rx = Tx, only valid in 4 wire mode
                                                             This parameter can be true or false*/
                                                             
    uint8_t                     ClockPrescale;          /*!< (4bit 0~15) value of clkc divider ( fclk is 3Mhz~48Mhz), 
                                                             Spi rate is  fclk/(2* (fclk_div+1).*/
    
    bool                        ConflictDetect_Enable;  /*!< Enable TX send conflict detect,used in 3wire mode & GPIO open drain mode.
                                                             Thsi parameter can be true or false*/

} SPI_COnfig_t;
/**
  * @}
  */



/**
 * @brief  去初始化SPI模块
 */
void ll_spi_deinit(void);
/**
 * @brief  初始化SPI模块
 * @param config - SPI配置结构体指针 @ref SPI_COnfig_t
 */
void ll_spi_init( SPI_COnfig_t* config );
/**
 * @brief  配置SPI FIFO阈值
 * @param fifo - FIFO选择（发送或接收）@ref SPI_FifoSelect_t
 * @param threshold - 阈值配置 @ref SPI_FifoThreshold_t
 */
void ll_spi_fifoconfig( SPI_FifoSelect_t fifo, SPI_FifoThreshold_t threshold );
/**
 * @brief  使能/禁能SPI命令模式
 * @param state - ENABLE: 使能，DISABLE: 禁能
 */
void ll_spi_cmd( FunctionalState state );
/**
 * @brief  SPI发送数据
 * @param data - 发送数据缓冲区
 * @param length - 发送数据长度（字）
 */
void ll_spi_senddata( const uint32_t* data, uint16_t length );
/**
 * @brief  SPI接收数据
 * @param buffer - 接收数据缓冲区
 * @param length - 接收数据长度（字）
 */
void ll_spi_receivedata( uint32_t* buffer, uint16_t length );
/**
 * @brief  获取SPI状态
 * @param status_flag - 状态标志 @ref SPI_Status_t
 * @retval true: 状态有效，false: 状态无效
 */
bool ll_spi_statusget( SPI_Status_t status_flag );
/**
 * @brief  使能SPI中断
 * @param spi_int - 中断标志位
 */
void ll_spi_interruptenable ( uint16_t spi_int );
/**
 * @brief  禁能SPI中断
 * @param spi_int - 中断标志位
 */
void ll_spi_interruptdisable ( uint16_t spi_int );
/**
 * @brief  获取SPI中断状态
 * @param spi_int - 中断标志位
 * @retval true: 中断已触发，false: 中断未触发
 */
bool ll_spi_interruptstatusget ( uint16_t spi_int );
/**
 * @brief  清除SPI中断标志
 * @param spi_int - 中断标志位
 */
void ll_spi_interruptclear ( uint16_t spi_int );
/**
 * @brief  使能/禁能SPI从机输出
 * @param state - ENABLE: 使能输出，DISABLE: 禁能输出
 */
void ll_spi_slaveoutenable(FunctionalState state);
/**
 * @brief  使能/禁能SPI主机输入
 * @param state - ENABLE: 使能输入，DISABLE: 禁能输入
 */
void ll_spi_masterinenable(FunctionalState state);
/**
 * @brief  配置SPI IO引脚模式
 * @param w_mode - 接线模式（3线或4线）@ref SPI_WireMode_t
 */
void ll_spi_ioconfig(SPI_WireMode_t w_mode);

#ifdef __cplusplus
}

#endif  /*__cplusplus*/
/**
  * @}
  */
#endif /* __TCAE10_LL_SPI_H__ */


