/**
 *****************************************************************************
 * @brief   store map header file.
 *
 * @file    store_map.h
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

#ifndef __STORE_MAP_H__
#define __STORE_MAP_H__

/**
 * @name   芯片平台选择
 * @brief  根据芯片型号和应用版本选择不同的 Flash 地址映射
 * @{
 */
#if defined (__TCPL01X__)
#if defined APP_MATCH_BOOT_V2_1
#define FLASH_APP_ADDR                      (0x00002E1CUL)
#elif defined APP_MATCH_BOOT_V3_0
#define FLASH_APP_ADDR                      (0x00001E1CUL)
#endif
/** @} */

/**
 * @name   TCPL01X Flash 分区基址
 * @brief  TCPL01X 平台各存储分区基址
 * @{
 */
#define CUSTOMER_PARAM_BASE_ADDR            (0x00800000UL) /**< 客户参数区基址（sector 0） */
#define LED0_PARAM_BASE_ADDR                (0x00800200UL) /**< LED 参数区基址（sector 1） */
#define SYSTEM_PARAM_BASE_ADDR              (0x00800400UL) /**< 系统参数区基址（sector 2） */
/** @} */
#elif defined (__TCPL03X__)
#define FLASH_APP_ADDR                      (0x00001E1CUL)

/**
 * @name   TCPL03X Flash 分区基址
 * @brief  TCPL03X 平台各存储分区基址
 * @{
 */
#define SYSTEM_PARAM_BASE_ADDR              (0x0000FA00UL) /**< 系统参数区基址（sector 0） */
#define LED0_PARAM_BASE_ADDR                (0x0000FC00UL) /**< LED 参数区基址（sector 1） */
#define CUSTOMER_PARAM_BASE_ADDR            (0x0000FA00UL) /**< 客户参数区基址（sector 2） */
/** @} */
#else
#define FLASH_APP_ADDR                      (0x00001E1CUL)

/**
 * @name   默认 Flash 分区基址
 * @brief  默认平台各存储分区基址
 * @{
 */
#define CUSTOMER_PARAM_BASE_ADDR            (0x00800000UL) /**< 客户参数区基址（sector 0） */
#define LED0_PARAM_BASE_ADDR                (0x00800200UL) /**< LED 参数区基址（sector 1） */
#define SYSTEM_PARAM_BASE_ADDR              (0x00800400UL) /**< 系统参数区基址（sector 2） */
/** @} */
#endif

/**
 * @name   Sector 1 - LED 参数偏移映射
 * @brief  LED 参数在 Flash sector 1 中的偏移、大小及对齐补白
 * @{
 */
#define LED_PARAM_CRC_SIZE          (4)     /**< CRC 校验字节数 */

#define LED_TEMP_PN_VOLT_OFFSET     (0)     /**< PN 结电压参数偏移 */
#define LED_TEMP_PN_VOLT_SIZE       (sizeof(cm_pn_volt_param_t)) /**< 8 Byte 数据 + 4 Byte CRC */
#define LED_TEMP_PN_VOLT_RESEVERD   (4-LED_TEMP_PN_VOLT_SIZE%4) /**< 4 字节对齐补白 */

#define LED_RGB_OFFSET              (LED_TEMP_PN_VOLT_OFFSET+LED_TEMP_PN_VOLT_SIZE+LED_PARAM_CRC_SIZE+LED_TEMP_PN_VOLT_RESEVERD)
#define LED_RGB_SIZE                (sizeof(cm_led_param_t)) /**< 24 Byte 数据 + 4 Byte CRC */
#define LED_RGB_RESEVERD            (4-LED_RGB_SIZE%4)      /**< 4 字节对齐补白 */

#define LED_WHITE_COLOR_OFFSET      (LED_RGB_OFFSET+LED_RGB_SIZE+LED_PARAM_CRC_SIZE +LED_RGB_RESEVERD)
#define LED_WHITE_COLOR_SIZE        (sizeof(cm_white_point_param_t)) /**< 14 Byte 数据 + 4 Byte CRC */
#define LED_WHITE_COLOR_RESEVERD    (4-LED_WHITE_COLOR_SIZE%4)      /**< 4 字节对齐补白 */

#define LED_RELATIVE_FACTOR_OFFSET  (LED_WHITE_COLOR_OFFSET+LED_WHITE_COLOR_SIZE+LED_PARAM_CRC_SIZE+LED_WHITE_COLOR_RESEVERD)
#define LED_RELATIVE_FACTOR_SIZE    (sizeof(uint32_t))       /**< 4 Byte 数据 */
#define LED_RELATIVE_FACTOR_RESEVERD (4-LED_RELATIVE_FACTOR_SIZE%4) /**< 4 字节对齐补白 */

#define LED_SERIES_NUM_OFFSET       (LED_RELATIVE_FACTOR_OFFSET+LED_RELATIVE_FACTOR_SIZE+LED_PARAM_CRC_SIZE+LED_RELATIVE_FACTOR_RESEVERD)
#define LED_SERIES_NUM_SIZE         (sizeof(uint32_t))       /**< 4 Byte 数据 */
#define LED_SERIES_NUM_RESEVERD     (4-LED_SERIES_NUM_SIZE%4) /**< 4 字节对齐补白 */

/** @brief 单个 LED 通道参数总大小（含 CRC） */
#define TOTAL_LED_PARAM_SIZE        (LED_SERIES_NUM_OFFSET+LED_SERIES_NUM_SIZE+LED_SERIES_NUM_RESEVERD+LED_PARAM_CRC_SIZE)
/** @} */

/**
 * @name   Sector 2 - 系统参数偏移映射
 * @brief  系统参数在 Flash sector 2 中的偏移、大小及对齐补白
 * @{
 */
#define SYSTEM_PARAM_CRC_SIZE       (4)     /**< CRC 校验字节数 */
#define SYSTEM_CFG_OFFSET           (0)     /**< 系统配置参数偏移 */
#define SYSTEM_CFG_SIZE             (sizeof(sys_cfg_t)) /**< 20 Byte 数据 + 4 Byte CRC */
#define SYSTEM_CFG_RESEVERD         (4-SYSTEM_CFG_SIZE%4) /**< 4 字节对齐补白 */
#define SYSTEM_ID_CFG_OFFSET        (SYSTEM_CFG_OFFSET + SYSTEM_CFG_SIZE + SYSTEM_PARAM_CRC_SIZE + SYSTEM_CFG_RESEVERD)
#define SYSTEM_ID_CFG_SIZE          (66)    /**< LIN 帧 ID 配置大小 */
#define SYSTEM_ID_CFG_RESEVERD      (4-SYSTEM_ID_CFG_SIZE%4) /**< 4 字节对齐补白 */

/** @brief 系统参数总大小（含 CRC） */
#define TOTAL_SYSTEM_PARAM_SIZE     (SYSTEM_ID_CFG_OFFSET+SYSTEM_ID_CFG_SIZE+SYSTEM_ID_CFG_RESEVERD+SYSTEM_PARAM_CRC_SIZE)
/** @} */

#endif
