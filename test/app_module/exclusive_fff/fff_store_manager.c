/**
 *****************************************************************************
 * @brief   store manager source file.
 *
 * @file    store_manager.c
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

#include "fff_store_manager.h"
#include "fff_pal_store.h"

sys_cfg_t g_sys_cfgs =
    {
        .nad = 0x6F,
#if CFG_SUPPROT_LINSNPD_EXT_RES
        .cur_th_st12 = 0x68003C00,
        .cur_th_st34 = 0x50009D00,
#else
        .cur_th_st12 = 0x68003C00,
        .cur_th_st34 = 0x50009D00,
#endif
        .org_nad = 0x01,
};

// const uint32_t led_param_addr_map[] =
//{
//     LED_TEMP_PN_VOLT_OFFSET,
//     LED_RGB_OFFSET,
//     LED_WHITE_COLOR_OFFSET,
//     LED_RELATIVE_FACTOR_OFFSET,
//     LED_SERIES_NUM_OFFSET,
// };

const uint32_t sys_param_addr_map[] =
    {
        SYSTEM_CFG_OFFSET,
        SYSTEM_ID_CFG_OFFSET,
};

DEFINE_FAKE_VOID_FUNC(store_manager_clear);
DEFINE_FAKE_VOID_FUNC(store_manager_init);
DEFINE_FAKE_VOID_FUNC(store_system_data_set, system_param_type_e, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, store_system_data_get, system_param_type_e, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, store_chip_info_get, chip_info_type_e, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, store_customer_data_set, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, store_customer_data_get, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(store_default_whitepoint);
DEFINE_FAKE_VALUE_FUNC(bool, store_slow_read, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, store_slow_smart_read, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, store_slow_write, uint32_t, uint8_t *, uint16_t);
