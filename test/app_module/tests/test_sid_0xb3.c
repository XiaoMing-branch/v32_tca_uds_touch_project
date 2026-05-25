#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"

extern void lin_diag_conditional_change_nad(uint8_t *ptr, uint16_t length);
extern l_u8 lin_configured_NAD;

DEFINE_FFF_GLOBALS;

void setUp(void)
{
    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    lin_configured_NAD = 0x68;
}
void tearDown(void) {}

void test_conditional_change_nad_id_nonzero_does_nothing(void)
{
    uint8_t data[] = {0xB3, 0x01, 0x00, 0x00, 0x00, 0x6A};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x68, lin_configured_NAD);
}

void test_conditional_change_nad_byte_zero_does_nothing(void)
{
    uint8_t data[] = {0xB3, 0x00, 0x00, 0xFF, 0x00, 0x6A};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
}

void test_conditional_change_nad_byte_six_does_nothing(void)
{
    uint8_t data[] = {0xB3, 0x00, 0x06, 0xFF, 0x00, 0x6A};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
}

void test_conditional_change_nad_byte1_mask_zero_always_matches(void)
{
    /* mask=0 => result always 0 => NAD changed */
    uint8_t data[] = {0xB3, 0x00, 0x01, 0x00, 0xFF, 0x6C};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB3, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x6C, lin_configured_NAD);
}

void test_conditional_change_nad_byte5_variant_matches(void)
{
    /* product_id.variant=0 so byte=0, invert=0, mask=0xFF => result=0 => match */
    uint8_t data[] = {0xB3, 0x00, 0x05, 0xFF, 0x00, 0x6B};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x6B, lin_configured_NAD);
}

void test_conditional_change_nad_byte3_func_lsb_match(void)
{
    product_id.function_id = 0x0078;
    uint8_t data[] = {0xB3, 0x00, 0x03, 0xFF, 0x78, 0x6C};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB3, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x6C, lin_configured_NAD);
}

void test_conditional_change_nad_byte4_func_msb_path(void)
{
    product_id.function_id = 0x5678;
    uint8_t data[] = {0xB3, 0x00, 0x04, 0xFF, 0x00, 0x6D};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB3, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x6D, lin_configured_NAD);
}

void test_conditional_change_nad_byte1_no_match(void)
{
    product_id.supplier_id = 0x0034;
    uint8_t data[] = {0xB3, 0x00, 0x01, 0xFF, 0x00, 0x6E};
    lin_diag_conditional_change_nad(data, sizeof(data));
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x68, lin_configured_NAD);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_conditional_change_nad_id_nonzero_does_nothing);
    RUN_TEST(test_conditional_change_nad_byte_zero_does_nothing);
    RUN_TEST(test_conditional_change_nad_byte_six_does_nothing);
    RUN_TEST(test_conditional_change_nad_byte1_mask_zero_always_matches);
    RUN_TEST(test_conditional_change_nad_byte5_variant_matches);
    RUN_TEST(test_conditional_change_nad_byte3_func_lsb_match);
    RUN_TEST(test_conditional_change_nad_byte4_func_msb_path);
    RUN_TEST(test_conditional_change_nad_byte1_no_match);
    return UNITY_END();
}
