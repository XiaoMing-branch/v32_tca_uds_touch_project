/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   utilities header file.
 *
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

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX2_VALUE_GET(x, y) ((x > y) ? x : y)

#define MIN2_VALUE_GET(x, y) ((x < y) ? x : y)

#define CLAMP_VALUE_GET(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define MAX3_VALUE_GET(x, y, z) (MAX2_VALUE_GET(MAX2_VALUE_GET(x, y), z))

#define DIV_FIXED_POINT(x, y) (((x) + ((y) >> 1)) / (y)) /* Fixed-point division */

    uint16_t averge_calculate_utils(uint16_t *data, uint16_t length);
    uint16_t crc16_calculate_func(uint16_t init_crc, const uint8_t *data, uint16_t length);
    uint32_t crc32_calculate_func(uint32_t init_crc, const uint8_t *data, uint32_t length);
    uint8_t checksum_calculate_func(uint8_t init_sum, const uint8_t *data, uint16_t length);
    uint32_t endian_swap_func(uint8_t *data, uint16_t length);
    void bit_invert_swap_func(void *bit_data, uint8_t bit_length);
#ifdef __cplusplus
}
#endif

#endif /* __UTILITIES_H__ */
/* PRQA S 1534 -- */
