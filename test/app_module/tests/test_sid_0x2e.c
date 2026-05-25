#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"

extern void lin_diag_write_by_identifier(uint8_t *ptr, uint16_t length);

DEFINE_FFF_GLOBALS;

void setUp(void) { FFF_RESET_HISTORY(); RESET_FAKE(lin_diag_positive_notify); }
void tearDown(void) {}

void test_write_by_identifier_calls_positive_notify_with_null(void)
{
    uint8_t data[] = {0x2E, 0x01, 0x02, 0x03, 0x04, 0x05};
    lin_diag_write_by_identifier(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x2E, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_NULL(lin_diag_positive_notify_fake.arg1_val);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.arg2_val);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_write_by_identifier_calls_positive_notify_with_null);
    return UNITY_END();
}
