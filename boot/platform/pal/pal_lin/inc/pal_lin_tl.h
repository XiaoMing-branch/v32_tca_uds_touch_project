/**
 *****************************************************************************
 * @brief   pal lin tl header file.
 *
 * @file    pal_lin_tl.h
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

#ifndef __PAL_LIN_TL_H__
#define __PAL_LIN_TL_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif


#define POSITIVE 1
#define NEGATIVE 0

#define LIN_BROADCAST_NAD                     0x7Fu   /**< NAD */
#define LIN_FUNCTION_NAD                      0x7Eu
#define LIN_RES_NEGATIVE                      0x7Fu      /**< negative response */

/**
 * @brief lin negative response
 */
#define RES_NEGATIVE                                0x7Fu      /* negative response */
#define GENERAL_REJECT                              0x10u      /*Error code raised when request for service not supported comes  */
#define SERVICE_NOT_SUPPORTED                       0x11u      /*not supported service */
#define SUBFUNCTION_NOT_SUPPORTED                   0x12u      /*not supported subfunction  */
#define IMLOIF                                      0x13u      /*incorrect Message LengtOr InvalidFormat*/
#define RESPONSE_TOO_LONG                           0x14u      /*responseTooLong*/
#define BUSY_REPEAT_REQUEST                         0x21u      /*busyRepeatRequest*/
#define CONDITION_NOT_CORRECT                       0x22u      /*conditionsNotCorrect*/
#define REQUEST_SEQUEENCE_ERROR                     0x24u      /*requestSequenceError*/
#define NRFSC                                       0x25u      /*no Response From Subnet Component*/
#define FPEORA                                      0x26u      /*Failure Prevents Execution Of Requested Action*/
#define REQUEST_OUT_RANGE                           0x31u      /*request Out Of Range*/
#define SECURITY_ACCESS_DENIED                      0x33u      /*security Access Denied*/
#define INVALID_KEY                                 0x35u      /*invalid Key*/
#define ENOA                                        0x36u      /*exceed Number Of Attempts*/
#define REQUIREDTIMEDELAY_NOTEXPIRED                0x37u     
#define DOWNLOAD_REJECTED                           0x70u      /*Upload/download request has been rejected*/
#define TRANSFER_DATA_PAUSE                         0x71u      /*Data transmission is paused*/
#define RCRRP                                       0x78u      /*request Correctly Received-Response Pending*/
#define GENERAL_PROGRAM_FAILURE                     0x72u      /*general Programming Failure*/
#define BLOCK_SEQUENCE_COUNT_ERR                    0x73u      /*block sequence counter error*/
#define SUBFUNCTION_NOTSUPPORTED_INACTIVESESSION    0x7Eu
#define SERVICENOTSUPPORTED_INACTIVESESSION         0x7Fu

/*-------------enum and struct---------------------------*/
/**
 * @brief  LIN事件类型枚举
 * @note   定义LIN传输层内部事件类型
 */
typedef enum
{
    LIN_EVENT_PID_OK,               /**< PID校验正确 */
    LIN_EVENT_TX_COMPLETED,         /**< 发送完成 */
    LIN_EVENT_RX_COMPLETED,         /**< 接收完成 */
    LIN_EVENT_PID_ERR,              /**< PID校验错误 */
    LIN_EVENT_CHECKSUM_ERR,         /**< 校验和错误 */

    LIN_EVENT_SYNC_VALUE_ERR,       /**< 同步场值错误 */
    LIN_EVENT_RX_PTY_CHK_ERR,       /**< 接收奇偶校验错误 */
    LIN_EVENT_RX_TIMEOUT,           /**< 接收超时 */
    LIN_EVENT_TX_RX_CONF,           /**< 发送接收冲突 */
    LIN_EVENT_TX_PID_DONE,          /**< PID发送完成 */
    LIN_EVENT_RX_BYTE,              /**< 接收到一个字节 */
} lin_event_type_e;

/**
 * @brief  LIN总线状态枚举
 * @note   定义传输层总线状态机
 */
typedef enum
{
    LIN_BUS_IDLE           = 0,
    LIN_BUS_SEND          = 1,
    LIN_BUS_RECV,
    LIN_BUS_ERROR,
} lin_bus_state_e;

/**
 * @brief  LIN传输层数据(8字节)
 */
typedef uint8_t lin_tl_data[8];

/**
 * @brief  LIN数据包结构体
 * @note   包含帧ID、长度和8字节数据
 */
typedef struct
{
    uint8_t id;
    uint8_t len;
    uint8_t buff[8];
} lin_packet_t;

/**
 * @brief  LIN传输层队列结构体
 * @note   用于管理LIN数据帧的发送/接收环形队列
 */
typedef struct
{
    bool            ready;
    uint8_t         header;           /**< 队列头指针 */
    uint8_t         tail;             /**< 队列尾指针 */
    uint16_t        frame_index;      /**< 帧序号 */
    uint8_t         frame_byte;       /**< 帧内字节计数 */
    uint8_t         pid;              /**< 当前帧PID */
    lin_tl_data     *tl_data;         /**< LIN传输层数据指针 */
} lin_tl_queue_t;

/**
 * @brief  LIN接收上下文结构体
 * @note   用于UDS多帧接收的状态管理
 */
typedef struct
{
    uint8_t         nad;
    uint8_t         pci;
    uint8_t         sid;
    uint16_t        remain_length;
    uint16_t        total_length;
    uint8_t         frame_index;
} lin_recv_context_t;

/*-------------Function port--------------------------*/
/**
 * @brief  LIN传输层初始化
 * @note   清空发送和接收队列
 */
void lin_tl_init(void);

/**
 * @brief  UDS诊断消息发送
 * @param  nad - 节点地址
 * @param  data - 待发送数据
 * @param  length - 数据长度
 * @retval true - 入队成功, false - 失败
 */
bool lin_uds_send(uint8_t nad, uint8_t *data, uint16_t length);

/**
 * @brief  UDS否定响应发送
 * @param  nad - 节点地址
 * @param  sid - 服务ID
 * @param  error_code - 错误码
 * @retval true - 入队成功, false - 失败
 */
bool lin_uds_negative_response(uint8_t nad, uint8_t sid, uint8_t error_code);

/**
 * @brief  UDS诊断消息接收
 * @param  nad - 节点地址
 * @param  data - 接收数据缓冲区
 * @param  length - 输出接收长度
 * @retval true - 接收完成, false - 无数据或未完成
 */
bool lin_uds_receive(uint8_t nad, uint8_t *data, uint16_t *length);

/**
 * @brief  获取无条件帧数据
 * @param  bus - LIN总线号
 * @param  pid - 输出帧PID
 * @param  buffer - 输出帧数据缓冲区
 * @retval true - 有数据, false - 无数据
 */
bool lin_tl_uncd_frame_get(lin_bus_e bus, uint8_t *pid, uint8_t *buffer);
#ifdef __cplusplus
}
#endif
#endif
