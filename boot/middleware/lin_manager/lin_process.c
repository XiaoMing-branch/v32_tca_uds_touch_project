/**
 *****************************************************************************
 * @brief   lin process source file.
 *
 * @file    lin_process.c
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

#include "lin_hw_cfg.h"
#include "lin_cfg.h"
#include "lin_process.h"
#include "lin_wakeup.h"
#include "lin_snpd.h"
#include "logging.h"
#include "pal_lin_comm.h"
#include "pal_lin_tl.h"
#include "pal_gpio.h"

#define MULTI_BYTES_MODE
#undef MULTI_BYTES_MODE
#define RX_BYTE_PRINT_DEBUG
#undef RX_BYTE_PRINT_DEBUG

#define CFG_LIN_DEBUG   0

#if 1 == CFG_LIN_DEBUG
#define LOG_LIN(...)  do{log_debug("[LIN] " __VA_ARGS__);}while(0)
#define LOG_LIN1(format, args ...)  do{log_debug("[LIN] " format, ## args);}while(0)
#else
#define LOG_LIN(format, args ...)
#endif

#define LIN_STA_SUCC_TRANSFER           1         /**< LIN status bit mask: success transfer */
#define LIN_STA_ERROR_RESP              2         /**< LIN status bit mask: error in response */
#define LIN_STA_BUS_ACTIVITY            4         /**< LIN status bit mask: bus activity */
#define LIN_STA_FRAME_ERR               8         /**< LIN status bit mask: frame error */
#define LIN_STA_CHECKSUM_ERR            16        /**< LIN status bit mask: checksum error */
#define LIN_STA_READBACK_ERR            32        /**< LIN status bit mask: readback error */
#define LIN_STA_PARITY_ERR              64        /**< LIN status bit mask: parity error */
#define LIN_STA_RESET                   128       /**< LIN status bit mask: reset */

#define  SLEEP_AFTER_INIT 0

volatile uint8_t lin_error;

lin_precfg_t lin_cfg;

static  l_u8 ifc = 0xFF;
static  l_u8 state = UNINIT;
static  lin_status l_status;
static  l_u8 cnt_byte = 0;
static  l_u8 *ptr = 0;
static  l_u8 current_id = 0x00;
static  l_u8 *response_buffer = 0;
static  l_u8 pid = 0x80;
static  l_u16 frame_timeout_cnt = 0;
static  l_u16 res_frame_timeout_cnt = 0;
l_u16 idle_timeout_cnt = 0;

extern const l_u16 lin_max_frame_res_timeout_val[8];
extern l_u8 lin_lld_response_buffer[10];

extern void lin_sci_baudrate_update(void);
extern void lin_goto_idle_state(void);
extern void lin_process_break_handle(void);

void lin_lld_isr_callback(uint32_t isr);

/**
 * @brief  更新SCI波特率（弱函数，可被覆盖）
 * @param  None
 * @note   Boot版本默认空实现，App可根据需要覆盖。
 *         在每次发送完成后调用以动态调整波特率。
 * @retval None
 */
__attribute__((weak)) void lin_sci_baudrate_update(void)
{
    //do noting
}

/**
 * @brief  LIN BREAK中断处理钩子（弱函数，可被覆盖）
 * @param  None
 * @note   Boot版本默认空实现，检测到BREAK场时调用。
 *         App可覆盖此函数实现自定义BREAK处理逻辑。
 * @retval None
 */
__attribute__((weak)) void lin_process_break_handle(void)
{
    //do noting
}

/**
 * @brief  LIN参数初始化（Boot版本）
 * @param  None
 * @note   从编译时宏配置填充lin_cfg结构体，包括波特率、
 *         时间基数周期、最大空闲超时、帧数量、诊断服务数量、
 *         队列大小和N_CR/N_AS超时计数值。
 * @retval None
 */
