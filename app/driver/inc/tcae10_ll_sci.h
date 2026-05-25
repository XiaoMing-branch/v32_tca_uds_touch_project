/**
 *****************************************************************************
 * @brief   sci driver header.
 *
 * @file    tcae10_ll_sci.h
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

#ifndef __TCAE10_LL_SCI_H__
#define __TCAE10_LL_SCI_H__

#include "tcae10_ll_def.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief  SCI中断偏移量
 */
#define SCI_INT_OFFSET                (0)

/** @defgroup LIN_SCI_INT
  * @{
  */
#define SCI_INT_RX_1BYTE_DONE        LIN_SCI_IMR_RX_1BYTE_DONE_INT_MSK_MASK    //Receive each byte data done
#define SCI_INT_RX_DONE              LIN_SCI_IMR_RX_DONE_INT_MSK_MASK          //Receive all data and checksum done(when rx_num_mode =1)
#define SCI_INT_RX_PID_DONE          LIN_SCI_IMR_RX_PID_DONE_INT_MSK_MASK      //Receive pid done
#define SCI_INT_RX_PTY_CHK_ERR       LIN_SCI_IMR_RX_PTY_CHK_ERR_INT_MSK_MASK   //Receive parity check error (in LIN parity check only in PID).
#define SCI_INT_RX_CHKSUM_ERR        LIN_SCI_IMR_RX_CHKSUM_ERR_INT_MSK_MASK    //Receive checksum error
#define SCI_INT_RX_STP_ERR           LIN_SCI_IMR_RX_STP_ERR_INT_MSK_MASK       //Receive no stop bit error
#define SCI_INT_RX_FIFO_FULL         LIN_SCI_IMR_RX_FIFO_FULL_INT_MSK_MASK     //Receive fifo full
#define SCI_INT_RX_FIFO_OVF_ERR      LIN_SCI_IMR_RX_FIFO_OVF_ERR_INT_MSK_MASK  //Receive fifo overflow
#define SCI_INT_TX_DONE              LIN_SCI_IMR_TX_DONE_INT_MSK_MASK          //Transmit done(when tx_num_mode =1)
#define SCI_INT_TX_FIFO_EMPTY        LIN_SCI_IMR_TX_FIFO_EMPTY_INT_MSK_MASK    //Transmit fifo empty
//SCI_INT_TX_COL_DET_ERR
#define SCI_INT_BRK_DET              LIN_SCI_IMR_BRK_DET_INT_MSK_MASK         //Break bits detect
#define SCI_INT_SYNC_DET             LIN_SCI_IMR_SYNC_DET_INT_MSK_MASK         //Sync bits detect
#define SCI_INT_SYNC_VAL_ERR         LIN_SCI_IMR_SYNC_VAL_ERR_INT_MSK_MASK     //Sync value check error
#define SCI_INT_TX_PID_DONE          LIN_SCI_IMR_TX_PID_DONE_INT_MSK_MASK      //tx pid done
#define SCI_INT_TX_RX_CONF           LIN_SCI_IMR_TX_RX_CONF_INT_MSK_MASK       //tx rx conflict
//..new
#define SCI_INT_MP_MODE_ADDR         LIN_SCI_IMR_MP_MODE_ADDR_INT_MSK_MASK     //multi_processor mode address received
#define SCI_INT_SLV_SELECTED         LIN_SCI_IMR_SLV_SELECTED_INT_MSK_MASK    //LIN AA done
#define SCI_INT_AUTO_ADDR_DONE       LIN_SCI_IMR_AUTO_ADDR_DONE_INT_MSK_MASK    //LIN AA done
#define SCI_INT_TX_1BYTE_DONE        LIN_SCI_IMR_TX_1BYTE_DONE_INT_MSK_MASK    //tx 1byte done
#define SCI_INT_SLV_TX_BRK_DONE      LIN_SCI_IMR_SLV_TX_BRK_DONE_INT_MSK_MASK  //slave tx break done
#define SCI_INT_RX_DATA_DONE         LIN_SCI_IMR_RX_DATA_DONE_INT_MSK_MASK     //Receive all data done(when rx_num_mode =1)
#define SCI_INT_SHORT_TO_GND         LIN_SCI_IMR_SHORT_GND_DET_INT_MSK_MASK    //Short to GND detected interrupt
#define SCI_INT_RX_VALID             LIN_SCI_IMR_RX_FIFO_VLD_INT_MSK_MASK

