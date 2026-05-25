#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"

/*
 * NOTE: fff_diagnosticIII.o is excluded from link for this test (see Makefile).
 * Available fake tracking (from fff_lin.c, linked):
 *   ld_send_message_fake, ld_receive_message_fake, ld_tx_status_fake
 */

extern void setUDSNAD(uint8_t NAD);
extern uint8_t g_bUDSDataDumpFlag;

static l_u16 g_rx_length;
static l_u8 g_rx_data[20];

DEFINE_FFF_GLOBALS;

static void custom_ld_receive_message(l_u16 *length, l_u8 *data)
{
    *length = g_rx_length;
    for (l_u16 i = 0; i < g_rx_length && i < sizeof(g_rx_data); i++)
    {
        data[i] = g_rx_data[i];
    }
}

void setUp(void)
{
    RESET_FAKE(ld_send_message);
    RESET_FAKE(ld_receive_message);
    RESET_FAKE(ld_tx_status);

    g_rx_length = 0;
    memset(g_rx_data, 0, sizeof(g_rx_data));
    ld_receive_message_fake.custom_fake = custom_ld_receive_message;

    g_bUDSDataDumpFlag = 0;

    for (uint8_t i = 0; i < _DIAG_NUMBER_OF_SERVICES_; i++)
    {
        lin_diag_services_flag[i] = 0;
    }
}

void tearDown(void)
{
}

/* ========== setUDSNAD / lin_current_rcvd_nad ========== */

void test_set_nad_and_get(void)
{
    setUDSNAD(0x68);
    uint8_t nad = lin_current_rcvd_nad();
    TEST_ASSERT_EQUAL_UINT8(0x68, nad);
}

void test_set_nad_update(void)
{
    setUDSNAD(0x7E);
    uint8_t nad = lin_current_rcvd_nad();
    TEST_ASSERT_EQUAL_UINT8(0x7E, nad);
}

void test_set_nad_zero(void)
{
    setUDSNAD(0x00);
    uint8_t nad = lin_current_rcvd_nad();
    TEST_ASSERT_EQUAL_UINT8(0x00, nad);
}

/* ========== lin_diag_positive_notify ========== */

void test_positive_notify_calls_send_message(void)
{
    uint8_t data[] = {0x01, 0x02, 0x03};
    lin_diag_positive_notify(0xB2, data, 3);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL(4, ld_send_message_fake.arg0_val);
}

void test_positive_notify_zero_length(void)
{
    lin_diag_positive_notify(0x10, NULL, 0);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.arg0_val);
}

/* ========== lin_diag_negative_notify ========== */

void test_negative_notify_calls_send_message(void)
{
    lin_diag_negative_notify(0x11, 0x11);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL(3, ld_send_message_fake.arg0_val);
}

void test_negative_notify_different_sid(void)
{
    lin_diag_negative_notify(0x22, 0x31);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL(3, ld_send_message_fake.arg0_val);
}

/* ========== lin_diag_service_handle ========== */

void test_handle_no_service_flagged(void)
{
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(0, ld_receive_message_fake.call_count);
}

void test_handle_data_dump(void)
{
    lin_diag_services_flag[20] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, g_bUDSDataDumpFlag);
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[20]);
}

void test_handle_read_by_identifier(void)
{
    lin_diag_services_flag[18] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[18]);
}

void test_handle_io_control(void)
{
    lin_diag_services_flag[8] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[8]);
}

void test_handle_assign_nad(void)
{
    lin_diag_services_flag[16] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[16]);
}

void test_handle_assign_frame_id(void)
{
    lin_diag_services_flag[17] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL_MESSAGE(1, ld_receive_message_fake.call_count, "ld_receive_message should be called");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, lin_diag_services_flag[17], "flag should be cleared");
}

void test_handle_cond_change_nad(void)
{
    lin_diag_services_flag[19] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[19]);
}

void test_handle_save_config(void)
{
    lin_diag_services_flag[22] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[22]);
}

void test_handle_assign_frame_id_range(void)
{
    lin_diag_services_flag[23] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[23]);
}

void test_handle_default_service(void)
{
    lin_diag_services_flag[0] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[0]);
}

void test_handle_get_traceability_msg(void)
{
    uint8_t saved = lin_diag_services_supported[0];
    lin_diag_services_supported[0] = SERVICE_GET_TRACEABILITY_MSG;
    lin_diag_services_flag[0] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(1, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[0]);
    lin_diag_services_supported[0] = saved;
}

void test_handle_multiple_services(void)
{
    lin_diag_services_flag[18] = 1;
    lin_diag_services_flag[20] = 1;
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(2, ld_receive_message_fake.call_count);
    TEST_ASSERT_EQUAL(1, g_bUDSDataDumpFlag);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[18]);
    TEST_ASSERT_EQUAL_UINT8(0, lin_diag_services_flag[20]);
}

void test_handle_all_flags_cleared_after_processing(void)
{
    for (uint8_t i = 0; i < _DIAG_NUMBER_OF_SERVICES_; i++)
    {
        lin_diag_services_flag[i] = (i % 2 == 0) ? 1 : 0;
    }
    uint8_t expected_call_count = 0;
    for (uint8_t i = 0; i < _DIAG_NUMBER_OF_SERVICES_; i++)
    {
        if ((i % 2) == 0) expected_call_count++;
    }
    lin_diag_service_handle();
    TEST_ASSERT_EQUAL(expected_call_count, ld_receive_message_fake.call_count);
    for (uint8_t i = 0; i < _DIAG_NUMBER_OF_SERVICES_; i++)
    {
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, lin_diag_services_flag[i], "flag should be cleared after processing");
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_set_nad_and_get);
    RUN_TEST(test_set_nad_update);
    RUN_TEST(test_set_nad_zero);
    RUN_TEST(test_positive_notify_calls_send_message);
    RUN_TEST(test_positive_notify_zero_length);
    RUN_TEST(test_negative_notify_calls_send_message);
    RUN_TEST(test_negative_notify_different_sid);
    RUN_TEST(test_handle_no_service_flagged);
    RUN_TEST(test_handle_data_dump);
    RUN_TEST(test_handle_read_by_identifier);
    RUN_TEST(test_handle_io_control);
    RUN_TEST(test_handle_assign_nad);
    RUN_TEST(test_handle_assign_frame_id);
    RUN_TEST(test_handle_cond_change_nad);
    RUN_TEST(test_handle_save_config);
    RUN_TEST(test_handle_assign_frame_id_range);
    RUN_TEST(test_handle_default_service);
    RUN_TEST(test_handle_get_traceability_msg);
    RUN_TEST(test_handle_multiple_services);
    RUN_TEST(test_handle_all_flags_cleared_after_processing);
    return UNITY_END();
}
