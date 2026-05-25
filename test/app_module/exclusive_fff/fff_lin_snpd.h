/**
 *****************************************************************************
 * @brief   LIN SNPD header file.
 *
 * @file    lin_snpd.h
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

#ifndef __LIN_SNPD_H__
#define __LIN_SNPD_H__

#include <stdint.h>
#include "fff.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define SNPD_TEST_MODE_NONE     0
#define SNPD_TEST_MODE_PRINT    1
#define SNPD_TEST_MODE_LIN      2

#define SNPD_TEST_MODE SNPD_TEST_MODE_NONE

#if SNPD_TEST_MODE == SNPD_TEST_MODE_LIN
#define AA_SLAVE_NUM            15
#endif

#define SET_ORGINAL_NAD         1

#define LIN_SNPD_CMD_ENTER      0x01
#define LIN_SNPD_CMD_NAD        0x02
#define LIN_SNPD_CMD_SAVE       0x03
#define LIN_SNPD_CMD_EXIT       0x04

#define LIN_AA_STATE_IDLE       0
#define LIN_AA_STATE_ENTER      1
#define LIN_AA_STATE_SAVE       2
#define LIN_AA_STATE_EXIT       3

typedef enum
{
    LIN_AA_STATUS_STATE = 0,
    LIN_AA_STATUS_NAD,
    LIN_AA_STATUS_STEP,
    LIN_AA_STATUS_SELECT,
    LIN_AA_STATUS_RAW_CODE,
    LIN_AA_STATUS_MAX,
} lin_aa_status_e;

typedef void (*LIN_FUNC_CALLBACK)(void);

#if (SNPD_TEST_MODE == SNPD_TEST_MODE_LIN)
struct aa_adc_data
{
    uint8_t org_nad;
    uint8_t new_nad;
    uint16_t adc[5];
};
extern struct aa_adc_data adc_raw_data[AA_SLAVE_NUM];
#endif

typedef struct
{
    uint32_t timeout;
    uint8_t status[LIN_AA_STATUS_MAX];
    LIN_FUNC_CALLBACK enter_func;
    LIN_FUNC_CALLBACK exit_func;
} lin_snpd_context_t;


DECLARE_FAKE_VOID_FUNC(lin_snpd_raw_adc_out,uint8_t, uint8_t);
DECLARE_FAKE_VALUE_FUNC(uint8_t,lin_snpd_status_get,lin_aa_status_e);
DECLARE_FAKE_VOID_FUNC(lin_snpd_status_set,lin_aa_status_e,uint8_t);
DECLARE_FAKE_VOID_FUNC(lin_snpd_nad_read,uint8_t *);
#ifdef CFG_LIN_CONFORM_TEST
DECLARE_FAKE_VOID_FUNC(lin_snpd_id_read);
#endif
DECLARE_FAKE_VOID_FUNC(lin_snpd_nad_write);
DECLARE_FAKE_VOID_FUNC(lin_snpd_cur_th_get);
DECLARE_FAKE_VOID_FUNC(lin_snpd_cur_th_set);
DECLARE_FAKE_VOID_FUNC(lin_snpd_init);
DECLARE_FAKE_VOID_FUNC(lin_snpd_process_handle);
DECLARE_FAKE_VOID_FUNC(autoaddress_config_for_dfu);

// void lin_snpd_raw_adc_out(uint8_t org_nad, uint8_t new_nad);
// uint8_t lin_snpd_status_get(lin_aa_status_e type);
// void lin_snpd_status_set(lin_aa_status_e type, uint8_t value);
// void lin_snpd_nad_read(uint8_t *nad);
// #ifdef CFG_LIN_CONFORM_TEST
// void lin_snpd_id_read(void);
// #endif
// void lin_snpd_nad_write(uint8_t nad);
// void lin_snpd_cur_th_get(uint32_t *st12, uint32_t *st34);
// void lin_snpd_cur_th_set(uint32_t *st12, uint32_t *st34);
// void lin_snpd_init(lin_snpd_context_t *ctx);
// void lin_snpd_process_handle(void);
// void autoaddress_config_for_dfu(void);

#ifdef __cplusplus
}
#endif
#endif /* __LIN_SNPD_H__ */
