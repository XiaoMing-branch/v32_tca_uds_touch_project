#include "fff.h"
#include "unity.h"
#include "utilities.h"

DEFINE_FFF_GLOBALS;

void setUp(void)
{
}

void tearDown(void)
{
}

/* ========== averge_calculate_utils ========== */

void test_averge_len0_returns_zero(void)
{
    uint16_t data[1] = {0};
    uint16_t avg = averge_calculate_utils(data, 0);
    TEST_ASSERT_EQUAL_UINT16(0, avg);
}

void test_averge_len1_returns_value(void)
{
    uint16_t data[1] = {100};
    uint16_t avg = averge_calculate_utils(data, 1);
    TEST_ASSERT_EQUAL_UINT16(100, avg);
}

void test_averge_len2_averages(void)
{
    uint16_t data[2] = {50, 150};
    uint16_t avg = averge_calculate_utils(data, 2);
    TEST_ASSERT_EQUAL_UINT16(100, avg);
}

void test_averge_len3_removes_min_max(void)
{
    uint16_t data[3] = {1, 2, 100};
    uint16_t avg = averge_calculate_utils(data, 3);
    TEST_ASSERT_EQUAL_UINT16(2, avg);
}

void test_averge_len4_removes_min_max(void)
{
    uint16_t data[4] = {10, 20, 30, 40};
    uint16_t avg = averge_calculate_utils(data, 4);
    TEST_ASSERT_EQUAL_UINT16(25, avg);
}

void test_averge_len4_all_same(void)
{
    uint16_t data[4] = {100, 100, 100, 100};
    uint16_t avg = averge_calculate_utils(data, 4);
    TEST_ASSERT_EQUAL_UINT16(100, avg);
}

/* ========== checksum_calculate_func ========== */

void test_checksum_no_carry(void)
{
    uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t result = checksum_calculate_func(0x00, data, 8);
    TEST_ASSERT_EQUAL_UINT8(0xDB, result);
}

void test_checksum_with_carry(void)
{
    uint8_t data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t result = checksum_calculate_func(0x01, data, 8);
    TEST_ASSERT_EQUAL_UINT8(0xFE, result);
}

void test_checksum_zero_data(void)
{
    uint8_t data[8] = {0};
    uint8_t result = checksum_calculate_func(0xFF, data, 8);
    TEST_ASSERT_EQUAL_UINT8(0x00, result);
}

void test_checksum_single_byte_data(void)
{
    uint8_t data[8] = {0x10, 0, 0, 0, 0, 0, 0, 0};
    uint8_t result = checksum_calculate_func(0x00, data, 8);
    TEST_ASSERT_EQUAL_UINT8(0xEF, result);
}

/* ========== crc32_calculate_func ========== */

void test_crc32_len0_returns_init(void)
{
    uint8_t data[1] = {0};
    uint32_t crc = crc32_calculate_func(0xFFFFFFFFu, data, 0);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFu, crc);
}

void test_crc32_single_byte(void)
{
    uint8_t data[1] = {0xAB};
    uint32_t crc = crc32_calculate_func(0xFFFFFFFFu, data, 1);
    crc32_calculate_func(0x00000000u, data, 1);
    uint32_t c2 = crc32_calculate_func(0xFFFFFFFFu, data, 1);
    crc = c2;
    (void)crc;
}

void test_crc32_multi_byte(void)
{
    uint8_t data[32];
    for (uint8_t i = 0; i < 8; i++) data[i] = i;
    crc32_calculate_func(0x00000000u, data, 8);

    for (uint8_t i = 0; i < 32; i++) data[i] = i * 3u;
    crc32_calculate_func(0x12345678u, data, 32);

    for (uint8_t i = 0; i < 16; i++) data[i] = 0xFFu;
    crc32_calculate_func(0xFFFFFFFFu, data, 16);
}

void test_crc32_init_zero_len0(void)
{
    uint8_t data[1] = {0};
    uint32_t crc = crc32_calculate_func(0u, data, 0);
    TEST_ASSERT_EQUAL_UINT32(0, crc);
}

/* ========== crc16_calculate_func ========== */

void test_crc16_len0_returns_inverted_init(void)
{
    uint8_t data[1] = {0};
    uint16_t crc = crc16_calculate_func(0xFFFFu, data, 0);
    TEST_ASSERT_EQUAL_UINT16(0x0000u, crc);
}

void test_crc16_single_byte(void)
{
    uint8_t data[1] = {0xAB};
    crc16_calculate_func(0xFFFFu, data, 1);
    crc16_calculate_func(0x0000u, data, 1);
}

