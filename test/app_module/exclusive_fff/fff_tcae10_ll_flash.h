/**
 *****************************************************************************
 * @brief   flash header file.
 *
 * @file    tcae10_ll_flash.h
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

#ifndef __FFF_TCAE10_LL_FLASH_H__
#define __FFF_TCAE10_LL_FLASH_H__

#include "fff_tcae10.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FLASH_BYTE_ALIGN (4)

/** @defgroup FLASH_LOCK_Definitions
 * @{
 */
#define FLASH_LOCK_CONFIG() (EFLASH->WR_LOCK = 0X12345678)
#define FLASH_UNLOCK_CONFIG() (EFLASH->WR_LOCK = 0XAA55AA55)

#define FLASH_LOCK_NVR_ACCESS() (EFLASH->NVR_PROT = 0X12345678)
#define FLASH_UNLOCK_NVR_ACCESS() (EFLASH->NVR_PROT = 0XAA55AA55)

/** @defgroup FLASH_NVM_Definitions
 * @{
 */
#define FLASH_SECTOR_SIZE (512)

/* NVM: 64k */
#define NVM_FLASH_BASE_ADDR (0x00000000UL)
#define NVM_FLASH_SIZE (0x00010000UL)
#define NVM_FLASH_END (NVM_FLASH_BASE_ADDR + NVM_FLASH_SIZE)
#define NVM_FLASH_SECTOR_SIZE (FLASH_SECTOR_SIZE)

    typedef enum
    {
        FLASH_TYPE_NVM,
        FLASH_TYPE_MAX,
    } flash_type_e;

    DECLARE_FAKE_VOID_FUNC(ll_flash_init);
    DECLARE_FAKE_VALUE_FUNC(int, ll_flash_erase, flash_type_e, uint32_t, uint32_t);
    DECLARE_FAKE_VALUE_FUNC(int, ll_flash_read, flash_type_e, uint32_t, uint8_t *, uint32_t);
    DECLARE_FAKE_VALUE_FUNC(int, ll_flash_write, flash_type_e, uint32_t, uint8_t *, uint32_t);
    DECLARE_FAKE_VALUE_FUNC(int, ll_flash_smart_write, flash_type_e, uint32_t, uint8_t *, uint32_t);
    DECLARE_FAKE_VALUE_FUNC(int, ll_flash_reg_wr, bool, uint32_t, uint32_t *);

    // void ll_flash_init(void);
    // int ll_flash_erase(flash_type_e type, uint32_t addr, uint32_t length);
    // int ll_flash_read(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length);
    // int ll_flash_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length);
    // int ll_flash_smart_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length);
    // int ll_flash_reg_wr(bool is_write, uint32_t addr, uint32_t *reg_value);

#ifdef __cplusplus
}
#endif
#endif /* __FFF_TCAE10_LL_FLASH_H__ */
