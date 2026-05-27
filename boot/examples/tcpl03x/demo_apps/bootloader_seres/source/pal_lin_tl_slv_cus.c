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
#include "dfu_uds_manager.h"
#define CFG_LIN_DEBUG 0

#if 1 == CFG_LIN_DEBUG
#include "logging.h"

#define LOG_LIN_TL(...)                     \
    do                                      \
    {                                       \
        log_debug("[LIN_TL] " __VA_ARGS__); \
    } while (0)
#else
#define LOG_LIN_TL(...)
#endif

/**
 * @brief lin queue list size
 */
#ifndef LIN_TX_QUEUE_SIZE
#define LIN_TX_QUEUE_SIZE (3)
#endif
#define LIN_RX_QUEUE_SIZE (87) /**< 接收队列大小，容纳多帧UDS消息 */

/**
 * @brief UDS诊断帧类型定义
 */
#define RECV_FRAM_SF (0x00) /**< 单帧(Single Frame) - PCI类型标识，表示单帧诊断消息 */
#define RECV_FRAM_FF (0x10) /**< 首帧(First Frame) - PCI类型标识，表示多帧诊断消息的第一帧 */
#define RECV_FRAM_CF (0x20) /**< 续帧(Consecutive Frame) - PCI类型标识，表示多帧诊断消息的后续帧 */

extern void lin_callback_handle(lin_event_type_e event, uint8_t pid);
extern void lin_sci_baudrate_update(void);

#define LIN_CALLBACK_HANDLE(lin_event, pid) lin_callback_handle((lin_event), (pid)) /**< LIN事件回调调用宏 */

/*-------------变量声明(variable statement)---------------------*/
static lin_packet_t lin_frame = {0};          /**< LIN接收帧缓存 */
static lin_bus_state_e lin_bus_state = LIN_BUS_IDLE; /**< LIN总线状态机 */
static uint8_t current_rcvd_nad = 0x42;       /**< 当前接收到的节点地址(NAD) */
static uint16_t g_uds_length = 0;             /**< UDS消息总长度 */
/* 队列信息(QUEUE information) */
lin_tl_data tx_queue_data[LIN_TX_QUEUE_SIZE]; /**< 发送队列数据缓冲区 */
lin_tl_data rx_queue_data[LIN_RX_QUEUE_SIZE]; /**< 接收队列数据缓冲区 */

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

static lin_recv_context_t lin_recv_ctx = {0}; /**< LIN接收上下文，用于多帧重组状态跟踪 */

/**
 * @brief  SCI波特率更新(弱符号，可重定义)
 * @note   发送完成后调用，用于动态调整波特率（如LIN自动波特率检测）
 * @param  None
 * @retval None
 */
__attribute__((weak)) void lin_sci_baudrate_update(void)
{
    // do noting
}

/**
 * @brief  清空LIN传输层队列
 * @param[in]  queue 待清空的传输层队列指针
 * @retval None
 */
void lin_tl_queue_clear(lin_tl_queue_t *queue)
{
    queue->frame_index = queue->header = queue->tail = 0;
}


/**
 * @brief  设置UDS目标节点地址(NAD)
 * @param[in]  NAD 节点地址值
 * @retval None
 */
void lin_set_uds_nad(uint8_t NAD)
{
    current_rcvd_nad = NAD;
}

/**
 * @brief  获取当前UDS节点地址(NAD)
 * @param  None
 * @retval 当前节点地址值
 */
uint8_t lin_get_uds_nad(void)
{
    return current_rcvd_nad;
}
/**
 * @brief  UDS接收就绪检查，判断多帧消息是否接收完成
 * @param[in]  queue 接收队列指针
 * @retval None
 */