void test_crc16_multi_byte(void)
{
    uint8_t data[32];
    for (uint8_t i = 0; i < 8; i++) data[i] = i;
    crc16_calculate_func(0x0000u, data, 8);

    for (uint8_t i = 0; i < 32; i++) data[i] = i * 3u;
    crc16_calculate_func(0x1234u, data, 32);

    for (uint8_t i = 0; i < 16; i++) data[i] = 0xFFu;
    crc16_calculate_func(0xFFFFu, data, 16);
}

void test_crc16_init_zero_len0(void)
{
    uint8_t data[1] = {0};
    uint16_t crc = crc16_calculate_func(0u, data, 0);
    TEST_ASSERT_EQUAL_UINT16(0xFFFFu, crc);
}

void test_crc16_four_bytes(void)
{
    uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};
    crc16_calculate_func(0xFFFFu, data, 4);
}

/* ========== endian_swap_func ========== */

void test_endian_swap_len0(void)
{
    uint8_t data[4] = {0};
    endian_swap_func(data, 0);
}

void test_endian_swap_len1(void)
{
    uint8_t data[1] = {0xAB};
    endian_swap_func(data, 1);
    TEST_ASSERT_EQUAL_UINT8(0xAB, data[0]);
}

void test_endian_swap_len2(void)
{
    uint8_t data[2] = {0x12, 0x34};
    endian_swap_func(data, 2);
    TEST_ASSERT_EQUAL_UINT8(0x34, data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x12, data[1]);
}

void test_endian_swap_len4(void)
{
    uint8_t data[4] = {0x12, 0x34, 0x56, 0x78};
    uint32_t result = endian_swap_func(data, 4);
    TEST_ASSERT_EQUAL_UINT32(0x12345678, result);
}

void test_endian_swap_len7(void)
{
    uint8_t data[7] = {10, 11, 12, 13, 14, 15, 16};
    uint32_t result = endian_swap_func(data, 7);
    (void)result;
    TEST_ASSERT_EQUAL_UINT8(16, data[0]);
    TEST_ASSERT_EQUAL_UINT8(15, data[1]);
    TEST_ASSERT_EQUAL_UINT8(14, data[2]);
    TEST_ASSERT_EQUAL_UINT8(13, data[3]);
    TEST_ASSERT_EQUAL_UINT8(12, data[4]);
    TEST_ASSERT_EQUAL_UINT8(11, data[5]);
    TEST_ASSERT_EQUAL_UINT8(10, data[6]);
}

/* ========== bit_invert_swap_func ========== */

void test_bit_invert_invalid_len_does_nothing(void)
{
    uint32_t data = 0x12345678;
    bit_invert_swap_func(&data, 24);
    TEST_ASSERT_EQUAL_UINT32(0x12345678, data);
}

void test_bit_invert_8bit(void)
{
    uint8_t d8 = 0x81;
    bit_invert_swap_func(&d8, 8);
    TEST_ASSERT_EQUAL_UINT8(0x81, d8);
}

void test_bit_invert_16bit(void)
{
    uint16_t d16 = 0x1234;
    bit_invert_swap_func(&d16, 16);
    TEST_ASSERT_EQUAL_UINT16(0x2C48, d16);
}

void test_bit_invert_32bit(void)
{
    uint32_t d32 = 0x12345678;
    bit_invert_swap_func(&d32, 32);
    TEST_ASSERT_EQUAL_UINT32(0x1E6A2C48, d32);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_averge_len0_returns_zero);
    RUN_TEST(test_averge_len1_returns_value);
    RUN_TEST(test_averge_len2_averages);
    RUN_TEST(test_averge_len3_removes_min_max);
    RUN_TEST(test_averge_len4_removes_min_max);
    RUN_TEST(test_averge_len4_all_same);

    RUN_TEST(test_checksum_no_carry);
    RUN_TEST(test_checksum_with_carry);
    RUN_TEST(test_checksum_zero_data);
    RUN_TEST(test_checksum_single_byte_data);

    RUN_TEST(test_crc32_len0_returns_init);
    RUN_TEST(test_crc32_single_byte);
    RUN_TEST(test_crc32_multi_byte);
    RUN_TEST(test_crc32_init_zero_len0);

    RUN_TEST(test_crc16_len0_returns_inverted_init);
    RUN_TEST(test_crc16_single_byte);
    RUN_TEST(test_crc16_multi_byte);
    RUN_TEST(test_crc16_init_zero_len0);
    RUN_TEST(test_crc16_four_bytes);

    RUN_TEST(test_endian_swap_len0);
    RUN_TEST(test_endian_swap_len1);
    RUN_TEST(test_endian_swap_len2);
    RUN_TEST(test_endian_swap_len4);
    RUN_TEST(test_endian_swap_len7);

    RUN_TEST(test_bit_invert_invalid_len_does_nothing);
    RUN_TEST(test_bit_invert_8bit);
    RUN_TEST(test_bit_invert_16bit);
    RUN_TEST(test_bit_invert_32bit);

    return UNITY_END();
}
