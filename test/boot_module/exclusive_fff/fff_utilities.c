#include "fff_utilities.h"
DEFINE_FAKE_VALUE_FUNC(uint16_t, averge_calculate_utils, uint16_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint16_t, crc16_calculate_func, uint16_t, const uint8_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, crc32_calculate_func, uint32_t, const uint8_t *, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, endian_swap_func, uint8_t *, uint16_t);
DEFINE_FAKE_VOID_FUNC(bit_invert_swap_func, void *, uint16_t);