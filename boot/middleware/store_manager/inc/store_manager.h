/**
 *****************************************************************************
 * @brief   store manager header file.
 *
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
#include "pal_func_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  LED 参数类型枚举
 * @note   定义 LED 各参数类别在 Flash 中的存储索引
 */
typedef enum
{
    LED_PN_VOLT_PARAM,          /**< PN 结电压典型参数 */
    LED_RGB_PARAM,              /**< RGB 颜色参数 */
    LED_WHITE_COLOR_PARAM,      /**< 白平衡参数 */
    LED_RELATIVE_FACTOR_PARAM,  /**< 相对比例因子参数 */
    LED_SERIES_NUM_PARAM,       /**< 序列号参数 */
    LED_PARAM_TYPE_MAX,         /**< 参数类型总数 */
} led_param_type_e;

/**
 * @brief  系统参数类型枚举
 * @note   定义系统参数类别在 Flash 中的存储索引
 */
typedef enum
{
    SYSTEM_CFG_PARAM = 0,       /**< 系统配置参数（NAD、电流阈值等） */
    SYSTEM_ID_CFG_PARAM,        /**< LIN 帧 ID 配置参数 */
} system_param_type_e;

/**
 * @brief  芯片信息类型枚举
 */
typedef enum
{
    CHIP_INFO_VER_ID = 0,       /**< 芯片版本和 ID */
    CHIP_INFO_UUID,             /**< 芯片唯一标识 UUID */
    CHIP_INFO_BOOT_VER,         /**< Bootloader 版本号 */
} chip_info_type_e;

/**
 * @brief  芯片版本 ID 结构体
 */
typedef struct
{
    uint8_t  ver;               /**< 版本号 */
    uint16_t id;                /**< 芯片 ID */
} chip_ver_id_t;

/**
 * @brief  系统配置结构体
 * @note   存储 NAD 地址、电流阈值、欠压/过压/过温保护参数及帧 ID 配置
 */
typedef struct
{
    uint32_t    nad;            /**< 节点地址（NAD） */
    uint32_t    cur_th_st12;    /**< 状态 1/2 电流阈值 */
    uint32_t    cur_th_st34;    /**< 状态 3/4 电流阈值 */
    uint8_t     uvlo;           /**< 欠压锁定使能 */
    uint8_t     uvlo_th;        /**< 欠压锁定阈值 */
    uint8_t     ovlo;           /**< 过压锁定使能 */
    uint8_t     ovlo_th;        /**< 过压锁定阈值 */
    uint8_t     otp;            /**< 过温保护使能 */
    uint8_t     otp_th;         /**< 过温保护阈值 */
    uint8_t     org_nad;        /**< 原始 NAD（恢复用） */
    uint8_t     *frame_id_cfg;  /**< LIN 帧 ID 配置指针 */
} sys_cfg_t __attribute__((aligned(1)));

/** @brief 全局系统配置实例 */
extern sys_cfg_t g_sys_cfgs;

/**
 * @brief  清除所有存储参数
 * @retval 无
 */
void store_manager_clear(void);

/**
 * @brief  存储管理器初始化
 * @retval 无
 */
void store_manager_init(void);

/**
 * @brief  写入 LED 通用参数到 Flash
 * @param  channel - LED 通道索引
 * @param  type    - LED 参数类型
 * @param  param   - 待写入数据缓冲区指针
 * @param  len     - 数据长度（字节）
 * @retval 无
 */
void store_generic_data_set(led_channel_e channel, led_param_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  从 Flash 读取 LED 通用参数
 * @param  channel - LED 通道索引
 * @param  type    - LED 参数类型
 * @param  param   - 存放数据缓冲区指针
 * @param  len     - 期望数据长度（字节）
 * @retval true  - 读取成功
 * @retval false - 读取失败
 */
bool store_generic_data_get(led_channel_e channel, led_param_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  写入系统参数到 Flash
 * @param  type  - 系统参数类型
 * @param  param - 待写入数据缓冲区指针
 * @param  len   - 数据长度（字节）
 * @retval 无
 */
void store_system_data_set(system_param_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  从 Flash 读取系统参数
 * @param  type  - 系统参数类型
 * @param  param - 存放数据缓冲区指针
 * @param  len   - 期望数据长度（字节）
 * @retval true  - 读取成功
 * @retval false - 读取失败
 */
bool store_system_data_get(system_param_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  获取芯片信息
 * @param  type  - 芯片信息类型
 * @param  param - 存放数据缓冲区指针
 * @param  len   - 缓冲区长度（字节）
 * @retval true  - 获取成功
 */
bool store_chip_info_get(chip_info_type_e type, uint8_t *param, uint16_t len);

/**
 * @brief  从 Flash 加载指定通道所有 LED 参数
 * @param  channel - LED 通道索引
 * @retval 无
 */
void store_led_param_load(led_channel_e channel);

/**
 * @brief  写入客户自定义参数到 Flash
 * @param  addr_offset - 客户参数区偏移地址
 * @param  param       - 待写入数据缓冲区指针
 * @param  len         - 数据长度（字节）
 * @retval true  - 写入成功
 * @retval false - 地址越界
 */
bool store_customer_data_set(uint32_t addr_offset, uint8_t *param, uint16_t len);

/**
 * @brief  从 Flash 读取客户自定义参数
 * @param  addr_offset - 客户参数区偏移地址
 * @param  param       - 存放数据缓冲区指针
 * @param  len         - 期望数据长度（字节）
 * @retval true  - 读取成功
 * @retval false - 地址越界
 */
bool store_customer_data_get(uint32_t addr_offset, uint8_t *param, uint16_t len);

/**
 * @brief  恢复白平衡参数为默认值
 * @retval 无
 */
void store_default_whitepoint(void);

/**
 * @brief  写入芯片寄存器参数
 * @param  addr  - 寄存器地址指针（4 字节小端序）
 * @param  value - 寄存器值指针（4 字节小端序）
 * @retval 无
 */
void store_reg_param_set(uint8_t *addr, uint8_t *value);

/**
 * @brief  读取芯片寄存器参数
 * @param  addr  - 寄存器地址指针（4 字节小端序）
 * @param  value - 存放读取值的缓冲区指针（4 字节）
 * @retval 无
 */
void store_reg_param_get(uint8_t *addr, uint8_t *value);

#ifdef __cplusplus
}
#endif
#endif
