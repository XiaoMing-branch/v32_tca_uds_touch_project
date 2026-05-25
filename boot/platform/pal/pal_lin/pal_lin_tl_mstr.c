/**
 *****************************************************************************
 * @brief   pal lin master transport layer source file.
 *
 * @file    pal_lin_tl_mstr.c
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

#include "pal_lin_comm.h"
#include "pal_lin_tl.h"

#define CFG_LIN_DEBUG   1

#if 1 == CFG_LIN_DEBUG
#include "logging.h"

#define LOG_LIN_TL(...)  do{log_debug("[LIN_TL] " __VA_ARGS__);}while(0)
#else
#define LOG_LIN_TL(...)
#endif

/**
 * @brief lin queue list size
 */
#define LIN_TX_QUEUE_SIZE      (1)
#define LIN_RX_QUEUE_SIZE      (1)


/**
* @brief uds recv frame
*/
extern void lin_callback_handle(lin_event_type_e event, uint8_t frame_id);

#define LIN_CALLBACK_HANDLE(lin_event, pid)  lin_callback_handle((lin_event), (pid))

/*-------------variable statement---------------------*/
static lin_packet_t lin_frame = {0};
static lin_bus_state_e lin_bus_state = LIN_BUS_IDLE;

/* QUEUE information */
lin_tl_data tx_queue_data[LIN_TX_QUEUE_SIZE];    /*transmit queue data */
lin_tl_data rx_queue_data[LIN_RX_QUEUE_SIZE];    /*receive queue data */

/**
* @brief lin_tx_queue
*/
static lin_tl_queue_t lin_tx_queue =
{
    .header = 0,
    .tail = 0,
    .frame_index = 0,
    .tl_data = tx_queue_data,
};

/**
* @brief lin_rx_queue
*/
static lin_tl_queue_t lin_rx_queue =
{
    .header = 0,
    .tail = 0,
    .frame_index = 0,
    .tl_data = rx_queue_data,
};

/**
 * @brief  清空LIN传输层队列
 * @param  queue - 待清空的队列指针
 * @retval 无
 */
void lin_tl_queue_clear(lin_tl_queue_t *queue)
{
    queue->frame_byte = queue->frame_index = queue->header = queue->tail = 0;
}

/**
 * @brief  LIN主机回调处理函数(弱符号，可重写)
 * @param  event - LIN事件类型
 * @param  frame_id - 帧ID
 * @note   处理PID发送完成、接收完成、错误等事件的状态机流转
 * @retval 无
 */
__attribute__((weak)) void lin_callback_handle(lin_event_type_e event, uint8_t frame_id)
{
    lin_tl_queue_t *queue = (LIN_BUS_RECV == lin_bus_state) ? &lin_rx_queue : &lin_tx_queue;

    switch (event)
    {
        case LIN_EVENT_TX_PID_DONE:
            if (LIN_BUS_SEND == lin_bus_state)
            {
                pal_lin_tx_response(LIN_BUS_1, queue->pid, queue->tl_data[queue->tail], 8);
                queue->frame_byte = 4;
            }
            else if (LIN_BUS_RECV == lin_bus_state)
            {
                memset(&lin_frame, 0, sizeof(lin_packet_t));
                pal_lin_rx_response(LIN_BUS_1, queue->pid, queue->tl_data[queue->frame_index], 8);
            }

            break;

        case LIN_EVENT_RX_COMPLETED:
            LOG_LIN_TL("rx data %02x %02x %02x %02x %02x %02x %02x %02x\r\n", lin_frame.buff[0], lin_frame.buff[1], lin_frame.buff[2], lin_frame.buff[3], lin_frame.buff[4], lin_frame.buff[5], lin_frame.buff[6], lin_frame.buff[7]);

            if (lin_rx_queue.tail < LIN_RX_QUEUE_SIZE)
            {
                memcpy(&lin_rx_queue.tl_data[lin_rx_queue.tail++][0], lin_frame.buff, sizeof(lin_frame.buff));
            }
            else
            {
                memcpy(&lin_rx_queue.tl_data[LIN_RX_QUEUE_SIZE - 1][0], lin_frame.buff, sizeof(lin_frame.buff));
            }

            lin_rx_queue.ready = true;

            lin_bus_state = LIN_BUS_IDLE;

            // lin_uds_recv_ready_check(&lin_rx_queue);
            break;

        case LIN_EVENT_TX_COMPLETED:
        case LIN_EVENT_PID_ERR:
        case LIN_EVENT_CHECKSUM_ERR:
        case LIN_EVENT_SYNC_VALUE_ERR:
        case LIN_EVENT_RX_PTY_CHK_ERR:
        case LIN_EVENT_RX_TIMEOUT:
        case LIN_EVENT_TX_RX_CONF:
            lin_bus_state = LIN_BUS_IDLE;
            break;

        default:
            break;
    }
}