static void lin_uds_recv_ready_check(lin_tl_queue_t *queue)
{
    uint8_t frame_type;

    /* 提取最后一帧的PCI类型(高4位) */
    frame_type = queue->tl_data[queue->tail - 1][1] & 0xF0;

    if (queue->tail > 1)
    {
        /* 若收到新的单帧/首帧，或NAD地址发生变化，则丢弃旧队列数据，重新开始 */
        if (((RECV_FRAM_SF == frame_type) || (RECV_FRAM_FF == frame_type)) || (queue->tl_data[0][0] != queue->tl_data[queue->tail - 1][0]))
        {
            memcpy(&queue->tl_data[0][0], &queue->tl_data[queue->tail - 1][0], 8);
            queue->frame_index = queue->tail = 1; /* 队列重组：最新帧移至头部，重置索引 */
        }
    }

    if (RECV_FRAM_SF == frame_type)
    {
        /* 单帧：接收立即完成 */
        queue->ready = true;
    }
    else if (RECV_FRAM_FF == frame_type)
    {
        /* 首帧：提取总长度(PCI低4位<<8 | 字节2) */
        g_uds_length = (((queue->tl_data[queue->tail - 1][1] & 0x0F) << 8) | (queue->tl_data[queue->tail - 1][2]));
    } else if(RECV_FRAM_CF == frame_type)
    {
        /* 续帧：检查已接收数据量是否达到总长度(每帧6字节数据) */
        if (((queue->tail * 6 - 1) >= g_uds_length))
        {
            queue->ready = true;
        }
        /* 队列已满但数据未完，缩减剩余长度继续接收 */
        if ((queue->tail == LIN_RX_QUEUE_SIZE) )
        {
            g_uds_length -= 6;
        }
    }
}

/**
 * @brief  LIN回调处理函数(弱符号，可重定义)
 * @note   处理PID完成、接收完成、发送完成及各类错误事件
 * @param[in]  event  LIN事件类型
 * @param[in]  pid    LIN帧ID(含奇偶校验位)
 * @retval None
 */
__attribute__((weak)) void lin_callback_handle(lin_event_type_e event, uint8_t pid)
{
    /* 根据当前总线状态选择对应队列：接收态用rx_queue，否则用tx_queue */
    lin_tl_queue_t *queue = (LIN_BUS_RECV == lin_bus_state) ? &lin_rx_queue : &lin_tx_queue;

    switch (event)
    {
    case LIN_EVENT_PID_OK:
        /* PID 0x7D = 从机响应帧，发送队列中的数据 */
        if (0x7D == pid)
        {
            if (queue->frame_index < queue->tail)
            {
                pal_lin_tx_response(LIN_BUS_0, queue->pid, queue->tl_data[queue->frame_index], 8);
#if defined(__TCPL01X__)
                queue->frame_index++;
#endif
                queue->frame_byte = 4;
                lin_bus_state = LIN_BUS_SEND;
            }
            else
            {
                /* 队列为空，清空并回到空闲 */
                lin_tl_queue_clear(queue);
                lin_bus_state = LIN_BUS_IDLE;
            }
        }
        /* PID 0x3C = 主机请求帧，准备接收响应数据 */
        else if(0x3C == pid)
        {
            pal_lin_rx_response(LIN_BUS_0, queue->pid, lin_frame.buff, sizeof(lin_frame.buff));
            lin_bus_state = LIN_BUS_RECV;
            memset(&lin_frame, 0, sizeof(lin_packet_t));
        }

        break;

    case LIN_EVENT_RX_COMPLETED:
        LOG_LIN_TL("rx data %02x %02x %02x %02x %02x %02x %02x %02x\r\n", lin_frame.buff[0], lin_frame.buff[1], lin_frame.buff[2], lin_frame.buff[3], lin_frame.buff[4], lin_frame.buff[5], lin_frame.buff[6], lin_frame.buff[7]);
        lin_set_uds_nad(lin_frame.buff[0]);
        /* 处理TesterPresent(0x3E 80)心跳报文，仅复位超时计数器，不入队列 */
        if ((lin_frame.buff[0] == LIN_BROADCAST_NAD || lin_frame.buff[0] == LIN_FUNCTION_NAD || lin_frame.buff[0] == lin_configured_NAD) && (lin_frame.buff[1] == 0x02) && (lin_frame.buff[2] == 0x3E) && (lin_frame.buff[3] == 0x80))
        {
            dfu_ctx.uds_timeout = 0; /* 清除诊断会话超时计数 */
            memset(&lin_frame, 0, sizeof(lin_packet_t));
            lin_bus_state = LIN_BUS_IDLE;
            break;
        }
        else
        {
            /* 将接收数据存入rx队列尾 */
            if (lin_rx_queue.tail < LIN_RX_QUEUE_SIZE)
            {
                memcpy(&lin_rx_queue.tl_data[lin_rx_queue.tail++][0], lin_frame.buff, sizeof(lin_frame.buff));
            }
            else
            {
                /* 队列满则覆盖最后一帧 */
                memcpy(&lin_rx_queue.tl_data[LIN_RX_QUEUE_SIZE - 1][0], lin_frame.buff, sizeof(lin_frame.buff));
            }
        }

        memset(&lin_frame, 0, sizeof(lin_packet_t));
        lin_bus_state = LIN_BUS_IDLE;

        /* 检查是否已收到完整的UDS消息 */
        lin_uds_recv_ready_check(&lin_rx_queue);
        break;

    case LIN_EVENT_TX_COMPLETED:
        /* 发送完成：动态调整波特率，恢复总线空闲 */
        lin_sci_baudrate_update();
        lin_bus_state = LIN_BUS_IDLE;
        break;

    case LIN_EVENT_PID_ERR:
    case LIN_EVENT_CHECKSUM_ERR:
    case LIN_EVENT_SYNC_VALUE_ERR:
    case LIN_EVENT_RX_PTY_CHK_ERR:
    case LIN_EVENT_RX_TIMEOUT:
    case LIN_EVENT_TX_RX_CONF:
        /* 各类错误处理：重置总线状态为空闲 */
        lin_bus_state = LIN_BUS_IDLE;
        break;

    default:
        break;
    }
}

