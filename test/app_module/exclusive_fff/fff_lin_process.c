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

#include "fff_lin_hw_cfg.h"
#include "fff_lin_cfg.h"
#include "fff_lin_process.h"
#include "fff_lin_wakeup.h"
#include "fff_lin_snpd.h"
#include "fff_logging.h"
#include "fff_pal_lin_comm.h"

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

DEFINE_FAKE_VOID_FUNC(lin_init_sci);
DEFINE_FAKE_VOID_FUNC(lin_param_init);
DEFINE_FAKE_VOID_FUNC(lin_process_init);
#if defined (__TCPL03X__) || defined(__TCAE10__)
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_tx_master,uint8_t , uint8_t *, uint8_t );
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_rx_master,uint8_t , uint8_t *, uint8_t );
DEFINE_FAKE_VALUE_FUNC(bool,lin_lld_sci_rx_data,uint8_t *, uint8_t *);
DEFINE_FAKE_VOID_FUNC(lin_sci_master_init);
#endif
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_isr,uint32_t,uint32_t);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_init,l_ifc_handle);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_deinit);
DEFINE_FAKE_VALUE_FUNC(l_u8,lin_lld_sci_get_status);
DEFINE_FAKE_VALUE_FUNC(l_u8,lin_lld_sci_get_state);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_tx_wake_up);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_int_enable);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_ignore_response);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_int_disable);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_set_low_power_mode);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_tx_response);
DEFINE_FAKE_VOID_FUNC(lin_lld_sci_rx_response);

DEFINE_FAKE_VOID_FUNC(lin_lld_sci_timeout,l_u16);
DEFINE_FAKE_VOID_FUNC(lin_lld_slave_wakeup);
DEFINE_FAKE_VALUE_FUNC(l_u16,lin_lld_sci_rcv_break_cnt);