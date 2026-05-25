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
static  l_u16 lin_rcvbreak_cnt = 0;
l_u16 idle_timeout_cnt = 0;

extern const l_u16 lin_max_frame_res_timeout_val[8];
extern l_u8 lin_lld_response_buffer[10];

extern void lin_sci_baudrate_update(void);
extern void lin_goto_idle_state(void);
extern void lin_process_break_handle(void);

void lin_lld_isr_callback(uint32_t isr);

/********************************************************
** \brief   lin_sci_baudrate_update
**
** \param   None
**
** \retval  None
*********************************************************/
__attribute__((weak)) void lin_sci_baudrate_update(void)
{
    //do noting
}

/********************************************************
** \brief   lin_process_break_handle
**
** \param   None
**
** \retval  None
*********************************************************/
__attribute__((weak)) void lin_process_break_handle(void)
{
    //do noting
}

/********************************************************
** \brief   lin_param_init
**
** \param   None
**
** \retval  None
*********************************************************/
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

/********************************************************
** \brief   lin_init_sci
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_init_sci(void)
{
    pal_lin_init(LIN_BUS_0, LIN_MODE_SLV, LIN_BAUD_RATE, lin_lld_isr_callback);
}

/********************************************************
** \brief   lin_lld_sci_int_disable
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_int_disable(void)
{
    pal_lin_deinit(LIN_BUS_0);
}

/********************************************************
** \brief   lin_lld_sci_deinit
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_deinit(void)
{
    state = UNINIT;
    lin_lld_sci_int_disable();
}

/********************************************************
** \brief   lin_lld_sci_tx_wake_up
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_tx_wake_up(void)
{
    if ((state == IDLE) || (state == LIN_SLEEP_MODE))
    {
        /* Set Lin state to idle */
        lin_goto_idle_state();
    }
}

/********************************************************
** \brief   lin_lld_sci_int_enable
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_int_enable(void)
{
}

/********************************************************
** \brief   lin_lld_sci_ignore_response
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_ignore_response(void)
{
    lin_goto_idle_state();
}

/********************************************************
** \brief   lin_lld_sci_set_low_power_mode
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_set_low_power_mode(void)
{
    /* Configure Hw code */
    //lin_sleep_mode_set(DEEPSLEEP_MODE);//ecu & afe sleep
    /* Set Lin status = receiving data*/
    state = LIN_SLEEP_MODE;
    lin_goto_sleep_flg = 1;
}

/********************************************************
** \brief   pal_lin_init
**
** \param   l_u8        msg_length
**
** \retval  None
*********************************************************/
void lin_lld_sci_rx_response(l_u8 msg_length)
{
    /* Put response length and pointer of response buffer into descriptor */
    *(response_buffer) = msg_length;
    cnt_byte = 0;
    ptr = response_buffer;
    pal_lin_rx_response(LIN_BUS_0, pid, &response_buffer[1], msg_length);
    state = RECV_DATA;
}

/********************************************************
** \brief   lin_lld_sci_tx_response
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_tx_response(void)
{
    if (pal_lin_tx_response(LIN_BUS_0, pid,  &response_buffer[1], response_buffer[0]))
    {
        cnt_byte = (response_buffer[0] > 4) ? 4 : response_buffer[0];
        state = SEND_DATA;
    }
    else
    {
        lin_goto_idle_state();
    }
}
#if CFG_SUPPORT_LIN_MASTER
#if defined (__TCPL03X__) || defined(__TCAE10__)
extern void lin_tl_uncd_master_send(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length);
extern void lin_tl_uncd_master_receive(lin_bus_e bus, uint8_t pid, uint8_t *buffer, uint8_t length);
extern void lin1_lld_isr_callback(uint32_t isr);
/********************************************************
** \brief   lin_lld_sci_tx_master
**
** \param   lin_bus_e           bus
** \param   uint8_t             pid
** \param   uint8_t*            buffer
** \param   uint8_t             length
**
** \retval  None
*********************************************************/
void lin_lld_sci_tx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length)
{
    uint8_t pid = lin_process_parity(frame_id, MAKE_PARITY);
    lin_tl_uncd_master_send(LIN_BUS_1, pid, buffer, length);
}

