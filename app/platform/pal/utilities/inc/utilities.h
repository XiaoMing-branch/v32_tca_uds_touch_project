/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   utilities header file.
 * @file    utilities.h
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
/* PRQA S 1534 ++ #3261 - Unused macro defined for future extension and configuration compatibility */
#ifndef UTILITIES_H__
#define UTILITIES_H__

#include <stdint.h>

#include "test_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 取两数最大值
 */
#define MAX2_VALUE_GET(x, y) ((x > y) ? x : y)

/**
 * @brief 取两数最小值
 */
#define MIN2_VALUE_GET(x, y) ((x < y) ? x : y)

/**
 * @brief 值钳位（限制在_min和_max之间）
 */
#define CLAMP_VALUE_GET(x, _min, _max) ((x) < (_min) ? (_min) : ((x) > (_max) ? (_max) : (x)))

/**
 * @brief 取三数最大值
 */
#define MAX3_VALUE_GET(x, y, z) (MAX2_VALUE_GET(MAX2_VALUE_GET(x, y), z))

/**
 * @brief 定点除法（四舍五入）
 */
#define DIV_FIXED_POINT(x, y) (((x) + ((y) >> 1)) / (y))

/**
 * @brief  计算数组平均值（去极值）
 * @param  data   - 数据数组
 * @param  length - 数组长度
 * @retval 平均值
 */
uint16_t averge_calculate_utils(uint16_t *data, uint16_t length);
/**
 * @brief  CRC16校验计算
 * @param  init_crc - 初始值
 * @param  data     - 数据指针
 * @param  length   - 数据长度
 * @retval CRC16值
 */
uint16_t crc16_calculate_func(uint16_t init_crc, const uint8_t *data, uint16_t length);
/**
 * @brief  CRC32校验计算
 * @param  init_crc - 初始值
 * @param  data     - 数据指针
 * @param  length   - 数据长度
 * @retval CRC32值
 */
uint32_t crc32_calculate_func(uint32_t init_crc, const uint8_t *data, uint32_t length);
/**
 * @brief  LIN校验和计算
 * @param  init_sum - 初始和
 * @param  data     - 数据指针
 * @param  length   - 数据长度
 * @retval 校验和
 */
uint8_t checksum_calculate_func(uint8_t init_sum, const uint8_t *data, uint16_t length);
/**
 * @brief  字节序反转
 * @param  data   - 数据指针
 * @param  length - 数据长度
 * @retval 反转后的值
 */
uint32_t endian_swap_func(uint8_t *data, uint16_t length);
/**
 * @brief  位序反转
 * @param  bit_data   - 数据指针
 * @param  bit_length - 位长度
 */
void bit_invert_swap_func(void *bit_data, uint8_t bit_length);
#ifdef __cplusplus
}
#endif

#endif /* __UTILITIES_H__ */
/* PRQA S 1534 -- */