void lin_param_init(void)
{
    lin_cfg.lin_baud_rate = LIN_BAUD_RATE;
    lin_cfg.time_base_period = TIME_BASE_PERIOD;
#if SLEEP_AFTER_INIT
    lin_cfg.max_idle_timeout = _MAX_IDLE_TIMEOUT_ / 25;
#else
    lin_cfg.max_idle_timeout = _MAX_IDLE_TIMEOUT_;
#endif
    lin_cfg.lin_num_of_frms = LIN_NUM_OF_FRMS;
    lin_cfg.lin_cfg_frame_num = LIN_CFG_FRAME_NUM;
    lin_cfg.diag_number_of_services = _DIAG_NUMBER_OF_SERVICES_;
    lin_cfg.max_queue_size = MAX_QUEUE_SIZE;
    lin_cfg.n_max_timeout_cnt = ((unsigned short)(1000 * (1000 / TIME_BASE_PERIOD)));
}

/**
 * @brief  LIN硬件SCI初始化
 * @param  None
 * @note   通过PAL层调用pal_lin_init初始化LIN总线0，
 *         配置为从机模式(LIN_MODE_SLV)并注册中断回调函数。
 * @retval None
 */
void lin_init_sci(void)
{
    pal_lin_init(LIN_BUS_0, LIN_MODE_SLV, LIN_BAUD_RATE, lin_lld_isr_callback);
}

/**
 * @brief  禁用LIN中断（反初始化SCI）
 * @param  None
 * @note   调用pal_lin_deinit关闭LIN总线0的硬件和中断。
 *         用于JumpToApp前的硬件清理。
 * @retval None
 */
void lin_lld_sci_int_disable(void)
{
    pal_lin_deinit(LIN_BUS_0);
}

/**
 * @brief  LIN去初始化（JumpToApp前调用）
 * @param  None
 * @note   将状态机设为UNINIT并禁用LIN中断，
 *         用于Boot跳转App前的硬件清理。
 * @retval None
 */
void lin_lld_sci_deinit(void)
{
    state = UNINIT;
    lin_lld_sci_int_disable();
}

/**
 * @brief  LIN唤醒处理（从IDLE/Sleep模式唤醒）
 * @param  None
 * @note   当状态为IDLE或LIN_SLEEP_MODE时，
 *         调用lin_goto_idle_state回到空闲态。
 * @retval None
 */
void lin_lld_sci_tx_wake_up(void)
{
    if ((state == IDLE) || (state == LIN_SLEEP_MODE))
    {
        /* Set Lin state to idle */
        lin_goto_idle_state();
    }
}

/**
 * @brief  启用LIN中断（空函数，Boot版本未使用）
 * @param  None
 * @note   Boot版本中中断已由pal_lin_init自动使能，
 *         此处留空作为占位。
 * @retval None
 */
void lin_lld_sci_int_enable(void)
{
}

/**
 * @brief  忽略/中止当前LIN响应
 * @param  None
 * @note   直接切换到IDLE状态以终止正在进行的响应处理。
 * @retval None
 */
void lin_lld_sci_ignore_response(void)
{
    lin_goto_idle_state();
}

/**
 * @brief  设置LIN进入低功耗（睡眠模式）
 * @param  None
 * @note   将状态设为LIN_SLEEP_MODE并置位休眠标志。
 *         MCU和AFE进入深度睡眠以降低功耗。
 * @retval None
 */
void lin_lld_sci_set_low_power_mode(void)
{
    /* Configure Hw code */
    //lin_sleep_mode_set(DEEPSLEEP_MODE);//ecu & afe sleep
    /* Set Lin status = receiving data*/
    state = LIN_SLEEP_MODE;
    lin_goto_sleep_flg = 1;
}

/**
 * @brief  配置LIN接收响应数据
 * @param  msg_length 期望接收的响应字节数
 * @note   设置响应缓冲区长度指针，重置字节计数，
 *         通过PAL层配置接收描述符，状态切到RECV_DATA。
 * @retval None
 */
void lin_lld_sci_rx_response(l_u8 msg_length)
{
    /* Put response length and pointer of response buffer into descriptor */
    *(response_buffer) = msg_length;
    cnt_byte = 0;
    ptr = response_buffer;
    pal_lin_rx_response(LIN_BUS_0, pid, &response_buffer[1], msg_length);
    state = RECV_DATA;
}

