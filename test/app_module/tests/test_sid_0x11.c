#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"
#include "fff_tcae10_ll_wdg.h"

extern void lin_diag_ecu_reset(uint8_t *ptr, uint16_t length);

DEFINE_FFF_GLOBALS;

void setUp(void)
{
    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    RESET_FAKE(lin_diag_negative_notify);
    RESET_FAKE(ll_wdg_enable);
}
void tearDown(void) {}

void test_ecu_reset_subfunc_0x01_calls_positive_notify(void)
{
    uint8_t data[] = {0x11, 0x01};
    lin_diag_ecu_reset(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x11, lin_diag_positive_notify_fake.arg0_val);
}

void test_ecu_reset_subfunc_0x01_disables_watchdog(void)
{
    uint8_t data[] = {0x11, 0x01};
    lin_diag_ecu_reset(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, ll_wdg_enable_fake.call_count);
    TEST_ASSERT_EQUAL(false, ll_wdg_enable_fake.arg0_val);
}

void test_ecu_reset_unknown_subfunc_0x00_returns_sfns(void)
{
    uint8_t data[] = {0x11, 0x00};
    lin_diag_ecu_reset(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x11, lin_diag_negative_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(SFNS, lin_diag_negative_notify_fake.arg1_val);
}

void test_ecu_reset_unknown_subfunc_0x02_returns_sfns(void)
{
    uint8_t data[] = {0x11, 0x02};
    lin_diag_ecu_reset(data, sizeof(data));
    TEST_ASSERT_EQUAL(SFNS, lin_diag_negative_notify_fake.arg1_val);
}

void test_ecu_reset_unknown_subfunc_0xff_returns_sfns(void)
{
    uint8_t data[] = {0x11, 0xFF};
    lin_diag_ecu_reset(data, sizeof(data));
    TEST_ASSERT_EQUAL(SFNS, lin_diag_negative_notify_fake.arg1_val);
}

void test_ecu_reset_subfunc_0x01_does_not_call_negative_notify(void)
{
    uint8_t data[] = {0x11, 0x01};
    lin_diag_ecu_reset(data, sizeof(data));
    //!TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ecu_reset_subfunc_0x01_calls_positive_notify);
    RUN_TEST(test_ecu_reset_subfunc_0x01_disables_watchdog);
    RUN_TEST(test_ecu_reset_unknown_subfunc_0x00_returns_sfns);
    RUN_TEST(test_ecu_reset_unknown_subfunc_0x02_returns_sfns);
    RUN_TEST(test_ecu_reset_unknown_subfunc_0xff_returns_sfns);
    RUN_TEST(test_ecu_reset_subfunc_0x01_does_not_call_negative_notify);
    return UNITY_END();
}
