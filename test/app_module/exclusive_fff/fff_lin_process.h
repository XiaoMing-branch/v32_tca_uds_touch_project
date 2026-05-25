/**
 *****************************************************************************
 * @brief   lin_process header file.
 *
 * @file    lin_process.h
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

#ifndef __LIN_PROCESS_H__
#define __LIN_PROCESS_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "fff.h"
#include "fff_lin.h"

typedef struct
{
    uint32_t lin_baud_rate;
    uint16_t time_base_period;
    uint16_t max_idle_timeout;
    uint8_t lin_num_of_frms;
    uint8_t  lin_cfg_frame_num;
    uint8_t  diag_number_of_services;
    uint16_t max_queue_size;
    uint16_t n_max_timeout_cnt;
} lin_precfg_t;


DECLARE_FAKE_VOID_FUNC(lin_init_sci);
DECLARE_FAKE_VOID_FUNC(lin_param_init);
DECLARE_FAKE_VOID_FUNC(lin_process_init);
#if defined (__TCPL03X__) || defined(__TCAE10__)
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_tx_master,uint8_t , uint8_t *, uint8_t );
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_rx_master,uint8_t , uint8_t *, uint8_t );
DECLARE_FAKE_VALUE_FUNC(bool,lin_lld_sci_rx_data,uint8_t *, uint8_t *);
DECLARE_FAKE_VOID_FUNC(lin_sci_master_init);
#endif
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_isr,uint32_t,uint32_t);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_init,l_ifc_handle);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_deinit);
DECLARE_FAKE_VALUE_FUNC(l_u8,lin_lld_sci_get_status);
DECLARE_FAKE_VALUE_FUNC(l_u8,lin_lld_sci_get_state);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_tx_wake_up);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_int_enable);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_ignore_response);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_int_disable);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_set_low_power_mode);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_tx_response);
DECLARE_FAKE_VOID_FUNC(lin_lld_sci_rx_response);

DECLARE_FAKE_VOID_FUNC(lin_lld_sci_timeout,l_u16);
DECLARE_FAKE_VOID_FUNC(lin_lld_slave_wakeup);
DECLARE_FAKE_VALUE_FUNC(l_u16,lin_lld_sci_rcv_break_cnt);

// void lin_init_sci(void);
// void lin_param_init(void);
// void lin_process_init(void);
// #if defined (__TCPL03X__) || defined(__TCAE10__)
// void lin_lld_sci_tx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length);
// void lin_lld_sci_rx_master(uint8_t frame_id, uint8_t *buffer, uint8_t length);
// bool lin_lld_sci_rx_data(uint8_t *pid, uint8_t *buffer);
// void lin_sci_master_init(void);
// #endif
// void lin_lld_sci_isr(uint32_t isr0, uint32_t isr1);
// void lin_lld_sci_init(l_ifc_handle iii);
// void lin_lld_sci_deinit(void);
// l_u8 lin_lld_sci_get_status(void);
// l_u8 lin_lld_sci_get_state(void);
// void lin_lld_sci_tx_wake_up(void);
// void lin_lld_sci_int_enable(void);
// void lin_lld_sci_ignore_response(void);
// void lin_lld_sci_int_disable(void);
// void lin_lld_sci_set_low_power_mode(void);
// void lin_lld_sci_tx_response(void);
// void lin_lld_sci_rx_response(l_u8 msg_length);
// void lin_lld_sci_timeout(l_u16 cnt_tick);
// void lin_lld_slave_wakeup(void);
// l_u16 lin_lld_sci_rcv_break_cnt(void);

#define       errNONE       0       /*no    error*/
#define       errSYNC       1       /*sync  error*/
#define       errSTOP       2       /*stop  error*/
#define       errPID        3       /*pid   error*/
#define       errCHECKSUM   4       /*checksum error*/
#define       errRXFULL     5       /*rx fifo full error*/
#define       errRXOVF      6       /*rx fifo ovf  error*/
#define       errRXTIMEOUT  7       /*rx timeout error*/
#define       errREADBACK   8       /*readback   error*/

#define LIN_SCI_INIT_USER    ((LIN_SCI_CTRL_GLB_EN_SET(1))|          (LIN_SCI_CTRL_RX_EN_SET(1))|        (LIN_SCI_CTRL_AUTO_BAUD_EN_SET(1))| \
                              (LIN_SCI_CTRL_CHKSUM_EN_SET(1))|       (LIN_SCI_CTRL_CHKSUM_TYPE_SET(1))|  (LIN_SCI_CTRL_TX_NUM_MODE_SET(1))| \
                              (LIN_SCI_CTRL_RX_NUM_MODE_SET(1))|     (LIN_SCI_CTRL_RX_NUM_SET(8))|       (LIN_SCI_CTRL_TX_NUM_SET(8))| \
                              (LIN_SCI_CTRL_DUTY_CYC_CHK_EN_SET(1))| (LIN_SCI_CTRL_BIT_ERR_DET_EN_SET(1)))
#define LIN_SCI_INIT_BOOT    ((LIN_SCI_CTRL_GLB_EN_SET(1))|          (LIN_SCI_CTRL_RX_EN_SET(1))|        (LIN_SCI_CTRL_AUTO_BAUD_EN_SET(1))| \
                              (LIN_SCI_CTRL_CHKSUM_EN_SET(1))|       (LIN_SCI_CTRL_CHKSUM_TYPE_SET(1))|  (LIN_SCI_CTRL_TX_NUM_MODE_SET(1))| \
                              (LIN_SCI_CTRL_RX_NUM_MODE_SET(1))|     (LIN_SCI_CTRL_RX_NUM_SET(8))|       (LIN_SCI_CTRL_TX_NUM_SET(8))| \
                              (LIN_SCI_CTRL_DUTY_CYC_CHK_EN_SET(1)))

#if defined(__cplusplus)
}
#endif
#endif /*__LIN_PROCESS_H__*/