/********************************************************
** \brief   lin_lld_sci_rx_master
**
** \param   lin_bus_e           bus
** \param   uint8_t             pid
** \param   uint8_t*            buffer
** \param   uint8_t             length
**
** \retval  None
*********************************************************/
void lin_lld_sci_rx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length)
{
    uint8_t pid = lin_process_parity(frame_id, MAKE_PARITY);
    lin_tl_uncd_master_receive(LIN_BUS_1, pid, buffer, length);
}

/********************************************************
** \brief   lin_lld_receive_date_get()
**
** \param   uint8_t*                pid
** \param   uint8_t*                buffer
**
** \retval  bool
*********************************************************/
bool lin_lld_sci_rx_data(uint8_t *pid, uint8_t *buffer)
{
    return (lin_tl_uncd_frame_get(LIN_BUS_1, pid, buffer));
}


/********************************************************
** \brief   lin_sci_master_init
**
** \param   None
**
** \retval  None
*********************************************************/
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

/********************************************************
** \brief   lin_lld_sci_get_status
**
** \param   None
**
** \retval  l_u8
*********************************************************/
l_u8 lin_lld_sci_get_status(void)
{
    return l_status.byte;
}

/********************************************************
** \brief   lin_lld_sci_get_state
**
** \param   None
**
** \retval  l_u8
*********************************************************/
l_u8 lin_lld_sci_get_state(void)
{
    return state;
}

/********************************************************
** \brief   lin_lld_sci_timeout
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_timeout(l_u16 cnt_tick)
{
    /* Multi frame support */
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
    if (LD_CHECK_N_CR_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_receive_msg_status = LD_N_CR_TIMEOUT;
            tl_rx_msg_status = LD_N_CR_TIMEOUT;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
            tl_diag_state = LD_DIAG_IDLE;
        }
        else
        {
            tl_check_timeout = ((tl_check_timeout > cnt_tick) ? (tl_check_timeout - cnt_tick) : 0);
        }
    }

    if (LD_CHECK_N_AS_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_tx_msg_status = LD_N_AS_TIMEOUT;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
            tl_diag_state = LD_DIAG_IDLE;
        }
        else
        {
            tl_check_timeout = ((tl_check_timeout > cnt_tick) ? (tl_check_timeout - cnt_tick) : 0);
        }
    }

#else

    /* Single Frame */
    if (LD_CHECK_N_AS_TIMEOUT == tl_check_timeout_type)
    {
        if (0 == tl_check_timeout)
        {
            /* update status of transport layer */
            tl_service_status = LD_SERVICE_ERROR;
            tl_check_timeout_type = LD_NO_CHECK_TIMEOUT;
        }
        else
        {
            tl_check_timeout = ((tl_check_timeout > cnt_tick) ? (tl_check_timeout - cnt_tick) : 0);
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
            idle_timeout_cnt = ((idle_timeout_cnt > cnt_tick) ? (idle_timeout_cnt - cnt_tick) : 0);
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
            frame_timeout_cnt = ((frame_timeout_cnt > cnt_tick) ? (frame_timeout_cnt - cnt_tick) : 0);
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
            res_frame_timeout_cnt = ((res_frame_timeout_cnt > cnt_tick) ? (res_frame_timeout_cnt - cnt_tick) : 0);
        }

        break;

    case PROC_CALLBACK:
        break;

    default:
        break;
    }
}

/********************************************************
** \brief   lin_goto_idle_state
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_goto_idle_state(void)
{
    /* set lin status: ~bus_activity */
    l_status.byte &= ~LIN_STA_BUS_ACTIVITY;
    /* Set max idle timeout */
    idle_timeout_cnt = lin_cfg.max_idle_timeout;
    state = IDLE;
}

