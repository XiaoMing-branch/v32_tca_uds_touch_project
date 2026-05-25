#include "unity.h"
#include "fff.h"
#include "fff_diagnosticIII.h"
#include "fff_lin.h"
#include "fff_linlib.h"
#include "fff_store_manager.h"
#include "fff_pal_store.h"

extern void lin_diagservice_assign_nad(uint8_t NAD, uint8_t *ptr, uint16_t length);

extern l_u8 lin_configuration_RAM[];
extern lin_product_id product_id;
extern l_u8 lin_configured_NAD;
extern l_u8 lin_initial_NAD;
extern sys_cfg_t g_sys_cfgs;

DEFINE_FFF_GLOBALS;

static void pal_store_read_fill_ff_fake(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        value[i] = 0xFF;
    }
}

static uint8_t mock_flash_data[8];

static void pal_store_read_mock_fake(flash_type_e type, uint32_t addr, uint8_t *value, uint16_t length)
{
    (void)type;
    (void)addr;
    for (uint16_t i = 0; i < length && i < sizeof(mock_flash_data); i++)
    {
        value[i] = mock_flash_data[i];
    }
}

void setUp(void)
{
    FFF_RESET_HISTORY();
    RESET_FAKE(lin_diag_positive_notify);
    RESET_FAKE(lin_diag_negative_notify);
    RESET_FAKE(pal_store_read);
    RESET_FAKE(pal_store_write);

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    product_id.variant = 0x00;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;
    g_sys_cfgs.nad = 0x6F;

    pal_store_read_fake.custom_fake = pal_store_read_fill_ff_fake;
    pal_store_write_fake.return_val = true;
}

void tearDown(void)
{
}

void test_assign_nad_length_not_6(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x68};

    lin_diagservice_assign_nad(0x68, data, 5);

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_assign_nad_nad_not_match(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x68};

    lin_diagservice_assign_nad(0x70, data, 6);

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_assign_nad_initial_nad_match(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB0, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_assign_nad_broadcast_match(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x7F, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB0, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_assign_nad_configured_nad_match(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x70;

    lin_diagservice_assign_nad(0x70, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB0, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_assign_nad_supplier_not_match(void)
{
    uint8_t data[] = {0xB0, 0x01, 0x02, 0x00, 0x00, 0x60};

    product_id.supplier_id = 0x9999;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_assign_nad_function_not_match(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x01, 0x02, 0x60};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x9999;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_assign_nad_any_supplier_match(void)
{
    uint8_t data[] = {0xB0, 0xFF, 0x7F, 0x00, 0x00, 0x60};

    product_id.supplier_id = 0x1234;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB0, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_assign_nad_any_function_match(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0xFF, 0xFF, 0x60};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x1234;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB0, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_assign_nad_invalid_new_nad_low(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x5F};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB0, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x5F, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x5F, g_sys_cfgs.nad);
}

void test_assign_nad_invalid_new_nad_high(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x6E};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0xB0, lin_diag_positive_notify_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x6E, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x6E, g_sys_cfgs.nad);
}

void test_assign_nad_valid_nad_0x60(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_assign_nad_valid_nad_0x64(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x64};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x64, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x64, g_sys_cfgs.nad);
}

void test_assign_nad_valid_nad_0x67(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x67};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x67, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x67, g_sys_cfgs.nad);
}

void test_assign_nad_valid_nad_0x68(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x68};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x68, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x68, g_sys_cfgs.nad);
}

void test_assign_nad_valid_nad_0x6A(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x6A};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x6A, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x6A, g_sys_cfgs.nad);
}

void test_assign_nad_valid_nad_0x6C(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x6C};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x6C, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x6C, g_sys_cfgs.nad);
}

void test_assign_nad_valid_nad_0x6D(void)
{
    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x6D};

    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0x6D, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x6D, g_sys_cfgs.nad);
}

void test_fast_nad_write_nonzero_offset(void)
{
    pal_store_read_fake.custom_fake = pal_store_read_mock_fake;
    mock_flash_data[0] = 0x00;
    for (int i = 1; i < 8; i++) mock_flash_data[i] = 0xFF;

    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};
    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(1, pal_store_write_fake.call_count);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_fast_nad_write_duplicate(void)
{
    pal_store_read_fake.custom_fake = pal_store_read_mock_fake;
    mock_flash_data[0] = 0x60;
    for (int i = 1; i < 8; i++) mock_flash_data[i] = 0xFF;

    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};
    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, pal_store_write_fake.call_count);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_fast_nad_write_write_fail(void)
{
    pal_store_write_fake.return_val = false;

    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};
    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(1, pal_store_write_fake.call_count);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

void test_fast_nad_write_flash_full(void)
{
    pal_store_read_fake.custom_fake = pal_store_read_mock_fake;
    for (int i = 0; i < 8; i++) mock_flash_data[i] = 0x00;

    uint8_t data[] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x60};
    product_id.supplier_id = 0x0000;
    product_id.function_id = 0x0000;
    lin_initial_NAD = 0x68;
    lin_configured_NAD = 0x68;

    lin_diagservice_assign_nad(0x68, data, 6);

    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, pal_store_write_fake.call_count);
    TEST_ASSERT_EQUAL(0x60, lin_configured_NAD);
    TEST_ASSERT_EQUAL(0x60, g_sys_cfgs.nad);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_assign_nad_length_not_6);
    RUN_TEST(test_assign_nad_nad_not_match);
    RUN_TEST(test_assign_nad_initial_nad_match);
    RUN_TEST(test_assign_nad_broadcast_match);
    RUN_TEST(test_assign_nad_configured_nad_match);
    RUN_TEST(test_assign_nad_supplier_not_match);
    RUN_TEST(test_assign_nad_function_not_match);
    RUN_TEST(test_assign_nad_any_supplier_match);
    RUN_TEST(test_assign_nad_any_function_match);
    RUN_TEST(test_assign_nad_invalid_new_nad_low);
    RUN_TEST(test_assign_nad_invalid_new_nad_high);
    RUN_TEST(test_assign_nad_valid_nad_0x60);
    RUN_TEST(test_assign_nad_valid_nad_0x64);
    RUN_TEST(test_assign_nad_valid_nad_0x67);
    RUN_TEST(test_assign_nad_valid_nad_0x68);
    RUN_TEST(test_assign_nad_valid_nad_0x6A);
    RUN_TEST(test_assign_nad_valid_nad_0x6C);
    RUN_TEST(test_assign_nad_valid_nad_0x6D);
    RUN_TEST(test_fast_nad_write_nonzero_offset);
    RUN_TEST(test_fast_nad_write_duplicate);
    RUN_TEST(test_fast_nad_write_write_fail);
    RUN_TEST(test_fast_nad_write_flash_full);

    return UNITY_END();
}
