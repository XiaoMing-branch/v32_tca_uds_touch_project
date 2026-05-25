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

#if defined (__TCPL01X__)
#define STORE_TYPE_SEL              (FLASH_TYPE_NVR)
#define STORE_SECTOR_SIZE           (NVR_FLASH_SECTOR_SIZE)
#define BOOT_VERSION_ADDR           (0x00002E18UL)
#elif defined (__TCPL03X__)
#define STORE_TYPE_SEL              (FLASH_TYPE_NVM)
#define STORE_SECTOR_SIZE           (NVM_FLASH_SECTOR_SIZE)
#define BOOT_VERSION_ADDR           (0x00001E18UL)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  存储数据到Flash(带CRC校验)
 * @param  addr - 起始地址
 * @param  data - 写入数据
 * @param  length - 数据长度
 */
void pal_store_data_set(uint32_t addr, uint8_t *data, uint16_t length);

/**
 * @brief  从Flash读取数据(CRC校验)
 * @param  addr - 起始地址
 * @param  data - 输出数据
 * @param  length - 数据长度
 * @retval true - 成功, false - CRC校验失败
 */
bool pal_store_data_get(uint32_t addr, uint8_t *data, uint16_t length);

/**
 * @brief  初始化存储数据(CRC失败则写入默认值)
 * @param  addr - 起始地址
 * @param  data - 默认数据
 * @param  length - 数据长度
 * @retval true - 数据有效, false - 已重新初始化
 */
bool pal_store_data_init(uint32_t addr, uint8_t *data, uint16_t length);

/**
 * @brief  清除存储数据
 * @param  addr - 起始地址
 * @param  length - 清除长度
 * @retval true - 成功
 */
bool pal_store_data_clear(uint32_t addr, uint16_t length);

/**
 * @brief  擦除Flash扇区
 * @param  type - Flash类型
 * @param  addr - 起始地址
 * @param  length - 擦除长度
 * @retval true - 成功
 */
bool pal_store_erase(flash_type_e type, uint32_t addr, uint16_t length);

/**
 * @brief  写Flash数据
 * @param  type - Flash类型
 * @param  addr - 地址
 * @param  value - 写入数据
 * @param  length - 长度
 * @retval true - 成功
 */
bool pal_store_write(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length);

/**
 * @brief  读Flash数据
 * @param  type - Flash类型
 * @param  addr - 地址
 * @param  value - 输出数据
 * @param  length - 长度
 * @retval true - 成功
 */
bool pal_store_read(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length);

/**
 * @brief  获取芯片唯一ID
 * @param  uid - 输出UID(3个uint32_t)
 */
void pal_store_uid_get(uint32_t *uid);

/**
 * @brief  获取Boot版本号
 * @param  boot_ver - 输出版本号
 */
void pal_store_boot_ver_get(uint32_t *boot_ver);

/**
 * @brief  获取芯片版本和ID
 * @param  chip_ver - 输出版本
 * @param  chip_id - 输出ID
 */
void pal_store_chip_ver_id_get(uint8_t *chip_ver, uint16_t *chip_id);

/**
 * @brief  Flash寄存器读写
 * @param  is_write - true:写, false:读
 * @param  addr - 寄存器地址
 * @param  value - 写入/读取的值
 */
void pal_store_reg_rw(bool is_write, uint32_t addr, uint32_t *value);

#ifdef __cplusplus
}
#endif
#endif /*__PAL_STORE_H__*/
