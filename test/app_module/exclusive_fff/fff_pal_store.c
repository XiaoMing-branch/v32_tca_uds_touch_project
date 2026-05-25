/**
 *****************************************************************************
 * @brief   pal store source file.
 *
 * @file    pal_store.c
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

#include "fff_pal_store.h"
#include "fff_utilities.h"
#include "fff_store_manager.h"

DEFINE_FAKE_VOID_FUNC(pal_store_data_set, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, pal_store_data_get, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, pal_store_data_init, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, pal_store_data_clear, uint32_t, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, pal_store_erase, flash_type_e, uint32_t, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, pal_store_write, flash_type_e, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, pal_store_read, flash_type_e, uint32_t, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(pal_store_uid_get, uint32_t *);
DEFINE_FAKE_VOID_FUNC(pal_store_boot_ver_get, uint32_t *);
DEFINE_FAKE_VOID_FUNC(pal_store_chip_ver_id_get, uint8_t *, uint16_t *);
DEFINE_FAKE_VOID_FUNC(pal_store_reg_rw, bool, uint32_t, uint32_t *);