/********************************************************
** \brief   lin_lld_sci_err_isr
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_err_isr(void)
{
}

/********************************************************
** \brief   lin_lld_sci_rx_isr
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_sci_rx_isr(void)
{
}

/********************************************************
** \brief   lin_lld_isr_callback
**
** \param   uint32_t               isr
**
** \retval  None
*********************************************************/

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
        ++lin_rcvbreak_cnt;
        //lin_conflict_err =0 ;
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

    /* stop err */
    if (0 != (isr & LIN_INT_STOP_BIT_ERROR_FLAG))
    {
        LOG_LIN("LIN_INT_STOP_BIT_ERROR_FLAG\r\n"); 

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
            lin_error = errSTOP;
            CALLBACK_HANDLER((l_ifc_handle)ifc, LIN_LLD_FRAME_ERR, current_id);
            lin_goto_idle_state();
        }
    }

    /* sync detected */
    if (0 != (isr & LIN_INT_SYNC_DET_FLAG))
    {
        pal_lin_autobaudrate_check(LIN_BUS_0);
        state = RECV_PID;
    }

    /* PID done */
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

#ifdef CFG_LIN_CONFORM_TEST
//        extern uint8_t lin_slave_wakeup_flag;
//        lin_slave_wakeup_flag = 0;
#endif
    }

    /* rx one-byte done */
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

    /* tx done */
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

    /* bit err */
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

#if defined (__TCPL03X__) || defined(__TCAE10__)

    /* 4ytes tx */
    if (0 != (isr & LIN_INT_TX_FIFO_EMPTY_FLAG))
    {
        if (state == SEND_DATA)
        {
            if (cnt_byte <= response_buffer[0])
            {
                if (lin_error != errREADBACK || lin_error != errSTOP)
                {
                    len = ((response_buffer[0] - cnt_byte) <= 4) ? (response_buffer[0] - cnt_byte) : 4;
                    pal_lin_tx_4byte(LIN_BUS_0, &response_buffer[cnt_byte + 1], len);
                    cnt_byte += len;
                }
            }
        }
    }

#endif

    /* sync value err */
    if (0 != (isr & LIN_INT_SYNC_VALUE_ERROR_FLAG))
    {
        lin_error = errSYNC;
        LOG_LIN("sync value err\r\n");
        lin_goto_idle_state();
    }

    /* AFE_INT_RX_PTY_CHK_ERR */
    if (0 != (isr & LIN_INT_RX_CHKPTY_ERROR_FLAG))
    {
        lin_error = errPID;
        LOG_LIN("pid check err\r\n");
    }

#ifndef MULTI_BYTES_MODE

    if (0 != (isr & LIN_INT_RX_FIFO_FULL_FLAG))
    {
        lin_error = errRXFULL;
        LOG_LIN("AFE_INT_RX_FIFO_FULL\r\n");
    }

#endif

    if (0 != (isr & LIN_INT_RX_FIFO_OVF_FLAG))
    {
        lin_error = errRXOVF;
        LOG_LIN("AFE_INT_RX_FIFO_OVF_ERR\r\n");
    }

#if defined (__TCPL01X__)

    if (0 != (isr & LIN_INT_RX_TIMEOUT_FLAG))
    {
        lin_error = errRXTIMEOUT;
        LOG_LIN("AFE_INT_RX_TIMEOUT\r\n");
    }

#endif

#if CFG_SUPPORT_LIN_SNPD

    if (0 != (isr & LIN_INT_AUTOADDR_DONE_FLAG))
    {
        //LOG_LIN("AFE_INT_AUTO_ADDR_DONE\r\n");
    }

    if (0 != (isr & LIN_INT_SLV_SELECT_FLAG))
    {
        lin_snpd_status_set(LIN_AA_STATUS_SELECT, 1);
    }

#endif
}

/********************************************************
** \brief   lin_lld_sci_init
**
** \param   l_ifc_handle    iii
**
** \retval  None
*********************************************************/
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

/********************************************************
** \brief   lin_lld_timer_TCPL_init
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_timer_TCPL_init(void)
{
}

#if defined (__TCPL03X__) || defined(__TCAE10__)
/********************************************************
** \brief   lin_lld_slave_wakeup
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_lld_slave_wakeup(void)
{
    ll_lin_ctrl_brk_tx(LL_SCI_BUS_1, 8);
}
#endif
/********************************************************
** \brief   lin_process_init
**
** \param   None
**
** \retval  None
*********************************************************/
void lin_process_init(void)
{
    l_sys_init();
    l_ifc_init();
    ld_init();
}
l_u16 lin_lld_sci_rcv_break_cnt(void)
{
    return lin_rcvbreak_cnt;
}
