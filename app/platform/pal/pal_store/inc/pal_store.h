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

#ifndef __PAL_STORE_H__
#define __PAL_STORE_H__

#include "pal_func_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flash类型选择宏（根据芯片系列选择NVR或NVM）
 */
#if defined (__TCPL01X__)
#define STORE_TYPE_SEL              (FLASH_TYPE_NVR)
#define STORE_SECTOR_SIZE           (NVR_FLASH_SECTOR_SIZE)
#define BOOT_VERSION_ADDR           (0x00002E18UL)
#elif defined (__TCPL03X__) || defined(__TCAE10__)
#define STORE_TYPE_SEL              (FLASH_TYPE_NVM)
#define STORE_SECTOR_SIZE           (NVM_FLASH_SECTOR_SIZE)
#define BOOT_VERSION_ADDR           (0x00001E18UL)
#endif

/**
 * @brief  写入存储数据（带CRC）
 * @param  addr   - 地址
 * @param  data   - 数据指针
 * @param  length - 长度
 */
void pal_store_data_set(uint32_t addr, uint8_t *data, uint16_t length);
/**
 * @brief  读取存储数据（带CRC校验）
 * @param  addr   - 地址
 * @param  data   - 数据缓冲区
 * @param  length - 长度
 * @retval true 成功 false 失败
 */
bool pal_store_data_get(uint32_t addr, uint8_t *data, uint16_t length);
/**
 * @brief  存储数据初始化
 * @param  addr   - 地址
 * @param  data   - 默认数据
 * @param  length - 长度
 * @retval true 已有有效数据 false 写入默认值
 */
bool pal_store_data_init(uint32_t addr, uint8_t *data, uint16_t length);
/**
 * @brief  清除存储数据
 * @param  addr   - 地址
 * @param  length - 长度
 * @retval true 成功 false 失败
 */
bool pal_store_data_clear(uint32_t addr, uint16_t length);
/**
 * @brief  擦除Flash区域
 * @param  type   - Flash类型
 * @param  addr   - 地址
 * @param  length - 长度
 * @retval true 成功 false 失败
 */
bool pal_store_erase(flash_type_e type, uint32_t addr, uint16_t length);
/**
 * @brief  写入Flash
 * @param  type   - Flash类型
 * @param  addr   - 地址
 * @param  value  - 数据指针
 * @param  length - 长度
 * @retval true 成功 false 失败
 */
bool pal_store_write(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length);
/**
 * @brief  读取Flash
 * @param  type   - Flash类型
 * @param  addr   - 地址
 * @param  value  - 缓冲区
 * @param  length - 长度
 * @retval true 成功 false 失败
 */
bool pal_store_read(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length);
/**
 * @brief  获取芯片UID
 * @param  uid - UID缓冲区
 */
void pal_store_uid_get(uint32_t *uid);
/**
 * @brief  获取Boot版本号
 * @param  boot_ver - Boot版本号
 */
void pal_store_boot_ver_get(uint32_t *boot_ver);
/**
 * @brief  获取芯片版本和ID
 * @param  chip_ver - 芯片版本
 * @param  chip_id  - 芯片ID
 */
void pal_store_chip_ver_id_get(uint8_t *chip_ver, uint16_t *chip_id);
/**
 * @brief  Flash寄存器读写
 * @param  is_write - 写标志
 * @param  addr     - 寄存器地址
 * @param  value    - 数据指针
 */
void pal_store_reg_rw(bool is_write, uint32_t addr, uint32_t *value);

#ifdef __cplusplus
}
#endif

#endif /*__PAL_STORE_H__*/
