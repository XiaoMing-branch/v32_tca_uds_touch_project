#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"
#include "fff_lin.h"

extern __WEAK void lin_diagservice_read_by_identifier(uint8_t *ptr, uint16_t length);

extern l_u8 lin_configured_NAD;

DEFINE_FFF_GLOBALS;

static l_u8 ld_read_by_id_callout_custom_fake(l_u8 id, l_u8 *data)
{
    data[0] = 0x01;
    return LD_POSITIVE_RESPONSE;
}

void setUp(void)
{
    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    RESET_FAKE(lin_diag_negative_notify);
    RESET_FAKE(ld_read_by_id_callout);

    product_id.supplier_id = 0x1234;
    product_id.function_id = 0x5678;
    product_id.variant     = 0x99;
    tl_slaveresp_cnt = 0;
    tl_service_status = 0;
}

void tearDown(void)
{
}

void test_supplier_id_mismatch_returns_early(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x00; ptr[3] = 0x01;
    ptr[4] = 0x78; ptr[5] = 0x56;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(0, tl_slaveresp_cnt);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_function_id_mismatch_returns_early(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x00; ptr[5] = 0x01;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(0, tl_slaveresp_cnt);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_any_id_pass_check_continues(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0xFF; ptr[3] = 0x7F;
    ptr[4] = 0xFF; ptr[5] = 0xFF;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(0x34, ptr[1]);
    TEST_ASSERT_EQUAL(0x12, ptr[2]);
    TEST_ASSERT_EQUAL(0x78, ptr[3]);
    TEST_ASSERT_EQUAL(0x56, ptr[4]);
    TEST_ASSERT_EQUAL(0x99, ptr[5]);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
}

void test_product_identifier_fills_data(void)
{
    uint8_t ptr[10] = {0};
    ptr[0] = 0xB2;
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = LIN_PRODUCT_IDENT;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(0x34, ptr[1]);
    TEST_ASSERT_EQUAL(0x12, ptr[2]);
    TEST_ASSERT_EQUAL(0x78, ptr[3]);
    TEST_ASSERT_EQUAL(0x56, ptr[4]);
    TEST_ASSERT_EQUAL(0x99, ptr[5]);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB2, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL_PTR(&ptr[1], lin_diag_positive_notify_fake.arg1_val);
    TEST_ASSERT_EQUAL(5, lin_diag_positive_notify_fake.arg2_val);
}

void test_serial_number_returns_negative(void)
{
    uint8_t ptr[10] = {0};
    ptr[0] = 0xB2;
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = SERIAL_NUMBER;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB2, lin_diag_negative_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(SUBFUNCTION_NOT_SUPPORTED, lin_diag_negative_notify_fake.arg1_val);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
}

void test_default_id_out_of_range_returns_negative(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = 0x10;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(SUBFUNCTION_NOT_SUPPORTED, lin_diag_negative_notify_fake.arg1_val);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
}

void test_usr_id_positive_response(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = 0x20;

    ld_read_by_id_callout_fake.custom_fake = ld_read_by_id_callout_custom_fake;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(1, ld_read_by_id_callout_fake.call_count);
    TEST_ASSERT_EQUAL(0x20, ld_read_by_id_callout_fake.arg0_val);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.arg2_val);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_usr_id_all_ff_data_returns_negative(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = 0x25;

    ld_read_by_id_callout_fake.return_val = LD_POSITIVE_RESPONSE;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(1, ld_read_by_id_callout_fake.call_count);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(SUBFUNCTION_NOT_SUPPORTED, lin_diag_negative_notify_fake.arg1_val);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
}

void test_usr_id_negative_response(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = 0x30;

    ld_read_by_id_callout_fake.return_val = LD_NEGATIVE_RESPONSE;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(1, ld_read_by_id_callout_fake.call_count);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    TEST_ASSERT_EQUAL(SUBFUNCTION_NOT_SUPPORTED, lin_diag_negative_notify_fake.arg1_val);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
}

void test_usr_id_no_response(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = 0x2F;

    ld_read_by_id_callout_fake.return_val = LD_ID_NO_RESPONSE;

    lin_diagservice_read_by_identifier(ptr, 10);

    TEST_ASSERT_EQUAL(1, ld_read_by_id_callout_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_lin_diagservice_read_by_identifier0(void)
{
    uint8_t ptr[10] = {0};
    ptr[2] = 0x34; ptr[3] = 0x12;
    ptr[4] = 0x78; ptr[5] = 0x56;
    ptr[1] = 64;

    ld_read_by_id_callout_fake.return_val = LD_ID_NO_RESPONSE;

    lin_diagservice_read_by_identifier(ptr, 10);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_supplier_id_mismatch_returns_early);
    RUN_TEST(test_function_id_mismatch_returns_early);
    RUN_TEST(test_any_id_pass_check_continues);
    RUN_TEST(test_product_identifier_fills_data);
    RUN_TEST(test_serial_number_returns_negative);
    RUN_TEST(test_default_id_out_of_range_returns_negative);
    RUN_TEST(test_usr_id_positive_response);
    RUN_TEST(test_usr_id_all_ff_data_returns_negative);
    RUN_TEST(test_usr_id_negative_response);
    RUN_TEST(test_usr_id_no_response);
    RUN_TEST(test_lin_diagservice_read_by_identifier0);

    return UNITY_END();
}