/**
 * @brief  LIN传输层初始化
 * @note   清空发送和接收队列，重置传输层状态
 * @param[in]  None
 * @retval None
 */
void lin_tl_init(void)
{
    lin_tl_queue_clear(&lin_tx_queue);
    lin_tl_queue_clear(&lin_rx_queue);
}

/**
 * @brief  UDS诊断消息发送
 * @note   支持单帧(SF, ≤6字节)和多帧(MF, >6字节)发送。
 *         单帧格式: [NAD][PCI=长度][SID][D1..D5]
 *         多帧格式: [NAD][PCI=0x10|长度高4位][长度低8位][SID][D1..D4] + [NAD][PCI=0x20|帧序号][D1..D6]...
 * @param[in]  nad    目标节点地址
 * @param[in]  data   待发送的数据缓冲区
 * @param[in]  length 数据长度(字节)
 * @retval true   入队成功
 * @retval false  数据长度为0，发送失败
 */
bool lin_uds_send(uint8_t nad, uint8_t *data, uint16_t length)
{
    if (!length)
    {
        return false;
    }

    lin_tl_queue_t *queue = &lin_tx_queue;
    lin_tl_queue_clear(queue);
    uint16_t remain_length = length;   /* 剩余待发送字节数 */
    uint8_t frame_index = 1;           /* 续帧帧序号(从1开始) */
    uint8_t send_length = 0;

    if (length <= 6)
    {
        /* 单帧：[NAD][PCI=长度][SID][D1][D2][D3][D4][D5] */
        memset(queue->tl_data[queue->tail], 0xCC, 8);
        queue->tl_data[queue->tail][0] = nad;
        queue->tl_data[queue->tail][1] = length;
        memcpy(&queue->tl_data[queue->tail][2], data, length);
        queue->tail++;
    }
    else
    {
        /* 首帧：[NAD][PCI=0x10|长度高4位][长度低8位][SID][D1][D2][D3][D4] */
        memset(queue->tl_data[queue->tail], 0xCC, 8);
        queue->tl_data[queue->tail][0] = nad;
        queue->tl_data[queue->tail][1] = 0x10 | ((length >> 8) & 0x0F); /* PCI = 0x10 | 长度[11:8] */
        queue->tl_data[queue->tail][2] = length & 0xFF;                  /* 长度[7:0] */
        send_length = 5;                                                  /* 首帧有效数据 = 5字节 */
        memcpy(&queue->tl_data[queue->tail][3], &data[length - remain_length], send_length);

        remain_length -= send_length;

        queue->tail++;

        /* 续帧(CF)：每帧6字节数据，[NAD][PCI=0x20|帧序号][D1..D6] */
        while (remain_length)
        {
            memset(queue->tl_data[queue->tail], 0xCC, 8);
            queue->tl_data[queue->tail][0] = nad;
            queue->tl_data[queue->tail][1] = 0x20 | (frame_index & 0x0F); /* PCI = 0x20 | 帧序号(循环0~15) */
            frame_index++;
            send_length = (remain_length <= 6) ? remain_length : 6;       /* 最后一帧可能不足6字节 */
            memcpy(&queue->tl_data[queue->tail][2], &data[length - remain_length], send_length);
            remain_length -= send_length;
            queue->tail++;
        }
    }

    return true;
}

/**
 * @brief  UDS否定响应发送
 * @note   发送格式: [NAD][PCI=0x03][0x7F(NR)][SID][错误码]
 * @param[in]  nad        目标节点地址
 * @param[in]  sid        被拒绝的服务ID
 * @param[in]  error_code 否定响应错误码
 * @retval true       入队成功
 */