/**
 * @brief  LIN传输层初始化(主机模式)
 * @param  无
 * @retval 无
 */
void lin_tl_init(void)
{
    lin_tl_queue_clear(&lin_tx_queue);
    lin_tl_queue_clear(&lin_rx_queue);
}

/**
 * @brief  主机发送无条件帧
 * @param  bus - LIN总线号
 * @param  pid - 帧PID
 * @param  buffer - 待发送数据
 * @param  length - 数据长度
 * @note   清除队列，设置PID并发送帧头
 * @retval 无
 */
void lin_tl_uncd_master_send(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length)
{
    lin_tl_queue_t *queue = &lin_tx_queue;
    lin_tl_queue_clear(queue);
    queue->pid = pid;
    lin_bus_state = LIN_BUS_SEND;
    memcpy(queue->tl_data[queue->tail], buffer, length);
    ll_lin_tx_header((ll_sci_bus_e)bus, pid);
}

/**
 * @brief  主机接收无条件帧
 * @param  bus - LIN总线号
 * @param  pid - 帧PID
 * @param  buffer - 接收数据缓冲区
 * @param  length - 数据长度
 * @note   清除队列，设置PID并发送帧头以请求从机响应
 * @retval 无
 */
void lin_tl_uncd_master_receive(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length)
{
    lin_tl_queue_t *queue = &lin_rx_queue;
    lin_tl_queue_clear(queue);
    queue->pid = pid;
    lin_bus_state = LIN_BUS_RECV;
    ll_lin_tx_header((ll_sci_bus_e)bus, pid);
}

/**
 * @brief  获取接收到的无条件帧数据
 * @param  bus - LIN总线号
 * @param  pid - 输出帧PID
 * @param  buffer - 输出帧数据缓冲区
 * @retval true - 获取成功, false - 无数据
 */
bool lin_tl_uncd_frame_get(lin_bus_e bus, uint8_t *pid, uint8_t *buffer)
{
    lin_tl_queue_t *queue = &lin_rx_queue;

    if (queue->ready)
    {
        *pid = queue->pid;
        memcpy(buffer, queue->tl_data[queue->header], 8);
        queue->ready = false;
        return true;
    }
    return false;
}


/**
 * @brief  LIN主机ISR回调函数
 * @param  isr - 中断状态标志位
 * @note   处理Break/同步/PID接收/字节接收/发送完成/错误等中断事件
 *         根据总线状态机调用对应的LIN_CALLBACK_HANDLE
 * @retval 无
 */
