/**
 *****************************************************************************
 * @brief   pal lin slave transport layer source file.
 *
 * @file    pal_lin_tl_slv.c
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

#define CFG_LIN_DEBUG   0

#if 1 == CFG_LIN_DEBUG
#include "logging.h"

#define LOG_LIN_TL(...)  do{log_debug("[LIN_TL] " __VA_ARGS__);}while(0)
#else
#define LOG_LIN_TL(...)
#endif

/**
 * @brief lin queue list size
 */
#ifndef LIN_TX_QUEUE_SIZE
#define LIN_TX_QUEUE_SIZE      (3)
#endif
#define LIN_RX_QUEUE_SIZE      (87)

/**
* @brief uds recv frame
*/
#define RECV_FRAM_SF    (0x00)
#define RECV_FRAM_FF    (0x10)
#define RECV_FRAM_CF    (0x20)

extern void lin_callback_handle(lin_event_type_e event, uint8_t pid);
extern void lin_sci_baudrate_update(void);

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

static lin_recv_context_t lin_recv_ctx = {0};

/**
 * @brief  SCI波特率更新(弱符号，可重写)
 * @param  无
 * @retval 无
 */
__attribute__((weak)) void lin_sci_baudrate_update(void)
{
    //do noting
}

/**
 * @brief  清空LIN传输层队列
 * @param  queue - 待清空的队列指针
 * @retval 无
 */
void lin_tl_queue_clear(lin_tl_queue_t *queue)
{
    queue->frame_index = queue->header = queue->tail = 0;
}

/**
 * @brief  UDS接收就绪检查
 * @param  queue - LIN接收队列指针
 * @note   根据单帧(SF)/首帧(FF)判断是否完成接收
 *         对多帧进行队列重组(NAD及帧类型匹配)
 * @retval 无
 */
static void lin_uds_recv_ready_check(lin_tl_queue_t *queue)
{
    uint8_t frame_type;

    frame_type =  queue->tl_data[queue->tail - 1][1] & 0xF0;

    if (queue->tail > 1)
    {
        /* queue nad以及type首帧或者续帧进行队列重组 */
        if (((RECV_FRAM_SF == frame_type) || (RECV_FRAM_FF == frame_type)) || (queue->tl_data[0][0] != queue->tl_data[queue->tail - 1][0]))
        {
            memcpy(&queue->tl_data[0][0], &queue->tl_data[queue->tail - 1][0], 8);
            queue->frame_index = queue->tail = 1;   /* queue 重组 */
        }
    }

    frame_type = queue->tl_data[0][1] & 0xF0;

    if (RECV_FRAM_SF == frame_type)
    {
        queue->ready = true;
    }
    else if (RECV_FRAM_FF == frame_type)
    {

        uint16_t length = (((queue->tl_data[0][1] & 0x0F) << 8) | (queue->tl_data[0][2]));

        if ((queue->tail * 6 - 1) >= length)
        {
            queue->ready = true;
        }
    }
}

/**
 * @brief  LIN从机回调处理函数(弱符号，可重写)
 * @param  event - LIN事件类型
 * @param  pid - 帧PID
 * @note   处理PID_OK(0x7D为发送响应，其他为接收响应)、
 *         接收完成、发送完成、错误等事件
 * @retval 无
 */