/**
 * @brief  发送LIN响应数据
 * @param  None
 * @note   通过PAL层发送响应缓冲区中的数据，
 *         成功则状态切到SEND_DATA，失败则回到IDLE。
 * @retval None
 */
void lin_lld_sci_tx_response(void)
{
    if (pal_lin_tx_response(LIN_BUS_0, pid,  &response_buffer[1], response_buffer[0]))
    {
        cnt_byte = 4;
        state = SEND_DATA;
    }
    else
    {
        lin_goto_idle_state();
    }
}
#if CFG_SUPPORT_LIN_MASTER
#ifdef  __TCPL03X__
extern void lin_tl_uncd_master_send(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length);
extern void lin_tl_uncd_master_receive(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length);
extern void lin1_lld_isr_callback(uint32_t isr);
/**
 * @brief  主节点发送LIN帧（TCPL03X双LIN模式）
 * @param  frame_id LIN帧ID
 * @param  buffer   发送数据缓冲区
 * @param  length   数据长度
 * @note   计算PID奇偶位后通过LIN总线1以主模式发送。
 *         仅TCPL03X芯片支持独立LIN1主节点功能。
 * @retval None
 */
void lin_lld_sci_tx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length)
{
    uint8_t pid = lin_process_parity(frame_id, MAKE_PARITY);
    lin_tl_uncd_master_send(LIN_BUS_1, pid, buffer, length);
}

/**
 * @brief  主节点接收LIN帧（TCPL03X双LIN模式）
 * @param  frame_id LIN帧ID
 * @param  buffer   接收数据缓冲区
 * @param  length   期望接收长度
 * @note   计算PID奇偶位后通过LIN总线1以主模式接收。
 *         仅TCPL03X芯片支持独立LIN1主节点功能。
 * @retval None
 */
void lin_lld_sci_rx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length)
{
    uint8_t pid = lin_process_parity(frame_id, MAKE_PARITY);
    lin_tl_uncd_master_receive(LIN_BUS_1, pid, buffer, length);
}

/**
 * @brief  获取主节点接收到的LIN帧数据（TCPL03X）
 * @param  pid    返回接收帧的PID
 * @param  buffer 返回接收帧的数据
 * @note   从传输层非确认数据队列中获取一帧数据。
 * @retval bool true=获取成功, false=队列为空
 */
bool lin_lld_sci_rx_data(uint8_t* pid, uint8_t *buffer)
{
    return (lin_tl_uncd_frame_get(LIN_BUS_1, pid, buffer));
}


/**
 * @brief  初始化LIN主节点（TCPL03X双LIN模式）
 * @param  None
 * @note   配置GPIO引脚，初始化LIN总线1为主模式，
 *         注册主节点中断回调。仅TCPL03X芯片支持。
 * @retval None
 */
void lin_sci_master_init(void)
{
    gpio_config_t gpio_cfg =
    {
        .gpio_pin = GPIO_PIN_5,
        .mode = GPIO_MODE_OUT_PP,
        .pull_mode = GPIO_PULL_UP,
        .pull_down_type = GPIO_PULLDOWN_SW_ONLY,

        .afio = AFIO_MUX_0,
        .trigger_flag = GPIO_TRIGGER_NULL,
    };
    pal_gpio_init(&gpio_cfg, NULL);
    pal_gpio_output(GPIO_PIN_5, true);
    gpio_cfg.gpio_pin = GPIO_PIN_2;
    pal_gpio_init(&gpio_cfg, NULL);
    pal_gpio_output(GPIO_PIN_2, true);

    pal_lin_init(LIN_BUS_1, LIN_MODE_MASTER, LIN_BAUD_RATE, lin1_lld_isr_callback);
}
#endif
#endif

/**
 * @brief  获取LIN状态字节
 * @param  None
 * @note   返回当前LIN状态寄存器的字节值，
 *         包含传输成功、错误响应、总线活动等标志位。
 * @retval l_u8 LIN状态字节
 */
