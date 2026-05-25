/**
 *****************************************************************************
 * @brief   pal lin communication header file.
 *
 * @file    pal_lin_comm.h
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

#ifndef __PAL_LIN_COMM_H__
#define __PAL_LIN_COMM_H__

#include "pal_func_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief LIN INT FLAG
 */
#define LIN_INT_RX_1BYTE_FLAG           (SCI_INT_RX_1BYTE_DONE >> SCI_INT_OFFSET)
#define LIN_INT_RX_DONE_FLAG            (SCI_INT_RX_DONE >> SCI_INT_OFFSET)
#define LIN_INT_RX_PID_DONE_FLAG        (SCI_INT_RX_PID_DONE >> SCI_INT_OFFSET)
#define LIN_INT_RX_CHKPTY_ERROR_FLAG    (SCI_INT_RX_PTY_CHK_ERR >> SCI_INT_OFFSET)
#define LIN_INT_RX_CHKSUM_ERROR_FLAG    (SCI_INT_RX_CHKSUM_ERR >> SCI_INT_OFFSET)
#define LIN_INT_STOP_BIT_ERROR_FLAG     (SCI_INT_RX_STP_ERR >> SCI_INT_OFFSET)
#define LIN_INT_RX_FIFO_FULL_FLAG       (SCI_INT_RX_FIFO_FULL >> SCI_INT_OFFSET)
#define LIN_INT_RX_FIFO_OVF_FLAG        (SCI_INT_RX_FIFO_OVF_ERR >> SCI_INT_OFFSET)
#define LIN_INT_TX_DONE_FLAG            (SCI_INT_TX_DONE >> SCI_INT_OFFSET)
#define LIN_INT_TX_FIFO_EMPTY_FLAG      (SCI_INT_TX_FIFO_EMPTY >> SCI_INT_OFFSET)

#define LIN_INT_BREAK_DET_FLAG          (SCI_INT_BRK_DET >> SCI_INT_OFFSET)
#define LIN_INT_SYNC_DET_FLAG           (SCI_INT_SYNC_DET >> SCI_INT_OFFSET)
#define LIN_INT_SYNC_VALUE_ERROR_FLAG   (SCI_INT_SYNC_VAL_ERR >> SCI_INT_OFFSET)
#define LIN_INT_TX_PID_DONE_FLAG        (SCI_INT_TX_PID_DONE >> SCI_INT_OFFSET)
#define LIN_INT_TX_RX_CONFLICT_FLAG     (SCI_INT_TX_RX_CONF >> SCI_INT_OFFSET)
#define LIN_INT_SLV_SELECT_FLAG         (SCI_INT_SLV_SELECTED >> SCI_INT_OFFSET)
#define LIN_INT_AUTOADDR_DONE_FLAG      (SCI_INT_AUTO_ADDR_DONE >> SCI_INT_OFFSET)
#define LIN_INT_TX_1BYTE_FLAG           (SCI_INT_TX_1BYTE_DONE >> SCI_INT_OFFSET)

#if defined (__TCPL01X__)
#define LIN_INT_RX_TIMEOUT_FLAG         (SCI_INT_RX_TIMEOUT >> SCI_INT_OFFSET)
#define LIN_INT_SLV_TX_BREAK_DONE_FLAG  (SCI_INT_SLV_TX_BRK_DONE << SCI_INT_OFFSET1)
#define LIN_INT_MP_MODE_ADDR_FLAG       (SCI_INT_MP_MODE_ADDR << SCI_INT_OFFSET1)
#define LIN_INT_TX_COL_DET_ERROR_FLAG   (SCI_INT_TX_COL_DET_ERR >> SCI_INT_OFFSET)
#define LIN_INT_IDLE_DET_FLAG           (SCI_INT_BUS_IDLE_DET << SCI_INT_OFFSET1) //bit=20
#define LIN_INT_WAKEUP_DET_FLAG         (SCI_INT_WAKE_UP_DET << SCI_INT_OFFSET1)
#define LIN_INT_AUTOADDR_START_FLAG     (SCI_INT_AUTO_ADDR_START << SCI_INT_OFFSET1)
#define LIN_INT_AUTOADDR_1BIT_FLAG      (SCI_INT_ADDR_1BIT << SCI_INT_OFFSET1)
#elif defined (__TCPL03X__) || defined(__TCAE10__)
#define LIN_INT_SLV_TX_BREAK_DONE_FLAG  (SCI_INT_SLV_TX_BRK_DONE >> SCI_INT_OFFSET)
#define LIN_INT_MP_MODE_ADDR_FLAG       (SCI_INT_MP_MODE_ADDR >> SCI_INT_OFFSET)
#define LIN_INT_SHORT_TO_GND_FLAG       (SCI_INT_SHORT_TO_GND >> SCI_INT_OFFSET)
#define LIN_INT_RX_VALID_FLAG           (SCI_INT_RX_VALID >> SCI_INT_OFFSET)
#endif