/**
  * @brief  ll sci bus enumeration
  */
typedef enum
{
    LL_SCI_BUS_0 = 0,
    LL_SCI_BUS_1,
    LL_SCI_BUS_2,
    LL_SCI_BUS_MAX
} ll_sci_bus_e;

/**
  * @brief  ll sic mode enumeration
  */
typedef enum
{
    SCI_MODE_LIN_S         = 0,
    SCI_MODE_LIN_M,
    SCI_MODE_UART,
    SCI_MODE_MAX,
} ll_sci_mode_e;

/**
  * @brief  ll sic clear enumeration
  */
typedef enum
{
    SCI_CLEAR_NULL          = (0),
    SCI_CLEAR_TX_FIFO     = (0x01 << 0),
    SCI_CLEAR_RX_FIFO     = (0x01 << 1),
    SCI_CLEAR_TX_ABORT    = (0x01 << 2),
    SCI_CLEAR_RX_ABORT    = (0x01 << 3),
} ll_sci_clear_type_e;

/**
  * @brief  ll sic mode enumeration
  */
typedef enum
{
    LIN_CHECKSUM_CLASSIC           = 0,
    LIN_CHECKSUM_ENHANCED          = 1,
    LIN_CHECKSUM_MAX,
} ll_lin_checksum_e;

typedef struct
{
    ll_clk_config_t clk_cfg;
    ll_isr_config_t isr_cfg;
    ll_sci_mode_e mode;
    uint32_t baudrate;
} sci_config_t;


#define IS_SCI_BUS(__BUS__)         (((__BUS__) == LL_SCI_BUS_0) || ((__BUS__) ==LL_SCI_BUS_1) )
#define IS_SCI_MODE(__MODE__)       ((__MODE__) < SCI_MODE_MAX )

/**
 * @brief  去初始化SCI模块
 * @param bus - SCI总线 @ref ll_sci_bus_e
 */
void ll_sci_deinit(ll_sci_bus_e bus);
/**
 * @brief  初始化SCI模块
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param config - SCI配置结构体指针
 * @param callback - 中断回调函数指针
 */
void ll_sci_init(ll_sci_bus_e bus, sci_config_t *config, ISR_FUNC_CALLBACK callback);
/**
 * @brief  配置SCI波特率
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param baudrate - 目标波特率
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_sci_baudrate_config(ll_sci_bus_e bus, uint32_t baudrate);
/**
 * @brief  使能/禁能SCI中断
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param enable - true: 使能，false: 禁能
 * @retval LL_OK 成功
 */
ll_status_e ll_sci_isr_enable(ll_sci_bus_e bus, bool enable);
/**
 * @brief  清除SCI状态（FIFO/中止）
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param type - 清除类型 @ref ll_sci_clear_type_e
 */
