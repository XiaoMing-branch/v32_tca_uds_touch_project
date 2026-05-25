
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

/**
 * @brief  系统参数类型枚举，用于索引 sys_param_addr_map 地址映射表
 */
typedef enum
{
    SYSTEM_CFG_PARAM = 0,       /**< 系统配置参数（nad、阈值、保护参数等） */
    SYSTEM_ID_CFG_PARAM,        /**< LIN 节点配置参数 */
} system_param_type_e;

/**
 * @brief  芯片信息类型枚举，标识需要读取的芯片信息种类
 */
typedef enum
{
    CHIP_INFO_VER_ID = 0,       /**< 芯片版本号和 ID */
    CHIP_INFO_UUID,             /**< 芯片唯一标识 UUID */
    CHIP_INFO_BOOT_VER,         /**< Bootloader 版本号 */
} chip_info_type_e;

/**
 * @brief  芯片版本号与 ID 结构体
 */
typedef struct
{
    uint8_t  ver;               /**< 芯片版本号 */
    uint16_t id;                /**< 芯片标识 ID */
} chip_ver_id_t;

/**
 * @brief  系统配置参数结构体（1 字节对齐）
 * @note   存储 LIN 节点的 NAD、电容触摸阈值、电压/温度保护阈值等运行参数
 */
typedef struct
{
    uint32_t    nad;            /**< LIN 节点 NAD 地址 */
    uint32_t    cur_th_st12;    /**< 通道 1/2 电容触摸阈值 */
    uint32_t    cur_th_st34;    /**< 通道 3/4 电容触摸阈值 */
    uint8_t     uvlo;           /**< 欠压锁定使能标志 */
    uint8_t     uvlo_th;        /**< 欠压锁定阈值 */
    uint8_t     ovlo;           /**< 过压锁定使能标志 */
    uint8_t     ovlo_th;        /**< 过压锁定阈值 */
    uint8_t     otp;            /**< 过温保护使能标志 */
    uint8_t     otp_th;         /**< 过温保护阈值 */
    uint8_t     org_nad;        /**< 原始 NAD（出厂默认） */
    uint8_t     match;          /**< 匹配标志位 */
} sys_cfg_t __attribute__((aligned(1)));

/** @brief  全局系统配置变量，初始化时从 Flash 加载 */
extern sys_cfg_t g_sys_cfgs;

/**
 * @brief  清除所有存储参数（擦除系统参数区 Flash）
 * @retval 无
 */
void store_manager_clear(void);

/**
 * @brief  初始化存储管理器（从 Flash 加载系统参数）
 * @retval 无
 */
void store_manager_init(void);

/**
 * @brief  写入系统参数到 Flash
 * @param  type  - 参数类型（system_param_type_e）
 * @param  param - 参数数据指针
 * @param  len   - 数据长度
 * @retval 无
 */
void store_system_data_set(system_param_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  从 Flash 读取系统参数
 * @param  type   - 参数类型（system_param_type_e）
 * @param  param  - 存放读取数据的缓冲区指针
 * @param  len    - 期望读取的数据长度
 * @retval true   - 读取成功
 * @retval false  - 读取失败
 */
bool store_system_data_get(system_param_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  读取芯片硬件信息（版本号/ID/UUID/Bootloader 版本）
 * @param  type   - 芯片信息类型（chip_info_type_e）
 * @param  param  - 存放读取数据的缓冲区指针
 * @param  len    - 缓冲区长度
 * @retval true   - 读取成功
 */
bool store_chip_info_get(chip_info_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  写入客户自定义参数到 Flash（sector 0 客户区）
 * @param  addr_offset - 在客户参数区内的偏移地址
 * @param  param       - 参数数据指针
 * @param  len         - 数据长度
 * @retval true        - 写入成功
 * @retval false       - 地址越界（超出 FLASH_SECTOR_SIZE）
 */
bool store_customer_data_set(uint32_t addr_offset, uint8_t *param, uint16_t len);

/**
 * @brief  从 Flash 读取客户自定义参数
 * @param  addr_offset - 在客户参数区内的偏移地址
 * @param  param       - 存放读取数据的缓冲区指针
 * @param  len         - 读取长度
 * @retval true        - 读取成功
 * @retval false       - 地址越界
 */
bool store_customer_data_get(uint32_t addr_offset, uint8_t *param, uint16_t len);

/**
 * @brief  恢复默认白平衡参数
 * @retval 无
 */
void store_default_whitepoint(void);

/**
 * @brief  慢速读取 Flash（地址需 4 字节对齐）
 * @param  addr   - 读取地址
 * @param  value  - 数据缓冲区指针
 * @param  length - 读取长度
 * @retval true   - 读取成功
 * @retval false  - 读取失败
 */
bool store_slow_read(uint32_t addr, uint8_t *value, uint16_t length);

/**
 * @brief  智能慢速读取 Flash（无地址对齐限制）
 * @note   自动处理非对齐地址，分三段读取：开头不对齐部分、中间对齐部分、结尾不对齐部分
 * @param  addr   - 读取地址
 * @param  value  - 数据缓冲区指针
 * @param  length - 读取长度
 * @retval true   - 读取成功
 * @retval false  - 读取失败
 */
bool store_slow_smart_read(uint32_t addr, uint8_t *value, uint16_t length);

/**
 * @brief  慢速写入 Flash（含擦除-交换-回写流程）
 * @note   使用交换区（FLASH_SWAP_BASE_ADDR）暂存原始数据，
 *         擦除目标扇区后将新数据合并写入，避免数据丢失
 * @param  addr   - 写入地址
 * @param  value  - 数据指针
 * @param  length - 写入长度
 * @retval true   - 写入成功
 * @retval false  - 写入失败
 */
bool store_slow_write(uint32_t addr, uint8_t *value, uint16_t length);

#ifdef __cplusplus
}
#endif
#endif