l_u8 lin_lld_sci_get_status(void)
{
    return l_status.byte;
}

/**
 * @brief  获取LIN状态机当前状态
 * @param  None
 * @note   返回LIN协议状态机的当前状态值
 *         （UNINIT/IDLE/RECV_SYN/RECV_PID/RECV_DATA/SEND_DATA等）。
 * @retval l_u8 状态机枚举值
 */
l_u8 lin_lld_sci_get_state(void)
{
    return state;
}

/**
 * @brief  LIN协议超时管理
 * @param  None
 * @note   管理三类超时：
 *         1) N_CR超时（多帧传输-连续帧接收超时）
 *         2) N_AS超时（从机响应/服务超时）
 *         3) 帧超时（frame_timeout_cnt/res_frame_timeout_cnt）
 *         IDLE状态下超时进入SLEEP模式，其他状态下回到IDLE。
 * @retval None
 */
void lin_lld_sci_timeout(void)
{
    /* 多帧模式超时检查：N_CR（连续帧接收超时）和N_AS（从机响应超时） */
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
    if (LD_CHECK_N_CR_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == --tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_receive_msg_status = LD_N_CR_TIMEOUT;
            tl_rx_msg_status = LD_N_CR_TIMEOUT;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
            tl_diag_state = LD_DIAG_IDLE;
        }
    }

    if (LD_CHECK_N_AS_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == --tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_tx_msg_status = LD_N_AS_TIMEOUT;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
            tl_diag_state = LD_DIAG_IDLE;
        }
    }

#else

    /* 单帧模式超时检查：N_AS（从机响应超时） */
    if (LD_CHECK_N_AS_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == --tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
        }
    }

#endif /* END (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)    */

    switch (state)
    {
        case IDLE:
            if (idle_timeout_cnt == 0)
            {
                /* Trigger callback */
                CALLBACK_HANDLER(ifc, LIN_LLD_BUS_ACTIVITY_TIMEOUT, 0xFF);
                /* goback to IDLE, reset max idle timeout */
                idle_timeout_cnt = lin_cfg.max_idle_timeout;
                /* Set state to sleep mode */
                state = LIN_SLEEP_MODE;
            }
            else
            {
                idle_timeout_cnt--;
            }

            break;

        case SEND_PID:        /* Master */
        case RECV_SYN:
        case RECV_PID:
        case SEND_DATA:
        case SEND_DATA_COMPLETED:

            /* timeout send has occurred - change state of the node and inform core */
            if (0 == frame_timeout_cnt)
            {
                lin_goto_idle_state();
            }
            else
            {
                frame_timeout_cnt--;
            }

            break;

        case RECV_DATA:

            /* timeout receive has occurred - change state of the node and inform core */
            if (res_frame_timeout_cnt == 0)
            {
                if (cnt_byte > 0)
                {
                    lin_error = errRXTIMEOUT;
                    /* set lin status: error_in_response */
                    l_status.byte |= LIN_STA_ERROR_RESP;
                    /* Trigger callback */
                    CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_NODATA_TIMEOUT, current_id);

                }

                {
                    pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_RX);
                }

                lin_goto_idle_state();
            }
            else
            {
                res_frame_timeout_cnt--;
            }

            break;

        case PROC_CALLBACK:
            break;

        default:
            break;
    }
}

/**
 * @brief  切换到IDLE空闲状态
 * @param  None
 * @note   清除总线活动标志，重置空闲超时计数器，
 *         将状态机设为IDLE。超时或错误恢复时调用。
 * @retval None
 */
void lin_goto_idle_state(void)
{
    /* set lin status: ~bus_activity */
    l_status.byte &= ~LIN_STA_BUS_ACTIVITY;
    /* Set max idle timeout */
    idle_timeout_cnt = lin_cfg.max_idle_timeout;
    state = IDLE;
}

/**
 * @brief  LIN错误中断处理（Boot版本空函数）
 * @param  None
 * @note   Boot版本中错误由主中断回调统一处理。
 * @retval None
 */
