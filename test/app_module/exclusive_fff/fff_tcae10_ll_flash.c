/**
 *****************************************************************************
 * @brief   flash Source file.
 *
 * @file    tcae10_ll_flash.c
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

#include "fff_tcae10.h"
#include "fff_system_tcae10.h"
#include "fff_tcae10_ll_flash.h"

DEFINE_FAKE_VOID_FUNC(ll_flash_init);
DEFINE_FAKE_VALUE_FUNC(int, ll_flash_erase, flash_type_e, uint32_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, ll_flash_read, flash_type_e, uint32_t, uint8_t *, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, ll_flash_write, flash_type_e, uint32_t, uint8_t *, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, ll_flash_smart_write, flash_type_e, uint32_t, uint8_t *, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, ll_flash_reg_wr, bool, uint32_t, uint32_t *);
