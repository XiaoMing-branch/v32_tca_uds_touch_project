#ifndef __FFF_UTILITIES_H__
#define __FFF_UTILITIES_H__

#include "fff.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX2_VALUE_GET(x, y) ((x > y) ? x : y)

#define MIN2_VALUE_GET(x, y) ((x < y) ? x : y)

#define MAX3_VALUE_GET(x, y, z) (MAX2_VALUE_GET(MAX2_VALUE_GET(x, y), z))

DECLARE_FAKE_VALUE_FUNC(uint16_t, averge_calculate_utils, uint16_t *, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint16_t, crc16_calculate_func, uint16_t, const uint8_t *, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, crc32_calculate_func, uint32_t, const uint8_t *, uint32_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, endian_swap_func, uint8_t *, uint16_t);
DECLARE_FAKE_VOID_FUNC(bit_invert_swap_func, void *, uint16_t);

// uint16_t averge_calculate_utils(uint16_t *data, uint16_t length);
// uint16_t crc16_calculate_func(uint16_t crc, const uint8_t *data, uint16_t len);
// uint32_t crc32_calculate_func(uint32_t crc, const uint8_t *data, uint32_t len);
// uint32_t endian_swap_func(uint8_t *data, uint16_t length);
// void bit_invert_swap_func(void *bit_data, uint16_t bit_length);

#ifdef __cplusplus
}
#endif

#endif /* __FFF_UTILITIES_H__ */
