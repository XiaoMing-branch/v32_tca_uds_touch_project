#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"
#include "fff_store_manager.h"

extern void lin_diag_save_configuration(uint8_t *ptr, uint16_t length);
extern l_u8 lin_configured_NAD;

DEFINE_FFF_GLOBALS;


extern sys_cfg_t g_sys_cfgs;

void setUp(void)
{
    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    RESET_FAKE(store_system_data_set);
}
void tearDown(void) {}

void test_save_configuration_sets_nad_from_configured(void)
{
    uint8_t data[] = {0xB6};
    lin_configured_NAD = 0x6A;
    lin_diag_save_configuration(data, sizeof(data));
    TEST_ASSERT_EQUAL(0x6A, g_sys_cfgs.nad);
}

void test_save_configuration_stores_system_cfg(void)
{
    uint8_t data[] = {0xB6};
    lin_diag_save_configuration(data, sizeof(data));
    TEST_ASSERT_EQUAL(SYSTEM_CFG_PARAM, store_system_data_set_fake.arg0_history[0]);
}

void test_save_configuration_sends_positive_notify(void)
{
    uint8_t data[] = {0xB6};
    lin_diag_save_configuration(data, sizeof(data));
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB6, lin_diag_positive_notify_fake.arg0_val);
}

void test_save_configuration_stores_both_params(void)
{
    uint8_t data[] = {0xB6};
    lin_diag_save_configuration(data, sizeof(data));
    TEST_ASSERT_EQUAL(2, store_system_data_set_fake.call_count);
    TEST_ASSERT_EQUAL(SYSTEM_CFG_PARAM, store_system_data_set_fake.arg0_history[0]);
    TEST_ASSERT_EQUAL(SYSTEM_ID_CFG_PARAM, store_system_data_set_fake.arg0_history[1]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_save_configuration_sets_nad_from_configured);
    RUN_TEST(test_save_configuration_stores_system_cfg);
    RUN_TEST(test_save_configuration_sends_positive_notify);
    RUN_TEST(test_save_configuration_stores_both_params);
    return UNITY_END();
}
