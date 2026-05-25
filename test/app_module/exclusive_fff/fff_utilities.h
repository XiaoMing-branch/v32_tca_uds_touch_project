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
#ifndef __FFF_UTILITIES_H__
#define __FFF_UTILITIES_H__

#include "fff.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX2_VALUE_GET(x, y) ((x > y) ? x : y)

#define MIN2_VALUE_GET(x, y) ((x < y) ? x : y)

#define CLAMP_VALUE_GET(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define MAX3_VALUE_GET(x, y, z) (MAX2_VALUE_GET(MAX2_VALUE_GET(x, y), z))

#define DIV_FIXED_POINT(x, y)   (((x)+((y)>>1)) / (y)) /* 定点除法 */

DECLARE_FAKE_VALUE_FUNC(uint16_t, averge_calculate_utils, uint16_t *, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint16_t, crc16_calculate_func, uint16_t, const uint8_t *, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, crc32_calculate_func, uint32_t, const uint8_t *, uint32_t);
DECLARE_FAKE_VALUE_FUNC(uint8_t, checksum_calculate_func, uint8_t, const uint8_t *, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, endian_swap_func, uint8_t *, uint16_t);
DECLARE_FAKE_VOID_FUNC(bit_invert_swap_func, void *, uint8_t);

// uint16_t averge_calculate_utils(uint16_t *data, uint16_t length);
// uint16_t crc16_calculate_func(uint16_t crc, const uint8_t *data, uint16_t len);
// uint32_t crc32_calculate_func(uint32_t crc, const uint8_t *data, uint32_t len);
// uint8_t checksum_calculate_func(uint8_t init_sum, const uint8_t *data, uint16_t length);
// uint32_t endian_swap_func(uint8_t *data, uint16_t length);
// void bit_invert_swap_func(void *bit_data, uint8_t bit_length);

#ifdef __cplusplus
}
#endif

#endif /* __UTILITIES_H__ */