void lin_lld_sci_err_isr(void)
{
}

/**
 * @brief  LIN接收中断处理（Boot版本空函数）
 * @param  None
 * @note   Boot版本中接收由主中断回调统一处理。
 * @retval None
 */
void lin_lld_sci_rx_isr(void)
{
}

/**
 * @brief  LIN主中断回调处理函数
 * @param  isr 中断标志位掩码
 * @note   处理所有LIN相关硬件中断事件，包括：唤醒检测、
 *         BREAK场检测、STOP位错误、SYNC场同步、PID接收、
 *         单字节接收（含校验和验证）、发送完成、TX/RX冲突、
 *         多字节发送、SYNC值错误、PID奇偶校验错误、
 *         FIFO溢出等。根据当前状态机状态进行相应处理。
 * @retval None
 */
void lin_lld_isr_callback(uint32_t isr)
{
    uint8_t tmp_byte;
    uint8_t len;
#ifdef MULTI_BYTES_MODE
    uint8_t chk;
#endif

#if defined (__TCPL01X__)

    /******************************
    *** 0. WAKEUP  DETECTED
    *******************************/
    if (0 != (isr & LIN_INT_WAKEUP_DET_FLAG))
    {
#if SLEEP_AFTER_INIT
        max_idle_timeout = _MAX_IDLE_TIMEOUT_;
#endif

        if (LIN_SLEEP_MODE == state)
        {
            lin_goto_idle_state();
        }
    }

#endif

    /******************************
    *** 1. BREAK DETECTED
    *******************************/
    if (0 != (isr & LIN_INT_BREAK_DET_FLAG))
    {
        lin_process_break_handle();
#if SLEEP_AFTER_INIT
        max_idle_timeout = _MAX_IDLE_TIMEOUT_;
#endif
        frame_timeout_cnt = lin_max_frame_res_timeout_val[6];

        if (LIN_SLEEP_MODE == state)
        {
            lin_goto_idle_state();
            return;
        }

        l_status.byte = LIN_STA_BUS_ACTIVITY;
        state = RECV_SYN;
    }

    /* 3. STOP位错误检测：帧结构错误，中止当前收发 */
    if (0 != (isr & LIN_INT_STOP_BIT_ERROR_FLAG))
    {
        LOG_LIN("LIN_INT_STOP_BIT_ERROR_FLAG\r\n");
        lin_error = errSTOP;

        //if (((state == RECV_DATA) || (state == SEND_DATA)) && (tl_slaveresp_cnt != 0))
        if ((state == RECV_DATA) || (state == SEND_DATA) || (state == SEND_DATA_COMPLETED))
        {
            if (state == RECV_DATA)
            {
                //rx_abort
                pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_RX);
            }
            else if (state == SEND_DATA)
            {
                //tx_abort
                pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_TX);
            }

            CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_FRAME_ERR, current_id);
            lin_goto_idle_state();
        }
    }

    /* 4. SYNC场检测完成：执行自动波特率检查，等待PID */
    if (0 != (isr & LIN_INT_SYNC_DET_FLAG))
    {
        pal_lin_autobaudrate_check(LIN_BUS_0);
        state = RECV_PID;
    }

    /* 5. PID接收完成：校验奇偶位，提取帧ID，触发回调 */
    if (0 != (isr & LIN_INT_RX_PID_DONE_FLAG))
    {

        if (state == RECV_PID)
        {

            pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_PID, &tmp_byte);
            /* checkparity and extrait PID */
            current_id = lin_process_parity(tmp_byte, CHECK_PARITY);
            /* Keep the PID */
            pid = tmp_byte;

            if (current_id != 0xFF)
            {
                CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_PID_OK, current_id);

                res_frame_timeout_cnt = lin_max_frame_res_timeout_val[*(response_buffer) - 1];
            }
            else
            {
                lin_error = errPID;
                l_status.byte |= LIN_STA_PARITY_ERR;
                /* trigger callback */
                CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_PID_ERR, 0xFF);
                lin_goto_idle_state();
            }
        }

    /* 一致性测试：收到PID后清除唤醒标志 */