void ll_sci_state_clear(ll_sci_bus_e bus, ll_sci_clear_type_e type);
/**
 * @brief  设置LIN接收延迟
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param count - 延迟计数值
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_rx_delay_set(ll_sci_bus_e bus, uint8_t count);
/**
 * @brief  使能/禁能LIN唤醒功能
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param enable - true: 使能，false: 禁能
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_wakeup_enable(ll_sci_bus_e bus, bool enable);
/**
 * @brief  使能LIN自动寻址功能
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param type - 自动寻址类型 @ref lin_aa_type_e
 * @param ext_shunt_res - true: 使用外部分流电阻，false: 内部
 * @param cur_th - 电流阈值输出指针
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_lin_aa_enable(ll_sci_bus_e bus, lin_aa_type_e type, bool ext_shunt_res, uint16_t *cur_th);
/**
 * @brief  禁能LIN自动寻址功能
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_aa_disable(ll_sci_bus_e bus);
/**
 * @brief  设置LIN自动寻址就绪状态
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param enable - true: 就绪，false: 未就绪
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_aa_ready_set(ll_sci_bus_e bus, bool enable);
/**
 * @brief  SCI模式发送数据
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param buffer - 发送数据缓冲区
 * @param length - 发送数据长度
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_sci_transmit(ll_sci_bus_e bus, uint8_t *buffer, uint16_t length);
/**
 * @brief  SCI模式接收数据
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param buffer - 接收数据缓冲区
 * @param length - 接收数据长度
 * @retval LL_OK 成功，LL_COMM_ERROR 通信错误
 */
ll_status_e ll_sci_receive(ll_sci_bus_e bus, uint8_t *buffer, uint16_t length);
/**
 * @brief  LIN模式发送数据帧
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param pid - LIN协议ID（保护ID）
 * @param buffer - 发送数据缓冲区
 * @param length - 发送数据长度
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_lin_transmit(ll_sci_bus_e bus, uint8_t pid, uint8_t *buffer, uint16_t length);
/**
 * @brief  LIN模式接收数据帧
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param pid - LIN协议ID（保护ID）
 * @param buffer - 接收数据缓冲区
 * @param length - 接收数据长度
 * @retval LL_OK 成功，LL_COMM_ERROR 通信错误
 */
ll_status_e ll_lin_receive(ll_sci_bus_e bus, uint8_t pid, uint8_t *buffer,  uint16_t length);
/**
 * @brief  计算LIN校验和
 * @param pid - LIN协议ID
 * @param buffer - 数据缓冲区
 * @param length - 数据长度
 * @retval 计算后的校验和
 */
uint8_t ll_lin_checksum_calib_func(uint8_t pid, uint8_t *buffer, uint16_t length);
/**
 * @brief  读取LIN接收到的PID
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param pid - PID值输出指针
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_pid_read(ll_sci_bus_e bus, uint8_t *pid);
/**
 * @brief  读取LIN接收的单字节数据
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param byte - 字节数据输出指针
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_read_byte(ll_sci_bus_e bus, uint8_t *byte);
/**
 * @brief  读取LIN自动检测的波特率
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param baud - 波特率值输出指针
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_auto_baudrate_read(ll_sci_bus_e bus, uint32_t *baud);
/**
 * @brief  读取LIN当前配置的波特率
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param baud - 波特率值输出指针
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_baudrate_read(ll_sci_bus_e bus, uint32_t *baud);
/**
 * @brief  使能/禁能LIN全局使能
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param sw - true: 使能，false: 禁能
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_ctrl_glben(ll_sci_bus_e bus, bool sw);
/**
 * @brief  中止LIN接收
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param sw - true: 中止接收
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_ctrl_rx_abort(ll_sci_bus_e bus, bool sw);
/**
 * @brief  发送LIN break信号
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param brk_num - break长度（字节数）
 * @retval LL_OK 成功
 */
ll_status_e ll_lin_ctrl_brk_tx(ll_sci_bus_e bus, uint8_t brk_num);
/**
 * @brief  发送LIN帧头（Break + Sync + PID）
 * @param bus - SCI总线 @ref ll_sci_bus_e
 * @param pid - LIN协议ID
 * @retval LL_OK 成功，LL_ERROR 失败
 */
ll_status_e ll_lin_tx_header(ll_sci_bus_e bus, uint8_t pid);

#if defined(__cplusplus)
}
#endif
#endif /* __TCAE10_LL_SCI_H__ */
