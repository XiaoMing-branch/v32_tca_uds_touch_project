/**
 *****************************************************************************
 * @brief   pal store header file.
 *
 * @file    pal_store.h
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

#ifndef __FFF_PAL_STORE_H__
#define __FFF_PAL_STORE_H__

#include "fff.h"
#include "fff_pal_func_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__TCPL01X__)
#define STORE_TYPE_SEL (FLASH_TYPE_NVR)
#define STORE_SECTOR_SIZE (NVR_FLASH_SECTOR_SIZE)
#define BOOT_VERSION_ADDR (0x00002E18UL)
#elif defined(__TCPL03X__) || defined(__TCAE10__)
#define STORE_TYPE_SEL (FLASH_TYPE_NVM)
#define STORE_SECTOR_SIZE (NVM_FLASH_SECTOR_SIZE)
#define BOOT_VERSION_ADDR (0x00001E18UL)
#endif

    DECLARE_FAKE_VOID_FUNC(pal_store_data_set, uint32_t, uint8_t *, uint16_t);
    DECLARE_FAKE_VALUE_FUNC(bool, pal_store_data_get, uint32_t, uint8_t *, uint16_t);
    DECLARE_FAKE_VALUE_FUNC(bool, pal_store_data_init, uint32_t, uint8_t *, uint16_t);
    DECLARE_FAKE_VALUE_FUNC(bool, pal_store_data_clear, uint32_t, uint16_t);
    DECLARE_FAKE_VALUE_FUNC(bool, pal_store_erase, flash_type_e, uint32_t, uint16_t);
    DECLARE_FAKE_VALUE_FUNC(bool, pal_store_write, flash_type_e, uint32_t, uint8_t *, uint16_t);
    DECLARE_FAKE_VALUE_FUNC(bool, pal_store_read, flash_type_e, uint32_t, uint8_t *, uint16_t);
    DECLARE_FAKE_VOID_FUNC(pal_store_uid_get, uint32_t *);
    DECLARE_FAKE_VOID_FUNC(pal_store_boot_ver_get, uint32_t *);
    DECLARE_FAKE_VOID_FUNC(pal_store_chip_ver_id_get, uint8_t *, uint16_t *);
    DECLARE_FAKE_VOID_FUNC(pal_store_reg_rw, bool, uint32_t, uint32_t *);

    // void pal_store_data_set(uint32_t addr, uint8_t *data, uint16_t length);
    // bool pal_store_data_get(uint32_t addr, uint8_t *data, uint16_t length);
    // bool pal_store_data_init(uint32_t addr, uint8_t *data, uint16_t length);
    // bool pal_store_data_clear(uint32_t addr, uint16_t length);
    // bool pal_store_erase(flash_type_e type, uint32_t addr, uint16_t length);
    // bool pal_store_write(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length);
    // bool pal_store_read(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length);

    // void pal_store_uid_get(uint32_t *uid);
    // void pal_store_boot_ver_get(uint32_t *boot_ver);
    // void pal_store_chip_ver_id_get(uint8_t *chip_ver, uint16_t *chip_id);
    // void pal_store_reg_rw(bool is_write, uint32_t addr, uint32_t *value);

#ifdef __cplusplus
}
#endif

#endif /*__FFF_PAL_STORE_H__*/
