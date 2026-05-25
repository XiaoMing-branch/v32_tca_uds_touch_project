#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"

extern void lin_diag_tester_present(uint8_t *ptr, uint16_t length);

DEFINE_FFF_GLOBALS;

void setUp(void)
{
    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    RESET_FAKE(lin_diag_negative_notify);
}
void tearDown(void) {}

void test_tester_present_subfunc_0x00_sends_positive(void)
{
    uint8_t data[] = {0x3E, 0x00};
    lin_diag_tester_present(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x3E, lin_diag_positive_notify_fake.arg0_val);
}

void test_tester_present_subfunc_0x80_suppresses_response(void)
{
    uint8_t data[] = {0x3E, 0x80};
    lin_diag_tester_present(data, sizeof(data));
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_tester_present_subfunc_0x01_returns_sfns(void)
{
    uint8_t data[] = {0x3E, 0x01};
    lin_diag_tester_present(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x3E, lin_diag_negative_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(SFNS, lin_diag_negative_notify_fake.arg1_val);
}

void test_tester_present_subfunc_0xff_returns_sfns(void)
{
    uint8_t data[] = {0x3E, 0xFF};
    lin_diag_tester_present(data, sizeof(data));
    TEST_ASSERT_EQUAL(SFNS, lin_diag_negative_notify_fake.arg1_val);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_tester_present_subfunc_0x00_sends_positive);
    RUN_TEST(test_tester_present_subfunc_0x80_suppresses_response);
    RUN_TEST(test_tester_present_subfunc_0x01_returns_sfns);
    RUN_TEST(test_tester_present_subfunc_0xff_returns_sfns);
    return UNITY_END();
}
