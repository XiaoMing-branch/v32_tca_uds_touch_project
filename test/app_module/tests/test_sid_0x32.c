#include "unity.h"
#include "fff.h"

extern void lin_diag_get_traceability_msg(uint8_t *ptr, uint16_t length);

DEFINE_FFF_GLOBALS;

void setUp(void)  { FFF_RESET_HISTORY(); }
void tearDown(void) {}

void test_get_traceability_msg_empty_body_does_not_crash(void)
{
    uint8_t data[] = {0x32, 0x01, 0x02, 0x03, 0x04, 0x05};
    lin_diag_get_traceability_msg(data, sizeof(data));
    TEST_ASSERT_EQUAL(0x32, data[0]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_get_traceability_msg_empty_body_does_not_crash);
    return UNITY_END();
}