bool lin_uds_negative_response(uint8_t nad, uint8_t sid, uint8_t error_code)
{
    lin_tl_queue_t *queue = &lin_tx_queue;
    lin_tl_queue_clear(queue);

    /* 否定响应帧：[NAD][PCI=0x03][0x7F(NR)][SID][错误码][填充][填充][填充] */
    memset(queue->tl_data[queue->tail], 0xCC, 8);
    queue->tl_data[queue->tail][0] = nad;
    queue->tl_data[queue->tail][1] = 0x03;           /* PCI = 3字节数据长度 */
    queue->tl_data[queue->tail][2] = LIN_RES_NEGATIVE; /* 0x7F = Negative Response */
    queue->tl_data[queue->tail][3] = sid;
    queue->tl_data[queue->tail][4] = error_code;
    queue->tail++;

    return true;
}

/**
 * @brief  UDS诊断消息接收
 * @note   从接收队列中取出完整的UDS消息，支持单帧(SF)和多帧(FF+CF)重组。
 *         单帧直接从队列拷贝数据，多帧需等待所有续帧收齐后重组。
 * @param[in]  nad    期望的节点地址
 * @param[out] data   输出接收数据缓冲区
 * @param[out] length 输出数据实际长度
 * @retval true   接收完成，data/length有效
 * @retval false  无数据或消息尚未接收完整
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

    /* 遍历接收队列中的每一帧 */
    for (uint8_t i = queue->header; i < queue->tail; i++)
    {
        ctx->pci = queue->tl_data[i][1];

        switch (ctx->pci & 0xF0)
        {
        case RECV_FRAM_SF:
            /* ---- 单帧(SF)处理：PCI低4位 = 数据长度，数据从字节2开始 ---- */
            ctx->nad = queue->tl_data[i][0];
            if (ctx->nad == nad || LIN_BROADCAST_NAD == ctx->nad || LIN_FUNCTION_NAD == ctx->nad)
            {
                ctx->total_length = ctx->remain_length = ctx->pci & 0x0F;
                memcpy((uint8_t *)data, (uint8_t *)(&queue->tl_data[queue->header][2]), ctx->remain_length);
                ctx->sid = queue->tl_data[queue->header][2]; /* 提取服务ID */
                ctx->remain_length = 0;
                lin_tl_queue_clear(queue);
                res = true;
            }
            else
            {
                /* NAD不匹配，清空队列 */
                lin_tl_queue_clear(queue);
            }
            break;

        case RECV_FRAM_FF:
            /* ---- 首帧(FF)处理：PCI低4位||字节2 = 总长度，数据从字节3开始(5字节) ---- */
            ctx->nad = queue->tl_data[i][0];

            if (ctx->nad == nad || LIN_BROADCAST_NAD == ctx->nad || LIN_FUNCTION_NAD == ctx->nad)
            {
                ctx->remain_length = ctx->pci & 0x0F;
                ctx->remain_length = (ctx->remain_length << 8) | queue->tl_data[i][2]; /* 总长度 = PCI[3:0]<<8 | byte[2] */
                ctx->total_length = ctx->remain_length;
                memcpy((uint8_t *)data, (uint8_t *)&queue->tl_data[i][3], 5);           /* 首帧数据部分 = 5字节 */
                ctx->sid = queue->tl_data[i][3];                                         /* 提取服务ID */
                ctx->remain_length -= 5;                                                  /* 剩余待接收长度 */
                queue->frame_index = 1;                                                   /* 续帧序号从1开始 */
            }
            else
            {
                lin_tl_queue_clear(queue);
            }

            break;

        case RECV_FRAM_CF:
            /* ---- 续帧(CF)处理：PCI低4位 = 帧序号，每帧6字节数据 ---- */
            if ((queue->tl_data[i][0] == ctx->nad) && ((ctx->pci & 0x0F) == (queue->frame_index & 0x0F)) && queue->frame_index && (ctx->nad == nad || LIN_BROADCAST_NAD == ctx->nad || LIN_FUNCTION_NAD == ctx->nad))
            {
                recv_length = ((ctx->remain_length <= 6) ? ctx->remain_length : 6); /* 最后一帧可能不满6字节 */
                memcpy(&data[ctx->total_length - ctx->remain_length], (uint8_t *)&queue->tl_data[i][2], recv_length);
                queue->frame_index++;
                ctx->remain_length -= recv_length;

                if (!ctx->remain_length)
                {
                    /* 所有数据已收齐 */
                    lin_tl_queue_clear(queue);
                    res = true;
                    LOG_LIN_TL("RECV_FRAM_CF %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
                }
            }
            else if ((queue->tl_data[i][0] == ctx->nad) && ((ctx->pci & 0x0F) != (queue->frame_index & 0x0F)) && queue->frame_index)
            {
                /* 帧序号不匹配：可能是队列回绕导致的序号循环，特殊处理 */
                if (queue->frame_index == (LIN_RX_QUEUE_SIZE -1))
                {
                    res = true;
                }

            } else {
                /* NAD或帧序号无效，清空队列 */
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
 * @brief  LIN底层ISR回调函数
 * @note   处理LIN硬件中断事件，包括Break检测、PID接收/校验、
 *         数据接收/校验和、发送完成及各类错误处理
 * @param[in]  isr   中断状态标志位组合
 * @retval None
 */
void lin_lld_isr_callback(uint32_t isr)
{
    uint8_t byte __attribute__((unused));
    uint8_t checksum __attribute__((unused));
    /* 根据总线状态选择当前工作队列 */
    lin_tl_queue_t *queue = (LIN_BUS_RECV == lin_bus_state) ? &lin_rx_queue : &lin_tx_queue;
#if defined(__TCPL01X__)

    /******************************
    *** 1. 唤醒检测(WAKEUP DETECTED)
    *******************************/
    if (0 != (isr & LIN_INT_WAKEUP_DET_FLAG))
    {
        /* TCPL01X唤醒事件处理(当前为空实现) */
    }

#endif

    /******************************
    *** 2. 间隔场检测(BREAK DETECTED)
    *******************************/
    if (0 != (isr & LIN_INT_BREAK_DET_FLAG))
    {
        // LOG_LIN_TL("AFE_INT0_BRK_DET\r\n");
    }

    /* 停止位错误 */
    if (0 != (isr & LIN_INT_STOP_BIT_ERROR_FLAG))
    {
    }

    /* 同步场检测 */
    if (0 != (isr & LIN_INT_SYNC_DET_FLAG))
    {
    }

    /* PID接收完成 */
    if (0 != (isr & LIN_INT_RX_PID_DONE_FLAG))
    {
        pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_PID, &byte);
        queue->pid = byte;

        /* 校验PID奇偶位，通过则触发PID_OK事件，否则触发PID_ERR事件 */
        if (0xFF != pal_lin_parity_calib(LIN_PARITY_CHECK, byte))
        {
            LIN_CALLBACK_HANDLE(LIN_EVENT_PID_OK, queue->pid);
        }
        else
        {
            LIN_CALLBACK_HANDLE(LIN_EVENT_PID_ERR, queue->pid);
        }
    }

    /* 接收一个字节完成 */
    if (0 != (isr & LIN_INT_RX_1BYTE_FLAG))
    {

        if (LIN_BUS_RECV == lin_bus_state)
        {
            pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_FIFO, &byte);

            if (lin_frame.len < 8)
            {
                /* 未满8字节时持续填充帧缓存 */
                lin_frame.buff[lin_frame.len++] = byte;
            }
            else
            {
                /* 第9字节为校验和，与计算值比对 */
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

    /* 同步场值错误 */
    if (0 != (isr & LIN_INT_SYNC_VALUE_ERROR_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_SYNC_VALUE_ERR, queue->pid);
    }

    /* 接收奇偶校验错误 */
    if (0 != (isr & LIN_INT_RX_CHKPTY_ERROR_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_RX_PTY_CHK_ERR, queue->pid);
    }

#if defined(__TCPL01X__)

    /* 接收超时 */
    if (0 != (isr & LIN_INT_RX_TIMEOUT_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_RX_TIMEOUT, queue->pid);
    }

#endif

    /* 发送完成 */
    if (0 != (isr & LIN_INT_TX_DONE_FLAG))
    {
        LIN_CALLBACK_HANDLE(LIN_EVENT_TX_COMPLETED, queue->pid);
    }

    /* 发送-接收冲突(位错误) */
    if (0 != (isr & LIN_INT_TX_RX_CONFLICT_FLAG))
    {
        pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_TX);
        LIN_CALLBACK_HANDLE(LIN_EVENT_TX_RX_CONF, queue->pid);
    }

#if defined(__TCPL03X__)
    /* TCPL03X特有：TX FIFO空，发送剩余4字节数据 */

    if (0 != (isr & LIN_INT_TX_FIFO_EMPTY_FLAG))
    {
        if (LIN_BUS_SEND == lin_bus_state)
        {
            /* 前4字节已通过pal_lin_tx_response发送，此处发送后4字节 */
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
