
/**
 *****************************************************************************
 * @brief   store manager header file.
 * @file    store_manager.h
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

#ifndef    __STORE_MANAGER_H__
#define    __STORE_MANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include "store_map.h"

#ifdef __cplusplus
extern "C"
{
#endif

//typedef enum
//{
//    LED_PN_VOLT_PARAM,
//    LED_RGB_PARAM,
//    LED_WHITE_COLOR_PARAM,
//    LED_RELATIVE_FACTOR_PARAM,
//    LED_SERIES_NUM_PARAM,
//} led_param_type_e;

typedef enum
{
    SYSTEM_CFG_PARAM = 0,
    SYSTEM_ID_CFG_PARAM,
} system_param_type_e;

/**
* @enum chip_info_type
*/
typedef enum
{
    CHIP_INFO_VER_ID = 0,
    CHIP_INFO_UUID,
    CHIP_INFO_BOOT_VER,
} chip_info_type_e;

/**
* @enum chip_ver_id enum
*/
typedef struct
{
    uint8_t  ver;
    uint16_t id;
} chip_ver_id_t;

typedef struct
{
    uint32_t    nad;
    uint32_t    cur_th_st12;
    uint32_t    cur_th_st34;
    uint8_t     uvlo;
    uint8_t     uvlo_th;
    uint8_t     ovlo;
    uint8_t     ovlo_th;
    uint8_t     otp;
    uint8_t     otp_th;
    uint8_t     org_nad;
    uint8_t     match;
//    uint8_t     *frame_id_cfg;
} sys_cfg_t __attribute__((aligned(1)));

extern sys_cfg_t g_sys_cfgs;

void store_manager_clear(void);
void store_manager_init(void);
//void store_generic_data_set(led_param_type_e type, uint8_t *param, uint16_t len);
//bool store_generic_data_get(led_param_type_e type, uint8_t *param, uint16_t len);
void store_system_data_set(system_param_type_e type, uint8_t *param, uint16_t len);
bool store_system_data_get(system_param_type_e type, uint8_t *param, uint16_t len);
bool store_chip_info_get(chip_info_type_e type, uint8_t *param, uint16_t len);
bool store_customer_data_set(uint32_t addr_offset, uint8_t *param, uint16_t len);
bool store_customer_data_get(uint32_t addr_offset, uint8_t *param, uint16_t len);
void store_default_whitepoint(void);

bool store_slow_read(uint32_t addr, uint8_t *value, uint16_t length);		//地址4字节对齐
bool store_slow_smart_read(uint32_t addr, uint8_t *value, uint16_t length);	//无地址对齐限制
bool store_slow_write(uint32_t addr, uint8_t *value, uint16_t length);

#ifdef __cplusplus
}
#endif
#endif
