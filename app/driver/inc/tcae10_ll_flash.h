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

#ifndef __TCAE10_LL_FLASH_H__
#define __TCAE10_LL_FLASH_H__

#include "tcae10.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   Flash字节对齐
 */
#define  FLASH_BYTE_ALIGN                   (4)

/** @defgroup FLASH_LOCK_Definitions
  * @{
  */
/** @brief  锁定Flash配置寄存器 */
#define FLASH_LOCK_CONFIG()             (EFLASH->WR_LOCK = 0X12345678)
/** @brief  解锁Flash配置寄存器 */
#define FLASH_UNLOCK_CONFIG()           (EFLASH->WR_LOCK = 0XAA55AA55)

/** @brief  锁定Flash NVR访问 */
#define FLASH_LOCK_NVR_ACCESS()         (EFLASH->NVR_PROT = 0X12345678)
/** @brief  解锁Flash NVR访问 */
#define FLASH_UNLOCK_NVR_ACCESS()       (EFLASH->NVR_PROT = 0XAA55AA55)

/** @defgroup FLASH_NVM_Definitions
  * @{
  */
/** @brief  Flash扇区大小（字节） */
#define  FLASH_SECTOR_SIZE              (512)

/* NVM: 64k */
/** @brief  NVM Flash基地址 */
#define  NVM_FLASH_BASE_ADDR            (0x00000000UL)
/** @brief  NVM Flash总大小 */
#define  NVM_FLASH_SIZE                 (0x00010000UL)
/** @brief  NVM Flash结束地址 */
#define  NVM_FLASH_END                  (NVM_FLASH_BASE_ADDR + NVM_FLASH_SIZE)
/** @brief  NVM Flash扇区大小 */
#define  NVM_FLASH_SECTOR_SIZE          (FLASH_SECTOR_SIZE)

typedef enum
{
    FLASH_TYPE_NVM,
    FLASH_TYPE_MAX,
} flash_type_e;

/**
 * @brief  初始化Flash模块
 */
void ll_flash_init(void);
/**
 * @brief  擦除Flash区域
 * @param type - Flash类型 @ref flash_type_e
 * @param addr - 起始地址
 * @param length - 擦除长度（字节）
 * @retval 0 成功，非0 失败
 */
int ll_flash_erase(flash_type_e type, uint32_t addr, uint32_t length);
/**
 * @brief  从Flash读取数据
 * @param type - Flash类型 @ref flash_type_e
 * @param addr - 读取地址
 * @param buffer - 数据缓冲区
 * @param length - 读取长度（字节）
 * @retval 0 成功，非0 失败
 */
int ll_flash_read(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length);
/**
 * @brief  写入数据到Flash
 * @param type - Flash类型 @ref flash_type_e
 * @param addr - 写入地址
 * @param buffer - 数据缓冲区
 * @param length - 写入长度（字节）
 * @retval 0 成功，非0 失败
 */
int ll_flash_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length);
/**
 * @brief  智能写入数据到Flash（自动判断是否需要擦除）
 * @param type - Flash类型 @ref flash_type_e
 * @param addr - 写入地址
 * @param buffer - 数据缓冲区
 * @param length - 写入长度（字节）
 * @retval 0 成功，非0 失败
 */
int ll_flash_smart_write(flash_type_e type, uint32_t addr, uint8_t *buffer, uint32_t length);
/**
 * @brief  读写Flash寄存器
 * @param is_write - true: 写入，false: 读取
 * @param addr - 寄存器地址
 * @param reg_value - 写入值/读取值指针
 * @retval 0 成功，非0 失败
 */
int ll_flash_reg_wr(bool is_write, uint32_t addr, uint32_t *reg_value);

#ifdef __cplusplus
}
#endif
#endif /* __TCAE10_LL_FLASH_H__ */
