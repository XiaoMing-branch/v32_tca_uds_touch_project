#include "unity.h"
#include "fff.h"
#include "fff_lin.h"
#include "fff_store_manager.h"

extern void lin_diag_snpd(uint8_t *ptr, uint16_t length);
extern void lin_snpd_diag_handle(uint8_t *ptr, uint16_t length);

extern l_u8 lin_configured_NAD;

DEFINE_FFF_GLOBALS;

void setUp(void)
{
    FFF_RESET_HISTORY();
    lin_configured_NAD = 0x68;
    g_sys_cfgs.nad = 0x6F;
}

void tearDown(void)
{
}

void test_snpd_no_notify_sent(void)
{
    uint8_t data[] = {0xB5, 0x01, 0x02, 0x03};
    lin_diag_snpd(data, sizeof(data));
}

void test_snpd_null_ptr_no_crash(void)
{
    lin_diag_snpd(NULL, 0);
}

void test_snpd_zero_length_no_crash(void)
{
    uint8_t data[] = {0xB5};
    lin_diag_snpd(data, 0);
}

void test_snpd_large_length_no_crash(void)
{
    uint8_t data[] = {0xB5, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    lin_diag_snpd(data, sizeof(data));
}

void test_snpd_state_unchanged(void)
{
    uint8_t data[] = {0xB5, 0x01, 0x02};
    l_u8 orig_nad = lin_configured_NAD;
    uint32_t orig_sys_nad = g_sys_cfgs.nad;

    lin_diag_snpd(data, sizeof(data));

    TEST_ASSERT_EQUAL_INT8(orig_nad, lin_configured_NAD);
    TEST_ASSERT_EQUAL_UINT32(orig_sys_nad, g_sys_cfgs.nad);
}

void test_snpd_handle_empty_body_executed(void)
{
    uint8_t data[] = {0xB5, 0xAA, 0xBB};
    lin_snpd_diag_handle(data, sizeof(data));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_snpd_no_notify_sent);
    RUN_TEST(test_snpd_null_ptr_no_crash);
    RUN_TEST(test_snpd_zero_length_no_crash);
    RUN_TEST(test_snpd_large_length_no_crash);
    RUN_TEST(test_snpd_state_unchanged);
    RUN_TEST(test_snpd_handle_empty_body_executed);
    return UNITY_END();
}