/**
  * @brief  LIN总线编号枚举
  */
typedef enum
{
    LIN_BUS_0 = 1,
#if defined (__TCPL03X__) || defined(__TCAE10__)
    LIN_BUS_1,
#endif
    LIN_BUS_MAX,
} lin_bus_e;

/**
  * @brief  LIN工作模式枚举（从机/主机）
  */
typedef enum
{
    LIN_MODE_SLV = 0,
    LIN_MODE_MASTER,
    LIN_MODE_MAX,
} lin_mode_e;

/**
  * @brief  LIN奇偶校验类型枚举
  */
typedef enum
{
    LIN_PARITY_MAKE,    /**< 生成奇偶校验位 */
    LIN_PARITY_CHECK,   /**< 校验奇偶位 */
} lin_parity_type_e;

/**
  * @brief  LIN中止类型枚举（位标志，支持组合）
  */
typedef enum
{
    LIN_ABORT_TYPE_NULL = (0),
    LIN_ABORT_TYPE_TX = (0x01 << 0),
    LIN_ABORT_TYPE_RX = (0x01 << 1),
} lin_abort_type_e;

/**
  * @brief  LIN读取类型枚举
  */
typedef enum
{
    LIN_READ_TYPE_PID,
    LIN_READ_TYPE_FIFO,
} lin_read_type_e;

/**
 * @brief  LIN通信初始化
 * @param  bus      - LIN总线编号
 * @param  mode     - 工作模式（主/从）
 * @param  baudrate - 波特率
 * @param  callback - ISR回调函数
 */
void pal_lin_init(lin_bus_e bus, lin_mode_e mode, uint32_t baudrate, ISR_FUNC_CALLBACK callback);
/**
 * @brief  LIN通信去初始化
 * @param  bus - LIN总线编号
 */
void pal_lin_deinit(lin_bus_e bus);
/**
 * @brief  接收LIN从机响应
 * @param  bus        - LIN总线编号
 * @param  pid        - 报文ID
 * @param  buffer     - 接收缓冲区
 * @param  msg_length - 数据长度
 */
void pal_lin_rx_response(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t msg_length);
/**
 * @brief  发送LIN从机响应
 * @param  bus        - LIN总线编号
 * @param  pid        - 报文ID
 * @param  buffer     - 发送缓冲区
 * @param  msg_length - 数据长度
 * @retval true  - 成功
 * @retval false - 失败
 */
bool pal_lin_tx_response(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t msg_length);
/**
 * @brief  发送4字节短报文
 * @param  bus        - LIN总线编号
 * @param  buffer     - 发送缓冲区
 * @param  msg_length - 数据长度
 */
void pal_lin_tx_4byte(lin_bus_e bus, uint8_t *buffer, uint8_t msg_length);
/**
 * @brief  发送LIN报文头（同步间隔+同步场+PID）
 * @param  bus - LIN总线编号
 * @param  pid - 报文ID
 */
void pal_lin_tx_header(lin_bus_e bus, uint8_t pid);
/**
 * @brief  进入LIN自动寻址模式
 * @param  aa_cur_th - 电流阈值数组
 */
void pal_lin_aa_enter(uint16_t *aa_cur_th);
/**
 * @brief  退出LIN自动寻址模式
 */
void pal_lin_aa_exit(void);
/**
 * @brief  标记自动寻址完成
 */
void pal_lin_aa_ready(void);
/**
 * @brief  计算/校验LIN PID奇偶位
 * @param  type - 奇偶类型（生成/校验）
 * @param  pid  - PID字节
 * @retval 处理后的PID值
 */
uint8_t pal_lin_parity_calib(lin_parity_type_e type, uint8_t pid);
/**
 * @brief  计算LIN报文校验和
 * @param  pid    - 报文ID
 * @param  buffer - 数据缓冲区
 * @retval 校验和值
 */
uint8_t pal_lin_checksum_calib(uint8_t pid, uint8_t *buffer);
/**
 * @brief  读取自动寻址ADC原始码
 * @param  bufffer - 原始码缓冲区
 * @param  length  - 缓冲区长度
 * @retval 读取的原始码数量
 */
uint16_t pal_lin_aa_raw_code_get(uint16_t *bufffer, uint16_t length);
/**
 * @brief  中止LIN收发操作
 * @param  bus  - LIN总线编号
 * @param  type - 中止类型
 */
void pal_lin_abort_handle(lin_bus_e bus, lin_abort_type_e type);
/**
 * @brief  读取LIN接收字节
 * @param  bus  - LIN总线编号
 * @param  type - 读取类型
 * @param  byte - 输出字节
 */
void pal_lin_read_byte(lin_bus_e bus, lin_read_type_e type, uint8_t *byte);
/**
 * @brief  自动波特率检测与纠偏
 * @param  bus - LIN总线编号
 */
void pal_lin_autobaudrate_check(lin_bus_e bus);

#ifdef __cplusplus
}
#endif
#endif /*__PAL_LIN_COMM_H__*/
