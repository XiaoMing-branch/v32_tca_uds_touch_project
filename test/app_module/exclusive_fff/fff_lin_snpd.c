/**
 *****************************************************************************
 * @brief   lin snpd example source file.
 *
 * @file    lin_snpd.c
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

#include "fff_lin.h"
#include "fff_lin_snpd.h"
#include "fff_store_manager.h"
#include "fff_pal_lin_comm.h"
#include "fff_logging.h"

#define LOG_SPND(...)  do{log_debug("[SNPD] " __VA_ARGS__);}while(0)
#define LIN_SNPD_TIMEOUT   40000

lin_snpd_context_t *lin_snpd_ctx;

#if (SNPD_TEST_MODE == SNPD_TEST_MODE_LIN)
struct aa_adc_data adc_raw_data[AA_SLAVE_NUM];
#endif

DEFINE_FAKE_VOID_FUNC(lin_snpd_raw_adc_out,uint8_t, uint8_t);
DEFINE_FAKE_VALUE_FUNC(uint8_t,lin_snpd_status_get,lin_aa_status_e);
DEFINE_FAKE_VOID_FUNC(lin_snpd_status_set,lin_aa_status_e,uint8_t);
DEFINE_FAKE_VOID_FUNC(lin_snpd_nad_read,uint8_t *);
#ifdef CFG_LIN_CONFORM_TEST
DECLARE_FAKE_VOID_FUNC(lin_snpd_id_read);
#endif
DEFINE_FAKE_VOID_FUNC(lin_snpd_nad_write);
DEFINE_FAKE_VOID_FUNC(lin_snpd_cur_th_get);
DEFINE_FAKE_VOID_FUNC(lin_snpd_cur_th_set);
DEFINE_FAKE_VOID_FUNC(lin_snpd_init);
DEFINE_FAKE_VOID_FUNC(lin_snpd_process_handle);
DEFINE_FAKE_VOID_FUNC(autoaddress_config_for_dfu);