void lin1_lld_isr_callback(uint32_t isr)
{
    uint8_t byte __attribute__((unused));
    uint8_t checksum __attribute__((unused));

    lin_tl_queue_t *queue = (LIN_BUS_RECV == lin_bus_state) ? &lin_rx_queue : &lin_tx_queue;

#if defined (__TCPL01X__)

    /******************************
    *** 1. WAKEUP  DETECTED
    *******************************/
    if (0 != (isr & LIN_INT_WAKEUP_DET_FLAG))
    {
    }

#endif

    /******************************
    *** 1. BREAK DETECTED
    *******************************/
    if (0 != (isr & LIN_INT_BREAK_DET_FLAG))
    {
        // LOG_LIN_TL("AFE_INT0_BRK_DET\r\n");
    }

    /* stop err */
    if (0 != (isr & LIN_INT_STOP_BIT_ERROR_FLAG))
    {
    }

    /* sync detected */
    if (0 != (isr & LIN_INT_SYNC_DET_FLAG))
    {
    }

    /* PID done */
    if (0 != (isr & LIN_INT_RX_PID_DONE_FLAG))
    {
        pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_PID,  &byte);
        queue->pid = byte;

        if (0xFF != pal_lin_parity_calib(LIN_PARITY_CHECK, byte))
        {
            LIN_CALLBACK_HANDLE(LIN_EVENT_PID_OK, queue->pid);
        }
        else
        {
            LIN_CALLBACK_HANDLE(LIN_EVENT_PID_ERR, queue->pid);
        }
    }


    /* PID TX done */
    if (0 != (isr & LIN_INT_TX_PID_DONE_FLAG))
    {

        LIN_CALLBACK_HANDLE(LIN_EVENT_TX_PID_DONE, queue->pid);
    }

    /* rx one-byte done */
    if (0 != (isr & LIN_INT_RX_1BYTE_FLAG))
    {

        if (LIN_BUS_RECV == lin_bus_state)
        {
            pal_lin_read_byte(LIN_BUS_1, LIN_READ_TYPE_FIFO,  &byte);

            if (lin_frame.len < 8)
            {
                lin_frame.buff[lin_frame.len++] = byte;
            }
            else
            {
                if (byte == (checksum = pal_lin_checksum_calib(queue->pid, (uint8_t *)&lin_frame.buff[0])))
                {
                    LIN_CALLBACK_HANDLE(LIN_EVENT_RX_COMPLETED, queue->pid);
                }
                else
                {
                    LOG_LIN_TL("checksum =%02x %02x\r\n", byte, checksum);
                    LIN_CALLBACK_HANDLE(LIN_EVENT_CHECKSUM_ERR, queue->pid);
                }
            }
        }
    }

#if defined (__TCPL01X__)

    if (0 != (isr & LIN_INT_RX_TIMEOUT_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_RX_TIMEOUT, queue->pid);
    }

#endif

    /* AFE_INT_RX_PTY_CHK_ERR */
    if (0 != (isr & LIN_INT_RX_CHKPTY_ERROR_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_RX_PTY_CHK_ERR, queue->pid);
    }

#if defined (__TCPL01X__)

    if (0 != (isr & LIN_INT_RX_TIMEOUT_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_RX_TIMEOUT, queue->pid);
    }

#endif

    /* tx done */
    if (0 != (isr & LIN_INT_TX_DONE_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_TX_COMPLETED, queue->pid);
    }

    /* bit err */
    if (0 != (isr & LIN_INT_TX_RX_CONFLICT_FLAG))
    {
        pal_lin_abort_handle(LIN_BUS_1, LIN_ABORT_TYPE_TX);
        LIN_CALLBACK_HANDLE(LIN_EVENT_TX_RX_CONF, queue->pid);
    }

#if defined (__TCPL03X__)

    /* 4bytes tx */
    if (0 != (isr & LIN_INT_TX_FIFO_EMPTY_FLAG))
    {
        if (LIN_BUS_SEND == lin_bus_state)
        {
            if (queue->frame_byte  == 4)
            {
                pal_lin_tx_4byte(LIN_BUS_1, &queue->tl_data[queue->frame_index][4], 4);
                queue->frame_byte = 0;
                queue->frame_index++;
            }
        }
    }

#endif
}
