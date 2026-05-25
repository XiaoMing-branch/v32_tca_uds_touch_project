#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"

extern void lin_diag_assign_frame_identifier(uint8_t *ptr, uint16_t length);
extern l_u8 lin_configuration_RAM[];
extern l_u16 lin_configuration_ROM[];
extern lin_product_id product_id;
extern l_u8 tl_slaveresp_cnt;
extern lin_service_status tl_service_status;
extern l_u8 lin_pFrameBuf[];
extern l_bool lin_frame_flag_tbl[];

DEFINE_FFF_GLOBALS;

void setUp(void)
{
    int i;
    l_u8 ram_default[LIN_SIZE_OF_CFG] = {0x00, 0x11, 0x13, 0x14, 0x12, 0x01, 0x3C, 0x3D, 0xFF};

    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    RESET_FAKE(lin_process_parity);

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    product_id.variant = 0x00;
    tl_slaveresp_cnt = 0;
    tl_service_status = LD_SERVICE_IDLE;

    for (i = 0; i < LIN_SIZE_OF_CFG; i++)
    {
        lin_configuration_RAM[i] = ram_default[i];
    }

    lin_pFrameBuf[8] = 0x00;
    lin_pFrameBuf[9] = 0x00;
    lin_frame_flag_tbl[1] = 1;
}

void tearDown(void) {}

void test_delete_not_found(void)
{
    uint8_t data[] = {0xB1, 0x00, 0x00, 0xFF, 0xFF, 0x40};

    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x11, lin_configuration_RAM[1]);
}

void test_delete_found(void)
{
    uint8_t data[] = {0xB1, 0x00, 0x00, 0x11, 0x00, 0x40};

    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB1, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0xFF, lin_configuration_RAM[1]);
}

void test_assign_success(void)
{
    uint8_t data[] = {0xB1, 0x00, 0x00, 0x11, 0x00, 0x01};

    lin_process_parity_fake.return_val = 0x11;
    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, lin_process_parity_fake.call_count);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB1, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x11, lin_configuration_RAM[1]);
}

void test_supplier_not_match(void)
{
    uint8_t data[] = {0xB1, 0x34, 0x12, 0x11, 0x00, 0x22};

    product_id.supplier_id = 0x5678;
    lin_process_parity_fake.return_val = 0x22;
    tl_slaveresp_cnt = 5;
    tl_service_status = LD_SERVICE_BUSY;

    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, tl_slaveresp_cnt);
    TEST_ASSERT_EQUAL(LD_SERVICE_IDLE, tl_service_status);
}

void test_id_conflict(void)
{
    uint8_t data[] = {0xB1, 0x00, 0x00, 0x13, 0x00, 0x01};

    lin_process_parity_fake.return_val = 0x11;
    tl_slaveresp_cnt = 5;
    tl_service_status = LD_SERVICE_BUSY;

    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, tl_slaveresp_cnt);
    TEST_ASSERT_EQUAL(LD_SERVICE_IDLE, tl_service_status);
}

void test_assign_any_supplier(void)
{
    uint8_t data[] = {0xB1, 0xFF, 0x7F, 0x11, 0x00, 0x01};

    product_id.supplier_id = 0x5678;
    lin_process_parity_fake.return_val = 0x11;

    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB1, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x11, lin_configuration_RAM[1]);
}

void test_assign_message_not_found(void)
{
    uint8_t data[] = {0xB1, 0x00, 0x00, 0x55, 0x00, 0x01};

    lin_process_parity_fake.return_val = 0x55;
    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x11, lin_configuration_RAM[1]);
}

void test_assign_zero_id(void)
{
    uint8_t data[] = {0xB1, 0x00, 0x00, 0x11, 0x00, 0x01};

    lin_process_parity_fake.return_val = 0x00;
    lin_diag_assign_frame_identifier(data, sizeof(data));

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB1, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x00, lin_configuration_RAM[1]);
    TEST_ASSERT_EQUAL(0xaa, lin_pFrameBuf[8]);
    TEST_ASSERT_EQUAL(0xbb, lin_pFrameBuf[9]);
    TEST_ASSERT_EQUAL(0, lin_frame_flag_tbl[1]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_delete_not_found);
    RUN_TEST(test_delete_found);
    RUN_TEST(test_assign_success);
    RUN_TEST(test_supplier_not_match);
    RUN_TEST(test_id_conflict);
    RUN_TEST(test_assign_any_supplier);
    RUN_TEST(test_assign_zero_id);
    RUN_TEST(test_assign_message_not_found);
    return UNITY_END();
}