__attribute__((weak)) void lin_callback_handle(lin_event_type_e event, uint8_t pid)
{
    lin_tl_queue_t *queue = (LIN_BUS_RECV == lin_bus_state) ? &lin_rx_queue : &lin_tx_queue;

    switch (event)
    {
        case LIN_EVENT_PID_OK:
            if (0x7D == pid)
            {
                if (queue->frame_index < queue->tail)
                {
                    pal_lin_tx_response(LIN_BUS_0, queue->pid, queue->tl_data[queue->frame_index], 8);
#if defined (__TCPL01X__)
                    queue->frame_index++;
#endif
                    queue->frame_byte = 4;
                    lin_bus_state = LIN_BUS_SEND;
                }
                else
                {
                    lin_tl_queue_clear(queue);
                    lin_bus_state = LIN_BUS_IDLE;
                }
            }
            else
            {
                pal_lin_rx_response(LIN_BUS_0, queue->pid, lin_frame.buff, sizeof(lin_frame.buff));
                lin_bus_state = LIN_BUS_RECV;
                memset(&lin_frame, 0, sizeof(lin_packet_t));
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

            memset(&lin_frame, 0, sizeof(lin_packet_t));
            lin_bus_state = LIN_BUS_IDLE;

            lin_uds_recv_ready_check(&lin_rx_queue);
            break;

        case LIN_EVENT_TX_COMPLETED:
            lin_sci_baudrate_update();
            lin_bus_state = LIN_BUS_IDLE;
            break;

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
 * @brief  LIN传输层初始化(从机模式)
 * @param  无
 * @retval 无
 */
void lin_tl_init(void)
{
    lin_tl_queue_clear(&lin_tx_queue);
    lin_tl_queue_clear(&lin_rx_queue);
}

/**
 * @brief  UDS诊断消息发送(单帧或多帧)
 * @param  nad - 节点地址
 * @param  data - 待发送数据
 * @param  length - 数据长度
 * @note   长度≤6使用单帧(SF)，>6使用首帧(FF)+续帧(CF)
 * @retval true - 入队成功, false - 失败
 */
bool lin_uds_send(uint8_t nad, uint8_t *data, uint16_t length)
{
    if (!length)
    {
        return false;
    }

    lin_tl_queue_t *queue = &lin_tx_queue;
    lin_tl_queue_clear(queue);
    uint16_t remain_length = length;
    uint8_t frame_index = 1;
    uint8_t send_length = 0;

    if (length <= 6)
    {
        /*NAD PCI SID D1 D2 D3 D4 D5 */
        memset(queue->tl_data[queue->tail], 0xFF, 8);
        queue->tl_data[queue->tail][0] = nad;
        queue->tl_data[queue->tail][1] = length;
        memcpy(&queue->tl_data[queue->tail][2], data, length);
        queue->tail++;
    }
    else
    {
        /*NAD PCI LEN SID D1 D2 D3 D4 */
        memset(queue->tl_data[queue->tail], 0xFF, 8);
        queue->tl_data[queue->tail][0] = nad;
        queue->tl_data[queue->tail][1] = 0x10 | ((length >> 8) & 0x0F);
        queue->tl_data[queue->tail][2] = length & 0xFF;
        send_length = 5;
        memcpy(&queue->tl_data[queue->tail][3], &data[length - remain_length], send_length);

        remain_length -= send_length;

        queue->tail++;

        while (remain_length)
        {
            /*NAD PCI D1 D2 D3 D4 D5 D6 */
            memset(queue->tl_data[queue->tail], 0xFF, 8);
            queue->tl_data[queue->tail][0] = nad;
            queue->tl_data[queue->tail][1] = 0x20 | (frame_index & 0x0F);
            frame_index++;
            send_length = (remain_length <= 6) ? remain_length : 6;
            memcpy(&queue->tl_data[queue->tail][2], &data[length - remain_length], send_length);
            remain_length -= send_length;
            queue->tail++;
        }
    }

    return true;
}

/**
 * @brief  UDS否定响应(Negative Response)发送
 * @param  nad - 节点地址
 * @param  sid - 服务ID
 * @param  error_code - 错误码
 * @retval true - 入队成功, false - 失败
 */
bool lin_uds_negative_response(uint8_t nad, uint8_t sid, uint8_t error_code)
{
    lin_tl_queue_t *queue = &lin_tx_queue;
    lin_tl_queue_clear(queue);

    /*NAD PCI SID D1 D2 D3 D4 D5 */
    memset(queue->tl_data[queue->tail], 0xFF, 8);
    queue->tl_data[queue->tail][0] = nad;
    queue->tl_data[queue->tail][1] = 0x03;
    queue->tl_data[queue->tail][2] = LIN_RES_NEGATIVE;
    queue->tl_data[queue->tail][3] = sid;
    queue->tl_data[queue->tail][4] = error_code;
    queue->tail++;

    return true;
}

/**
 * @brief  UDS诊断消息接收(单帧/多帧重组)
 * @param  nad - 节点地址
 * @param  data - 接收数据缓冲区
 * @param  length - 输出实际接收长度
 * @note   支持单帧(SF)、首帧(FF)和续帧(CF)的自动重组
 *         检查NAD地址匹配，支持广播地址0x7F
 * @retval true - 接收完成, false - 无数据或未完成
 */
bool lin_uds_receive(uint8_t nad, uint8_t *data, uint16_t *length)
{
    lin_tl_queue_t *queue = &lin_rx_queue;

    if (!queue->ready)
    {
        *length = 0;
        return false;
    }

    queue->ready = false;

    lin_recv_context_t *ctx = &lin_recv_ctx;

    uint8_t recv_length = 0;
    bool res = false;

    for (uint8_t i = queue->header; i < queue->tail; i++)
    {
        ctx->pci = queue->tl_data[i][1];

        switch (ctx->pci & 0xF0)
        {
            case RECV_FRAM_SF:

                ctx->nad = queue->tl_data[i][0];
                ctx->total_length = ctx->remain_length = ctx->pci & 0x0F;
                memcpy((uint8_t *)data, (uint8_t *)(&queue->tl_data[queue->header][2]), ctx->remain_length);
                ctx->sid = queue->tl_data[queue->header][2];
                ctx->remain_length = 0;
                lin_tl_queue_clear(queue);
                res = true;
                break;

            case RECV_FRAM_FF:
                ctx->nad = queue->tl_data[i][0];

                if (ctx->nad == nad || LIN_BROADCAST_NAD == ctx->nad)
                {
                    ctx->remain_length = ctx->pci & 0x0F;
                    ctx->remain_length = (ctx->remain_length << 8) | queue->tl_data[i][2];
                    ctx->total_length = ctx->remain_length;
                    memcpy((uint8_t *)data, (uint8_t *)&queue->tl_data[i][3], 5);
                    ctx->sid = queue->tl_data[i][3];
                    ctx->remain_length -= 5;
                    queue->frame_index = 1;
                }
                else
                {
                    lin_tl_queue_clear(queue);
                }

                break;

            case RECV_FRAM_CF:
                if ((queue->tl_data[i][0] == ctx->nad) && ((ctx->pci & 0x0F) == (queue->frame_index & 0x0F)) && queue->frame_index)
                {
                    recv_length = ((ctx->remain_length <= 6) ? ctx->remain_length : 6);
                    memcpy(&data[ctx->total_length - ctx->remain_length], (uint8_t *)&queue->tl_data[i][2], recv_length);
                    queue->frame_index++;
                    ctx->remain_length -= recv_length;

                    if (!ctx->remain_length)
                    {
                        lin_tl_queue_clear(queue);
                        res = true;
                        LOG_LIN_TL("RECV_FRAM_CF %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
                    }
                }
                else
                {
                    lin_tl_queue_clear(queue);
                }

                break;

            default:
                break;
        }
    }

    *length = res ? ctx->total_length : 0;

    return res;
}

/**
 * @brief  LIN从机ISR回调函数
 * @param  isr - 中断状态标志位
 * @note   处理Break检测/同步检测/PID接收/字节接收/校验和/
 *         发送完成/冲突等中断事件，驱动从机状态机
 * @retval 无
 */
void lin_lld_isr_callback(uint32_t isr)
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

    /* rx one-byte done */
    if (0 != (isr & LIN_INT_RX_1BYTE_FLAG))
    {

        if (LIN_BUS_RECV == lin_bus_state)
        {
            pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_FIFO,  &byte);

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

    /* sync value err */
    if (0 != (isr & LIN_INT_SYNC_VALUE_ERROR_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_SYNC_VALUE_ERR, queue->pid);
    }

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
        pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_TX);
        LIN_CALLBACK_HANDLE(LIN_EVENT_TX_RX_CONF, queue->pid);
    }

#if defined (__TCPL03X__)
    /* 4ytes tx */

    if (0 != (isr & LIN_INT_TX_FIFO_EMPTY_FLAG))
    {
        if (LIN_BUS_SEND == lin_bus_state)
        {
            if (queue->frame_byte == 4)
            {
                pal_lin_tx_4byte(LIN_BUS_0, &queue->tl_data[queue->frame_index][4], 4);
                queue->frame_byte = 0;
                queue->frame_index++;
            }
        }
    }

#endif
}