#ifdef CFG_LIN_CONFORM_TEST
        extern uint8_t lin_slave_wakeup_flag;
        lin_slave_wakeup_flag = 0;
#endif
    }

    /* 6. 单字节接收完成：存储数据，接收完成时校验和验证 */
    if (0 != (isr & LIN_INT_RX_1BYTE_FLAG))
    {
        pal_lin_read_byte(LIN_BUS_0, LIN_READ_TYPE_FIFO, &tmp_byte);

        switch (state)
        {
            case RECV_DATA:
                ptr++;
                *(ptr) = tmp_byte;

                /* Check bytes received fully */
#ifdef RX_BYTE_PRINT_DEBUG
                LOG_LIN(" %02x", tmp_byte);
#endif

                if (cnt_byte == (response_buffer[0]))
                {
#ifdef RX_BYTE_PRINT_DEBUG
                    LOG_LIN("\r\n");
#endif
                    /* rx_abort */
                    pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_RX);

                    /* checksum checking */
                    if (lin_checksum(response_buffer, pid) == tmp_byte)
                    {
                        /*******************************************/
                        /***  RX Buffer Full - Checksum OK       ***/
                        /*******************************************/
                        /* set lin status: successful_transfer */
                        l_status.byte |= LIN_STA_SUCC_TRANSFER;
                        /* disable RX interrupt */
                        state = PROC_CALLBACK;
                        /* trigger callback */
                        CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_RX_COMPLETED, current_id);

                        /* enable RX interrupt */
                        if (LIN_SLEEP_MODE != state)
                        {
                            lin_goto_idle_state();
                        }
                    }
                    else
                    {
                        lin_error = errCHECKSUM;
                        /* set lin status: error_in_response, checksum_error */
                        l_status.byte |= (LIN_STA_ERROR_RESP | LIN_STA_CHECKSUM_ERR);
                        LOG_LIN("err no = %x\r\n", l_status.byte);
                        /* trigger callback */
                        CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_CHECKSUM_ERR, current_id);
                        lin_goto_idle_state();
                    }

                }

                cnt_byte++;
                break;

            case LIN_SLEEP_MODE:
                if ((tmp_byte == 0xF0) || (tmp_byte == 0xE0) || (tmp_byte == 0xC0) || (tmp_byte == 0x80) || (tmp_byte == 0x00))
                {
                    /* Set idle timeout again */
                    lin_goto_idle_state();
                }

                break;

            default:
                break;
        }
    }

    /* 7. 发送完成：置位成功标志，触发TX完成回调 */
    if (0 != (isr & LIN_INT_TX_DONE_FLAG))
    {
        if (state == SEND_DATA)
        {
            l_status.byte |= LIN_STA_SUCC_TRANSFER;
            state = PROC_CALLBACK;
            CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_TX_COMPLETED, current_id);
            lin_sci_baudrate_update();
            lin_goto_idle_state();
        }
    }

    /* 8. TX/RX冲突（位错误）：回读校验失败，中止发送 */
    if (0 != (isr & LIN_INT_TX_RX_CONFLICT_FLAG))
    {
        if (state == SEND_DATA)
        {
            lin_error = errREADBACK;
            LOG_LIN("LIN_INT_TX_RX_CONFLICT_FLAG\r\n");

            /* tx_abort */
            pal_lin_abort_handle(LIN_BUS_0, LIN_ABORT_TYPE_TX);
            CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_READBACK_ERR, current_id);
            lin_goto_idle_state();
        }
    }

#if defined (__TCPL03X__)

    /* 9. 多字节发送（TCPL03X）：FIFO空时连续写入4字节 */
    if (0 != (isr & LIN_INT_TX_FIFO_EMPTY_FLAG))
    {
        if (state == SEND_DATA)
        {
            if (cnt_byte <= response_buffer[0])
            {
                if (lin_error != errREADBACK || lin_error != errSTOP)
                {
                    len = ((8 - cnt_byte) <= 4) ? (8 - cnt_byte) : 4;
                    pal_lin_tx_4byte(LIN_BUS_0, &response_buffer[cnt_byte + 1], len);
                    cnt_byte += len;
                }
            }
        }
    }

