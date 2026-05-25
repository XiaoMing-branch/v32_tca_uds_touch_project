#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"
#include "fff_lin_precfg.h"

extern void lin_diag_assign_frame_id_range(uint8_t *ptr, uint16_t length);

DEFINE_FFF_GLOBALS;


extern lin_precfg_t lin_cfg;

void setUp(void)
{
    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    RESET_FAKE(lin_diag_negative_notify);
    RESET_FAKE(lin_process_parity);
    lin_process_parity_fake.return_val = 0x42;
    lin_cfg.lin_cfg_frame_num = 20;
}
void tearDown(void) {}

void test_assign_frame_id_range_length_not_6_rejects(void)
{
    uint8_t data[] = {0xB7, 0x01, 0x02, 0x03, 0x04};
    lin_diag_assign_frame_id_range(data, 5);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(GENERAL_REJECT, lin_diag_negative_notify_fake.arg1_val);
}

void test_assign_frame_id_range_all_ff_keep_previous(void)
{
    uint8_t data[] = {0xB7, 0x01, 0xFF, 0xFF, 0xFF, 0xFF};
    lin_diag_assign_frame_id_range(data, 6);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_assign_frame_id_range_pid_00_unassigns(void)
{
    uint8_t data[] = {0xB7, 0x02, 0x00, 0xFF, 0xFF, 0xFF};
    lin_configuration_RAM[3] = 0xAA;
    lin_diag_assign_frame_id_range(data, 6);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xFF, lin_configuration_RAM[3]);
}

void test_assign_frame_id_range_default_pid_calls_parity(void)
{
    uint8_t data[] = {0xB7, 0x00, 0x55, 0xFF, 0xFF, 0xFF};
    lin_process_parity_fake.return_val = 0x77;
    lin_diag_assign_frame_id_range(data, 6);
    TEST_ASSERT_EQUAL(1, lin_process_parity_fake.call_count);
    TEST_ASSERT_EQUAL(0x55, lin_process_parity_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x77, lin_configuration_RAM[1]);
}

void test_assign_frame_id_range_parity_ff_rejects(void)
{
    uint8_t data[] = {0xB7, 0x00, 0x55, 0xFF, 0xFF, 0xFF};
    lin_process_parity_fake.return_val = 0xFF;
    lin_diag_assign_frame_id_range(data, 6);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(GENERAL_REJECT, lin_diag_negative_notify_fake.arg1_val);
}

void test_assign_frame_id_range_index_out_of_range_silent_return(void)
{
    lin_cfg.lin_cfg_frame_num = 2;
    uint8_t data[] = {0xB7, 0x02, 0x42, 0xFF, 0xFF, 0xFF};
    lin_diag_assign_frame_id_range(data, 6);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_assign_frame_id_range_length_not_6_rejects);
    RUN_TEST(test_assign_frame_id_range_all_ff_keep_previous);
    RUN_TEST(test_assign_frame_id_range_pid_00_unassigns);
    RUN_TEST(test_assign_frame_id_range_default_pid_calls_parity);
    RUN_TEST(test_assign_frame_id_range_parity_ff_rejects);
    RUN_TEST(test_assign_frame_id_range_index_out_of_range_silent_return);
    return UNITY_END();
}