#endif

    /* 10. SYNC字节值错误：同步场值不匹配0x55 */
    if (0 != (isr & LIN_INT_SYNC_VALUE_ERROR_FLAG))
    {
        lin_error = errSYNC;
        LOG_LIN("sync value err\r\n");
        lin_goto_idle_state();
    }

    /* 11. PID奇偶校验错误：PID奇偶位不匹配 */
    if (0 != (isr & LIN_INT_RX_CHKPTY_ERROR_FLAG))
    {
        lin_error = errPID;
        LOG_LIN("pid check err\r\n");
    }

    /* 12. RX FIFO满标志（非多字节模式） */
#ifndef MULTI_BYTES_MODE

    if (0 != (isr & LIN_INT_RX_FIFO_FULL_FLAG))
    {
        lin_error = errRXFULL;
        LOG_LIN("AFE_INT_RX_FIFO_FULL\r\n");
    }

#endif

    /* 13. RX FIFO溢出错误 */
    if (0 != (isr & LIN_INT_RX_FIFO_OVF_FLAG))
    {
        lin_error = errRXOVF;
        LOG_LIN("AFE_INT_RX_FIFO_OVF_ERR\r\n");
    }

#if defined (__TCPL01X__)

    /* 14. RX接收超时（TCPL01X） */
    if (0 != (isr & LIN_INT_RX_TIMEOUT_FLAG))
    {
        lin_error = errRXTIMEOUT;
        LOG_LIN("AFE_INT_RX_TIMEOUT\r\n");
    }

#endif

    /* 15. SNPD自动寻址完成（CFG_SUPPORT_LIN_SNPD） */
#if CFG_SUPPORT_LIN_SNPD

    if (0 != (isr & LIN_INT_AUTOADDR_DONE_FLAG))
    {
        //LOG_LIN("AFE_INT_AUTO_ADDR_DONE\r\n");
    }

    /* 16. SNPD从节点选中 */
    if (0 != (isr & LIN_INT_SLV_SELECT_FLAG))
    {
        lin_snpd_status_set(LIN_AA_STATUS_SELECT, 1);
    }

#endif
}

/**
 * @brief  LIN SCI初始化（Boot版本）
 * @param  iii LIN接口句柄
 * @note   保存接口句柄，设置响应缓冲区指针，
 *         依次调用参数初始化和SCI硬件初始化，最终进入IDLE态。
 * @retval None
 */
void lin_lld_sci_init(l_ifc_handle iii)
{
    (void)ifc;

    ifc = (l_u8)iii;
    response_buffer = lin_lld_response_buffer;
    lin_param_init();
    lin_init_sci();

    /* Enter IDLE state */
    lin_goto_idle_state();
}

/**
 * @brief  定时器初始化（Boot版本空函数）
 * @param  None
 * @note   Boot版本定时器由PAL层管理，此处留空。
 * @retval None
 */
void lin_lld_timer_TCPL_init(void)
{
}

#ifdef  __TCPL03X__
/**
 * @brief  从节点发送唤醒脉冲
 * @param  None
 * @note   通过LIN总线1发送8位宽度的BREAK信号唤醒主节点。
 *         仅TCPL03X芯片支持独立LIN1总线唤醒。
 * @retval None
 */
void lin_lld_slave_wakeup(void)
{
    ll_lin_ctrl_brk_tx(LL_SCI_BUS_1, 8);
}
#endif

/**
 * @brief  LIN处理模块初始化链
 * @param  None
 * @note   依次初始化：系统层(l_sys_init)、接口层(l_ifc_init)、
 *         传输层(ld_init)。完成整个LIN协议栈的启动。
 * @retval None
 */
void lin_process_init(void)
{
    l_sys_init();
    l_ifc_init();
    ld_init();
}
