#include "unity.h"
#include "fff.h"
#include "custom_diagnosticIII.h"
#include "fff_lin_frame.h"
#include "fff_pal_store.h"
#include "fff_store_manager.h"
#include "fff_tcae10_ll_flash.h"
#include "fff_tcae10_ll_delay.h"
#include "fff_tcae10_ll_wdg.h"

DEFINE_FFF_GLOBALS;

/* ===== 从 custom_diagnosticIII.c 复制的宏 ===== */
#define CUS_UDS_RECEIVE_BUFFER_SIZE (20)
#define CUS_UDS_SEND_BUFFER_SIZE (66)
#define CONFIGURE_WORD_STATE_INIT 0
#define CONFIGURE_WORD_STATE_START 1
#define CONFIGURE_WORD_STATE_ASIGN 2
#define CONFIGURE_WORD_STATE_SAVE 3
#define CONFIGURE_WORD_STATE_END 4
#define CUS_UDS_PRODUCT_IDENT 0xF3u

/* ===== 来自 source 文件的全局变量 extern 声明 ===== */
extern uint8_t diagnosticRxBuffer[];
extern uint8_t diagnosticTxBuffer[];
extern uint8_t diagRxSize;
extern uint8_t negResponseCode;
extern uint8_t g_config_word_state;
extern uint16_t lock_failed_cnt;
extern uint8_t unlock_failed_store_flag;
extern uint32_t diagnostic_session_cnt;
extern lin_touch_data touch_data;

typedef struct
{
    uint8_t app_req_ext_program_flag;
    uint8_t app_need_res_flag;
    uint8_t lock_failed_index;
} ota_cfg_t_extern;

extern ota_cfg_t_extern g_ota_info;
extern user_cfg_t g_user_info;
extern void user_read_data_by_id(uint8_t mul_flag, uint8_t mul_len, uint16_t did, uint16_t *len);

static void reset_globals(void)
{
    unsigned int i;
    uint8_t *rx = diagnosticRxBuffer;
    uint8_t *tx = diagnosticTxBuffer;
    for (i = 0; i < CUS_UDS_RECEIVE_BUFFER_SIZE; i++)
        rx[i] = 0;
    for (i = 0; i < CUS_UDS_SEND_BUFFER_SIZE; i++)
        tx[i] = 0;
    diagRxSize = 0;
    negResponseCode = 0;
    session_mode = SESSION_MODE_DEFAULT;
    program_condition_check = 0;
    g_config_word_state = 0;
    {
        uint8_t *p = (uint8_t *)&g_user_info;
        for (i = 0; i < sizeof(user_cfg_t); i++)
            p[i] = 0;
    }
    {
        uint8_t *p = (uint8_t *)&g_ota_info;
        for (i = 0; i < sizeof(ota_cfg_t_extern); i++)
            p[i] = 0;
    }
    {
        uint8_t *p = (uint8_t *)&door_cmd;
        for (i = 0; i < sizeof(DoorCmd_T); i++)
            p[i] = 0;
    }
    {
        uint8_t *p = (uint8_t *)&touch_data;
        for (i = 0; i < sizeof(lin_touch_data); i++)
            p[i] = 0;
    }
    lock_failed_cnt = 0;
    unlock_failed_store_flag = 0;
    diagnostic_session_cnt = 0;
    {
        extern l_u8 lin_configuration_RAM[];
        extern l_u16 lin_configuration_ROM[];
        for (i = 0; i < 10; i++)
            lin_configuration_RAM[i] = 0;
        for (i = 0; i < 10; i++)
            lin_configuration_ROM[i] = 0;
    }
}

void setUp(void)
{
    FFF_RESET_HISTORY();
    reset_globals();

    ld_send_message_fake.call_count = 0;
    ld_send_message_fake.arg_history_len = 0;
    ld_tx_status_fake.call_count = 0;
    ld_tx_status_fake.return_val = LD_COMPLETED;
    lin_current_rcvd_nad_fake.call_count = 0;
    lin_current_rcvd_nad_fake.return_val = 0;
    pal_store_data_get_fake.call_count = 0;
    pal_store_data_get_fake.return_val = false;
    pal_store_data_set_fake.call_count = 0;
    store_slow_smart_read_fake.call_count = 0;
    store_slow_smart_read_fake.return_val = false;
    ld_read_by_id_callout_fake.call_count = 0;
    ld_read_by_id_callout_fake.return_val = 0;
    ld_read_by_id_callout_fake.custom_fake = NULL;
    lin_diag_positive_notify_fake.call_count = 0;
    lin_diag_negative_notify_fake.call_count = 0;
    delay1ms_fake.call_count = 0;
    ll_wdg_enable_fake.call_count = 0;
    ll_flash_read_fake.call_count = 0;
    pal_store_erase_fake.call_count = 0;
    pal_store_write_fake.call_count = 0;
}

void tearDown(void)
{
}

/* ========== send_negative_response_message ========== */
void test_send_negative_response_tx_completed(void)
{
    diagnosticRxBuffer[0] = 0x10;
    ld_tx_status_fake.return_val = LD_COMPLETED;
    send_negative_response_message(0x22);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0x10, diagnosticTxBuffer[1]);
    TEST_ASSERT_EQUAL_UINT8(0x22, diagnosticTxBuffer[2]);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_send_negative_response_tx_timeout(void)
{
    ld_tx_status_fake.return_val = LD_N_AS_TIMEOUT;
    send_negative_response_message(0x31);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0x31, diagnosticTxBuffer[2]);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_send_negative_response_tx_other(void)
{
    ld_tx_status_fake.return_val = 0xFF;
    send_negative_response_message(0x12);
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

/* ========== send_positive_response_message ========== */
void test_send_positive_response_tx_completed(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticTxBuffer[1] = 0x01;
    ld_tx_status_fake.return_val = LD_COMPLETED;
    send_positive_response_message(2);
    TEST_ASSERT_EQUAL_UINT8(0x50, diagnosticTxBuffer[0]);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_send_positive_response_tx_timeout(void)
{
    diagnosticRxBuffer[0] = 0x22;
    ld_tx_status_fake.return_val = LD_N_AS_TIMEOUT;
    send_positive_response_message(3);
    TEST_ASSERT_EQUAL_UINT8(0x62, diagnosticTxBuffer[0]);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_send_positive_response_tx_other(void)
{
    ld_tx_status_fake.return_val = 0xFF;
    send_positive_response_message(2);
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

/* ========== uds_diagnostic_session_control ========== */
void test_session_control_default_session(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL_UINT8(SESSION_MODE_DEFAULT, session_mode);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_session_control_extend_session(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x03;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL_UINT8(SESSION_MODE_EXTEND, session_mode);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_session_control_program_session_no_condition(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    program_condition_check = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_session_control_program_session_with_condition(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    program_condition_check = 1;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL_UINT8(0, program_condition_check);
    TEST_ASSERT_EQUAL(1, ll_wdg_enable_fake.call_count);
}

void test_session_control_wrong_length(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_session_control_nad_7e_no_response(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_session_control_nad_7f_no_response(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_session_control_program_session_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    program_condition_check = 1;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL_UINT8(3, g_ota_info.app_req_ext_program_flag);
    TEST_ASSERT_EQUAL(1, ll_wdg_enable_fake.call_count);
}

void test_session_control_unknown_subfunc_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x04;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

/* ========== uds_diagnostic_route_control ========== */
void test_route_control_nad_7e(void)
{
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_route_control_nad_7f(void)
{
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_route_control_wrong_subfunc_not_01_81(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x00;
    diagnosticRxBuffer[3] = 0x00;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_route_control_valid_route_not_extend(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x03;
    diagRxSize = 4;
    session_mode = SESSION_MODE_DEFAULT;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_route_control_wrong_length(void)
{
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_route_control_wrong_subfunc(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x00;
    diagnosticRxBuffer[3] = 0x00;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_route_control_valid_route_extend_good_speed(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x03;
    diagRxSize = 4;
    session_mode = SESSION_MODE_EXTEND;
    door_cmd.VehicleSpeedValid = 1;
    door_cmd.VehicleSpeed = 0x20;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL_UINT8(1, program_condition_check);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_route_control_valid_route_extend_bad_speed(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x03;
    diagRxSize = 4;
    session_mode = SESSION_MODE_EXTEND;
    door_cmd.VehicleSpeedValid = 1;
    door_cmd.VehicleSpeed = 0x40;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(2, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x71, diagnosticTxBuffer[0]);
}

void test_route_control_routine_ff01(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0xFF;
    diagnosticRxBuffer[3] = 0x01;
    diagRxSize = 4;
    session_mode = SESSION_MODE_EXTEND;
    door_cmd.VehicleSpeedValid = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_route_control_subfunc_81(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x81;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x03;
    diagRxSize = 4;
    session_mode = SESSION_MODE_EXTEND;
    door_cmd.VehicleSpeedValid = 1;
    door_cmd.VehicleSpeed = 0x20;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL_UINT8(1, program_condition_check);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

/* ========== uds_diagnostic_dtc_control ========== */
void test_dtc_control_not_extend(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    session_mode = SESSION_MODE_DEFAULT;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_dtc_control_extend_subfunc_01(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x01, diagnosticTxBuffer[1]);
}

void test_dtc_control_extend_subfunc_02(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x02, diagnosticTxBuffer[1]);
}

void test_dtc_control_extend_unknown_subfunc(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x05;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_dtc_control_wrong_length(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_dtc_control_extend_subfunc_01_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_dtc_control_extend_subfunc_01_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_dtc_control_not_extend_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    session_mode = SESSION_MODE_DEFAULT;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

/* ========== uds_diagnostic_clear_dtc_info ========== */
void test_clear_dtc_valid(void)
{
    diagnosticRxBuffer[0] = 0x14;
    diagnosticRxBuffer[1] = 0xFF;
    diagnosticRxBuffer[2] = 0xFF;
    diagnosticRxBuffer[3] = 0xFF;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_clear_dtc_info();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_clear_dtc_invalid_group(void)
{
    diagnosticRxBuffer[0] = 0x14;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x00;
    diagnosticRxBuffer[3] = 0x00;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_clear_dtc_info();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_clear_dtc_wrong_length(void)
{
    diagnosticRxBuffer[0] = 0x14;
    diagRxSize = 1;
    uds_diagnostic_clear_dtc_info();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_clear_dtc_valid_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x14;
    diagnosticRxBuffer[1] = 0xFF;
    diagnosticRxBuffer[2] = 0xFF;
    diagnosticRxBuffer[3] = 0xFF;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_clear_dtc_info();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_clear_dtc_invalid_group_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x14;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x00;
    diagnosticRxBuffer[3] = 0x00;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_clear_dtc_info();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

/* ========== uds_tester_present_control ========== */
void test_tester_present_subfunc_80(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x80;
    diagRxSize = 2;
    uds_tester_present_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_tester_present_subfunc_00(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_tester_present_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_tester_present_wrong_length(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 3;
    uds_tester_present_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_tester_present_unknown_subfunc(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_tester_present_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_tester_present_subfunc_00_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_tester_present_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_tester_present_subfunc_00_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_tester_present_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

/* ========== uds_diagnostic_rest ========== */
void test_ecu_reset_subfunc_01_speed_valid(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_rest();
    // TEST_ASSERT_EQUAL(2, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL(1, ll_wdg_enable_fake.call_count);
}

void test_ecu_reset_subfunc_01_speed_invalid(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 1;
    door_cmd.VehicleSpeed = 0x40;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_ecu_reset_subfunc_02(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_ecu_reset_wrong_length(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x00;
    diagRxSize = 3;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_ecu_reset_subfunc_81_speed_valid(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x81;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 0;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ll_wdg_enable_fake.call_count);
}

void test_ecu_reset_subfunc_81_speed_invalid(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x81;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 1;
    door_cmd.VehicleSpeed = 0x40;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_ecu_reset_subfunc_03(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x03;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_ecu_reset_subfunc_82_83(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x82;
    diagnosticRxBuffer[2] = 0x00;
    diagRxSize = 2;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_ecu_reset_unknown_subfunc(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x05;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

/* ========== uds_diagnostic_readdata_by_id ========== */
void test_readdata_by_id_nad_7e(void)
{
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_readdata_by_id_single_did_valid(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0xF1, diagnosticTxBuffer[1]);
    TEST_ASSERT_EQUAL_UINT8(0x87, diagnosticTxBuffer[2]);
}

void test_readdata_by_id_single_did_invalid(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x02;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_readdata_by_id_wrong_length(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagnosticRxBuffer[3] = 0x00;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_readdata_by_id_did_F18A(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x8A;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8('3', diagnosticTxBuffer[3]);
}

void test_readdata_by_id_did_F189(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x89;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8('S', diagnosticTxBuffer[3]);
}

void test_readdata_by_id_did_0216(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x16;
    diagRxSize = 3;
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT16(24, ld_send_message_fake.arg0_val);
}

void test_readdata_by_id_did_F190(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x90;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_did_F0FA(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF0;
    diagnosticRxBuffer[2] = 0xFA;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_did_0001(void)
{
    touch_data.key1_raw[0] = 100;
    touch_data.key1_base[0] = 200;
    touch_data.key1_diff[0] = 100;
    touch_data.key_val = 1;
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_did(void)
{
    uint8_t i;
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagRxSize = 5;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_F197(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x97;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagRxSize = 5;
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_F189(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x89;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagRxSize = 5;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_0216(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x16;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagRxSize = 5;
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_F184(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x84;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagRxSize = 5;
    store_slow_smart_read_fake.return_val = true;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_F089(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF0;
    diagnosticRxBuffer[2] = 0x89;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagRxSize = 5;
    ll_flash_read_fake.return_val = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_F180(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x80;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagRxSize = 5;
    ll_flash_read_fake.return_val = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_F197_RL(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x97;
    diagRxSize = 3;
    g_user_info.config_word = LEFT_REAR_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[8]);
    TEST_ASSERT_EQUAL_UINT8('L', diagnosticTxBuffer[9]);
}

void test_readdata_by_id_F197_FR(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x97;
    diagRxSize = 3;
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8('F', diagnosticTxBuffer[8]);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[9]);
}

void test_readdata_by_id_F197_RR(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x97;
    diagRxSize = 3;
    g_user_info.config_word = RIGHT_REAR_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[8]);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[9]);
}

void test_readdata_by_id_0216_RL(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x16;
    diagRxSize = 3;
    g_user_info.config_word = LEFT_REAR_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_0216_FR(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x16;
    diagRxSize = 3;
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_0216_RR(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x16;
    diagRxSize = 3;
    g_user_info.config_word = RIGHT_REAR_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

/* ========== uds_pal_store_data_set ========== */
void test_pal_store_data_set(void)
{
    uint8_t test_data[] = {0xAA, 0xBB};
    uds_pal_store_data_set(0x1000, test_data, 2);
    TEST_ASSERT_EQUAL(1, pal_store_data_set_fake.call_count);
}

/* ========== lin_custom_diag_service_handle ========== */
void test_lin_custom_diag_service_handle(void)
{
    uint8_t data[] = {0x10, 0x01};
    lin_custom_diag_service_handle(0x10, data, 2);
    TEST_ASSERT_EQUAL_UINT8(2, diagRxSize);
    TEST_ASSERT_EQUAL_UINT8(0x10, diagnosticRxBuffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0x01, diagnosticRxBuffer[1]);
    TEST_ASSERT_EQUAL_UINT32(0, diagnostic_session_cnt);
}

/* ========== lin_diag_service_hook ========== */
void test_lin_diag_service_hook(void)
{
    ld_tx_status_fake.return_val = LD_COMPLETED;
    lin_diag_service_hook();
    TEST_ASSERT_EQUAL(1, ld_tx_status_fake.call_count);
}

/* ========== store_system_data_init ========== */
void test_store_system_data_init(void)
{
    store_slow_smart_read_fake.return_val = true;
    store_system_data_init();
    TEST_ASSERT_EQUAL(2, store_slow_smart_read_fake.call_count);
}

/* ========== LinDiagnosticSessionCheck ========== */
void test_session_check_default(void)
{
    session_mode = SESSION_MODE_DEFAULT;
    LinDiagnosticSessionCheck();
    TEST_ASSERT_EQUAL_UINT32(0, diagnostic_session_cnt);
}

void test_session_check_not_expired(void)
{
    session_mode = SESSION_MODE_EXTEND;
    diagnostic_session_cnt = 100;
    LinDiagnosticSessionCheck();
    TEST_ASSERT_EQUAL_UINT32(101, diagnostic_session_cnt);
}

void test_session_check_expired(void)
{
    session_mode = SESSION_MODE_EXTEND;
    diagnostic_session_cnt = 2501;
    LinDiagnosticSessionCheck();
    TEST_ASSERT_EQUAL_UINT32(0, diagnostic_session_cnt);
    TEST_ASSERT_EQUAL_UINT8(SESSION_MODE_DEFAULT, session_mode);
    TEST_ASSERT_EQUAL_UINT8(0, program_condition_check);
}

void test_session_check_lock_failed_decrement(void)
{
    g_ota_info.lock_failed_index = 3;
    lock_failed_cnt = 5001;
    LinDiagnosticSessionCheck();
    TEST_ASSERT_EQUAL_UINT16(0, lock_failed_cnt);
    TEST_ASSERT_EQUAL_UINT8(2, g_ota_info.lock_failed_index);
    TEST_ASSERT_EQUAL_UINT8(1, unlock_failed_store_flag);
}

/* ========== SysDoFlashRoutine27Service ========== */
void test_sys_do_flash_routine_flag_set(void)
{
    unlock_failed_store_flag = 1;
    SysDoFlashRoutine27Service();
    TEST_ASSERT_EQUAL_UINT8(0, unlock_failed_store_flag);
}

void test_sys_do_flash_routine_flag_clear(void)
{
    unlock_failed_store_flag = 0;
    SysDoFlashRoutine27Service();
    TEST_ASSERT_EQUAL_UINT8(0, unlock_failed_store_flag);
}

/* ========== lin_customized_operation ========== */
void test_lin_customized_operation_flag_set(void)
{
    g_ota_info.app_need_res_flag = 1;
    ld_tx_status_fake.return_val = LD_COMPLETED;
    lin_customized_operation();
    TEST_ASSERT_EQUAL_UINT8(0, g_ota_info.app_need_res_flag);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_lin_customized_operation_flag_clear(void)
{
    g_ota_info.app_need_res_flag = 0;
    lin_customized_operation();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

/* ========== uds_diagnostic_configword_remap_nad ========== */
static void check_configword_remap_common(void)
{
    extern l_u8 lin_configured_NAD;
    extern l_u8 lin_initial_NAD;
    extern l_signal_handle LI0_response_error_signal;
    extern l_signal_handle response_error;
    extern l_u16 lin_response_error_byte_offset[];
    extern l_u8 lin_response_error_bit_offset[];

    TEST_ASSERT_EQUAL_UINT8(g_user_info.nad_info, lin_configured_NAD);
    TEST_ASSERT_EQUAL_UINT8(g_user_info.nad_info, lin_initial_NAD);

    if (g_user_info.config_word == LEFT_FRONT_DOOR)
    {
        TEST_ASSERT_EQUAL_UINT8(1, lin_configuration_RAM[5]);
        TEST_ASSERT_EQUAL_UINT8(LI0_EHIS_FL_ResponseError, LI0_response_error_signal);
    }
    else if (g_user_info.config_word == LEFT_REAR_DOOR)
    {
        TEST_ASSERT_EQUAL_UINT8(5, lin_configuration_RAM[5]);
        TEST_ASSERT_EQUAL_UINT8(LI0_EHIS_RL_ResponseError, LI0_response_error_signal);
    }
    else if (g_user_info.config_word == RIGHT_FRONT_DOOR)
    {
        TEST_ASSERT_EQUAL_UINT8(5, lin_configuration_RAM[5]);
        TEST_ASSERT_EQUAL_UINT8(LI0_EHIS_FR_ResponseError, LI0_response_error_signal);
    }
    else if (g_user_info.config_word == RIGHT_REAR_DOOR)
    {
        TEST_ASSERT_EQUAL_UINT8(2, lin_configuration_RAM[5]);
        TEST_ASSERT_EQUAL_UINT8(LI0_EHIS_RR_ResponseError, LI0_response_error_signal);
    }
}

void test_configword_remap_left_front(void)
{
    g_user_info.config_word = LEFT_FRONT_DOOR;
    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x68, g_user_info.nad_info);
    check_configword_remap_common();
}

void test_configword_remap_left_rear(void)
{
    g_user_info.config_word = LEFT_REAR_DOOR;
    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x6A, g_user_info.nad_info);
    check_configword_remap_common();
}

void test_configword_remap_right_front(void)
{
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x69, g_user_info.nad_info);
    check_configword_remap_common();
}

void test_configword_remap_right_rear(void)
{
    g_user_info.config_word = RIGHT_REAR_DOOR;
    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x6B, g_user_info.nad_info);
    check_configword_remap_common();
}

void test_uds_diagnostic_configword_remap_nad0(void)
{
    g_user_info.config_word = 4;
    uds_diagnostic_configword_remap_nad();
}

/* ========== uds_diagnostic_assign_NAD ========== */
void test_assign_nad_nad_7e(void)
{
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_INIT, g_config_word_state);
}

void test_assign_nad_fuc_id_01(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x01;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_START, g_config_word_state);
}

void test_assign_nad_fuc_id_02_valid_door(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x02;
    diagnosticRxBuffer[5] = LEFT_FRONT_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_ASIGN, g_config_word_state);
}

void test_assign_nad_fuc_id_03(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x03;
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_SAVE, g_config_word_state);
}

void test_assign_nad_fuc_id_04(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x04;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_END, g_config_word_state);
}

/* ========== uds_communction_control ========== */
void test_comm_control_not_extend(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_DEFAULT;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_comm_control_wrong_length(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_comm_control_enable_rx(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x11, lin_configuration_RAM[1]);
    TEST_ASSERT_EQUAL_UINT8(0x13, lin_configuration_RAM[2]);
    TEST_ASSERT_EQUAL_UINT8(0x14, lin_configuration_RAM[3]);
    TEST_ASSERT_EQUAL_UINT8(0x12, lin_configuration_RAM[4]);
}

void test_comm_control_disable_rx(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x03;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0xFF, lin_configuration_RAM[1]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, lin_configuration_RAM[2]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, lin_configuration_RAM[3]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, lin_configuration_RAM[4]);
}

void test_comm_control_invalid_control(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_comm_control_unknown_subfunc(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x10;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

/* ========== lin_handle_uds ========== */
void test_lin_handle_uds_service_14(void)
{
    diagnosticRxBuffer[0] = SERVICE_CLEAR_DTC_INFO;
    lin_handle_uds();
    TEST_ASSERT_EQUAL_UINT8(0, negResponseCode);
}

void test_lin_handle_uds_service_10(void)
{
    diagnosticRxBuffer[0] = SERVICE_SESSION_CONTROL;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
}

void test_lin_handle_uds_service_2e(void)
{
    diagnosticRxBuffer[0] = SERVICE_WRITE_DATA_BY_IDENTIFY;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_lin_handle_uds_service_27(void)
{
    diagnosticRxBuffer[0] = SERVICE_SECURITY_ACCESS;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_lin_handle_uds_default(void)
{
    diagnosticRxBuffer[0] = 0xFF;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_lin_handle_uds_service_22(void)
{
    diagnosticRxBuffer[0] = SERVICE_READ_DATA_BY_IDENTIFY;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagRxSize = 3;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_lin_handle_uds_service_85(void)
{
    diagnosticRxBuffer[0] = SERVICE_DTC_CONTROL;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_lin_handle_uds_service_28(void)
{
    diagnosticRxBuffer[0] = SERVICE_COMMUNICATION_CONTROL;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
}

void test_lin_handle_uds_service_3e(void)
{
    diagnosticRxBuffer[0] = SERVICE_TESTER_PRESENT;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
}

void test_lin_handle_uds_service_11(void)
{
    diagnosticRxBuffer[0] = SERVICE_ECU_RESET;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
}

void test_lin_handle_uds_service_b5(void)
{
    diagnosticRxBuffer[0] = SERVICE_ASSIGN_NAD_VIA_SNPD;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x01;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_START, g_config_word_state);
}

void test_lin_handle_uds_program_condition_active(void)
{
    program_condition_check = 1;
    lin_configured_NAD = 0x68;
    diagnosticRxBuffer[0] = SERVICE_TESTER_PRESENT;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(1, program_condition_check);
}

void test_lin_handle_uds_program_condition_cleared(void)
{
    program_condition_check = 1;
    lin_configured_NAD = 0x68;
    diagnosticRxBuffer[0] = 0xFF;
    diagRxSize = 1;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(0, program_condition_check);
}

void test_store_system_data_init_lock_index_clamped(void)
{
    store_slow_smart_read_fake.return_val = true;
    g_ota_info.lock_failed_index = 5;
    store_system_data_init();
    TEST_ASSERT_EQUAL_UINT8(3, g_ota_info.lock_failed_index);
}

void test_session_check_lock_failed_not_exceeded(void)
{
    g_ota_info.lock_failed_index = 3;
    lock_failed_cnt = 100;
    LinDiagnosticSessionCheck();
    TEST_ASSERT_EQUAL_UINT16(101, lock_failed_cnt);
    TEST_ASSERT_EQUAL_UINT8(0, unlock_failed_store_flag);
}

/* custom fake for ld_read_by_id_callout: writes to data buffer */
static l_u8 ld_read_by_id_callout_custom_fake(l_u8 id, l_u8 *data)
{
    data[0] = 0x01;

    return LD_POSITIVE_RESPONSE;
}

/* ========== lin_diagservice_read_by_identifier ========== */
void test_read_by_id_supplier_not_match(void)
{
    uint8_t data[8] = {0x12, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    uint16_t len = 8;
    lin_diagservice_read_by_identifier(data, len);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_read_by_id_id_in_range_callout_positive(void)
{
    uint8_t data[8] = {0x12, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ld_read_by_id_callout_fake.custom_fake = ld_read_by_id_callout_custom_fake;
    lin_diagservice_read_by_identifier(data, 8);
    // TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
}

void test_read_by_id_id_in_range_callout_negative(void)
{
    uint8_t data[8] = {0x12, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ld_read_by_id_callout_fake.return_val = LD_NEGATIVE_RESPONSE;
    lin_diagservice_read_by_identifier(data, 8);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
}

void test_read_by_id_id_out_of_range(void)
{
    uint8_t data[8] = {0x12, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    lin_diagservice_read_by_identifier(data, 8);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
}

void test_read_by_id_product_ident_format_mismatch(void)
{
    uint8_t data[6] = {0x12, 0xF3, 0x00, 0x00, 0x01, 0x00};
    lin_current_rcvd_nad_fake.return_val = 0x12;
    pal_store_data_get_fake.return_val = true;
    lin_diagservice_read_by_identifier(data, 6);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
}

/* ========== 补充的覆盖率测试 ========== */

void test_dtc_control_extend_subfunc_02_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_dtc_control_extend_unknown_subfunc_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x05;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_clear_dtc_invalid_bytes(void)
{
    diagnosticRxBuffer[0] = 0x14;
    diagnosticRxBuffer[1] = 0xFF;
    diagnosticRxBuffer[2] = 0x00;
    diagnosticRxBuffer[3] = 0xFF;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_clear_dtc_info();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_tester_present_unknown_subfunc_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_tester_present_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_uds_tester_present_control0(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_tester_present_control();
}

void test_ecu_reset_subfunc_01_speed_high(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 1;
    door_cmd.VehicleSpeed = 0x40;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_ecu_reset_subfunc_02_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_readdata_by_id_nad_7f(void)
{
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_did_invalid(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x00;
    diagnosticRxBuffer[4] = 0x03;
    diagRxSize = 5;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_readdata_by_id_did_F197(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x97;
    diagRxSize = 3;
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8('E', diagnosticTxBuffer[3]);
}

void test_readdata_by_id_did_F184(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x84;
    diagRxSize = 3;
    store_slow_smart_read_fake.return_val = true;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_did_F089(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF0;
    diagnosticRxBuffer[2] = 0x89;
    diagRxSize = 3;
    ll_flash_read_fake.return_val = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_did_F180(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x80;
    diagRxSize = 3;
    ll_flash_read_fake.return_val = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_route_control_extend_speed_high(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x03;
    diagRxSize = 4;
    session_mode = SESSION_MODE_EXTEND;
    door_cmd.VehicleSpeedValid = 1;
    door_cmd.VehicleSpeed = 0x40;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL(2, ld_send_message_fake.call_count);
}

void test_route_control_extend_speed_invalid(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x03;
    diagRxSize = 4;
    session_mode = SESSION_MODE_EXTEND;
    door_cmd.VehicleSpeedValid = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL_UINT8(1, program_condition_check);
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_comm_control_enable_rx_type_03(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x03;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_comm_control_subfunc_82(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x82;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_comm_control_subfunc_02(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x02;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_comm_control_unknown_subfunc_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x10;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_assign_nad_wrong_supplier(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x00;
    diagnosticRxBuffer[3] = 0x01;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_INIT, g_config_word_state);
}

void test_assign_nad_door_right_front(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x02;
    diagnosticRxBuffer[5] = RIGHT_FRONT_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_ASIGN, g_config_word_state);
}

void test_assign_nad_fuc_id_default(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x05;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_INIT, g_config_word_state);
}

void test_lin_handle_uds_service_2e_nad_7f(void)
{
    diagnosticRxBuffer[0] = SERVICE_WRITE_DATA_BY_IDENTIFY;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_lin_handle_uds_service_27_nad_7f(void)
{
    diagnosticRxBuffer[0] = SERVICE_SECURITY_ACCESS;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_lin_handle_uds_default_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0xFF;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

static l_u8 ld_read_by_id_callout_blank_fake(l_u8 id, l_u8 *data)
{
    return LD_POSITIVE_RESPONSE;
}

void test_read_by_id_supplier_match(void)
{
    uint8_t data[8] = {0x12, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ld_read_by_id_callout_fake.custom_fake = ld_read_by_id_callout_custom_fake;
    lin_diagservice_read_by_identifier(data, 8);
    // TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
}

void test_read_by_id_callout_all_ff(void)
{
    uint8_t data[8] = {0x12, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ld_read_by_id_callout_fake.custom_fake = ld_read_by_id_callout_blank_fake;
    lin_diagservice_read_by_identifier(data, 8);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
}

void test_session_control_program_session_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    program_condition_check = 1;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL_UINT8(3, g_ota_info.app_req_ext_program_flag);
}

void test_session_control_default_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x04;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_session_control_default_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x04;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_route_control_routine_ff00(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0xFF;
    diagnosticRxBuffer[3] = 0x00;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_route_control_routine_dd02(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0xDD;
    diagnosticRxBuffer[3] = 0x02;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_dtc_control_extend_subfunc_02_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_dtc_control_not_extend_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    session_mode = SESSION_MODE_DEFAULT;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_dtc_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_uds_diagnostic_dtc_control0(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x81;
    diagRxSize = 2;
    session_mode = 0x03;
    uds_diagnostic_dtc_control();
}

void test_uds_diagnostic_dtc_control1(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x82;
    diagRxSize = 2;
    session_mode = 0x03;
    uds_diagnostic_dtc_control();
}

void test_uds_diagnostic_dtc_control2(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    session_mode = 0x03;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_dtc_control();
}

void test_comm_control_enable_rx_subfunc_00(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_comm_control_enable_rx_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x80;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_comm_control_disable_rx_type_03(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x03;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0xFF, lin_configuration_RAM[1]);
}

void test_comm_control_subfunc_82_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x82;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_comm_control_unknown_subfunc_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x10;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_ecu_reset_subfunc_03_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x03;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_ecu_reset_subfunc_83(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x00;
    diagRxSize = 2;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_ecu_reset_subfunc_82_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x82;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_did_partial_valid(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagnosticRxBuffer[3] = 0x00;
    diagnosticRxBuffer[4] = 0x02;
    diagRxSize = 5;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_assign_nad_fuc_id_02_wrong_state(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x02;
    diagnosticRxBuffer[5] = LEFT_FRONT_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_INIT;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_INIT, g_config_word_state);
}

void test_assign_nad_fuc_id_03_wrong_state_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x03;
    g_config_word_state = CONFIGURE_WORD_STATE_INIT;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_INIT, g_config_word_state);
}

void test_lin_handle_uds_program_condition_service_10_82(void)
{
    program_condition_check = 1;
    lin_configured_NAD = 0x68;
    diagnosticRxBuffer[0] = SERVICE_SESSION_CONTROL;
    diagnosticRxBuffer[1] = 0x82;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
    TEST_ASSERT_EQUAL_UINT8(1, g_ota_info.app_req_ext_program_flag);
}

void test_lin_handle_uds_default_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0xFF;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_did_long(void)
{
    unsigned int i;
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagnosticRxBuffer[3] = 0xF1;
    diagnosticRxBuffer[4] = 0x8A;
    diagnosticRxBuffer[5] = 0xF1;
    diagnosticRxBuffer[6] = 0x97;
    diagnosticRxBuffer[7] = 0xF1;
    diagnosticRxBuffer[8] = 0x89;
    diagnosticRxBuffer[9] = 0x02;
    diagnosticRxBuffer[10] = 0x16;
    diagnosticRxBuffer[11] = 0xF1;
    diagnosticRxBuffer[12] = 0x84;
    diagnosticRxBuffer[13] = 0xF0;
    diagnosticRxBuffer[14] = 0x89;
    diagRxSize = 15;
    g_user_info.config_word = LEFT_FRONT_DOOR;
    store_slow_smart_read_fake.return_val = true;
    ll_flash_read_fake.return_val = 0;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_readdata_by_id_multi_include_0001(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagnosticRxBuffer[3] = 0x00;
    diagnosticRxBuffer[4] = 0x01;
    diagRxSize = 5;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
}

void test_uds_diagnostic_readdata_by_id0(void)
{
    diagnosticRxBuffer[0] = 0x22;
    diagnosticRxBuffer[1] = 0xF1;
    diagnosticRxBuffer[2] = 0x87;
    diagnosticRxBuffer[3] = 0x00;
    diagnosticRxBuffer[4] = 0x01;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_readdata_by_id();
}

void test_lin_handle_uds_service_31(void)
{
    diagnosticRxBuffer[0] = SERVICE_ROUTINE_CONTROL;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x02;
    diagnosticRxBuffer[3] = 0x03;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
}

void test_lin_handle_uds_program_condition_service_28(void)
{
    program_condition_check = 1;
    lin_configured_NAD = 0x68;
    diagnosticRxBuffer[0] = SERVICE_COMMUNICATION_CONTROL;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x03;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    lin_handle_uds();
}

void test_read_by_id_supplier_any(void)
{
    uint8_t data[8] = {0x12, 0x30, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00};
    ld_read_by_id_callout_fake.custom_fake = ld_read_by_id_callout_custom_fake;
    lin_diagservice_read_by_identifier(data, 8);
    // TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
}

void test_read_by_id_id_between_ranges(void)
{
    uint8_t data[8] = {0x12, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    lin_diagservice_read_by_identifier(data, 8);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
}

void test_read_by_id_callout_no_response(void)
{
    uint8_t data[8] = {0x12, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ld_read_by_id_callout_fake.return_val = LD_ID_NO_RESPONSE;
    lin_diagservice_read_by_identifier(data, 8);
    TEST_ASSERT_EQUAL(0, lin_diag_positive_notify_fake.call_count);
    TEST_ASSERT_EQUAL(0, lin_diag_negative_notify_fake.call_count);
}

void test_route_control_other_routine(void)
{
    diagnosticRxBuffer[0] = 0x31;
    diagnosticRxBuffer[1] = 0x01;
    diagnosticRxBuffer[2] = 0x12;
    diagnosticRxBuffer[3] = 0x34;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_route_control();
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

/* ========== 直接测试 user_read_data_by_id ========== */
static void helper_user_read_multi(uint8_t mul_flag, uint8_t mul_len, uint16_t did, uint8_t config_word)
{
    uint16_t len;
    g_user_info.config_word = config_word;
    user_read_data_by_id(mul_flag, mul_len, did, &len);
}

void test_user_read_default(void)
{
    uint16_t len;
    user_read_data_by_id(0, 0, 0x9999, &len);
    TEST_ASSERT_EQUAL_UINT16(0, len);
}

void test_user_read_F197_RL_mul(void)
{
    uint16_t len;
    g_user_info.config_word = LEFT_REAR_DOOR;
    user_read_data_by_id(1, 0, 0xF197, &len);
    TEST_ASSERT_EQUAL_UINT16(10, len);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[5]);
    TEST_ASSERT_EQUAL_UINT8('L', diagnosticTxBuffer[6]);
}

void test_user_read_F197_FR_mul(void)
{
    uint16_t len;
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    user_read_data_by_id(1, 0, 0xF197, &len);
    TEST_ASSERT_EQUAL_UINT8('F', diagnosticTxBuffer[5]);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[6]);
}

void test_user_read_F197_RR_mul(void)
{
    uint16_t len;
    g_user_info.config_word = RIGHT_REAR_DOOR;
    user_read_data_by_id(1, 0, 0xF197, &len);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[5]);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[6]);
}

void test_user_read_F197_NN_mul(void)
{
    uint16_t len;
    g_user_info.config_word = 4;
    user_read_data_by_id(1, 0, 0xF197, &len);
}

void test_user_read_0216_RL_mul(void)
{
    uint16_t len;
    g_user_info.config_word = LEFT_REAR_DOOR;
    user_read_data_by_id(1, 0, 0x0216, &len);
    TEST_ASSERT_EQUAL_UINT16(21, len);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[7]);
    TEST_ASSERT_EQUAL_UINT8('L', diagnosticTxBuffer[8]);
}

void test_user_read_0216_FR_mul(void)
{
    uint16_t len;
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    user_read_data_by_id(1, 0, 0x0216, &len);
    TEST_ASSERT_EQUAL_UINT8('F', diagnosticTxBuffer[7]);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[8]);
}

void test_user_read_0216_RR_mul(void)
{
    uint16_t len;
    g_user_info.config_word = RIGHT_REAR_DOOR;
    user_read_data_by_id(1, 0, 0x0216, &len);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[7]);
    TEST_ASSERT_EQUAL_UINT8('R', diagnosticTxBuffer[8]);
}

void test_user_read_0216_NN_mul(void)
{
    uint16_t len;
    g_user_info.config_word = 4;
    user_read_data_by_id(1, 0, 0x0216, &len);
}

void test_user_read_0216_NN_mul0(void)
{
    uint16_t len;
    g_user_info.config_word = 4;
    user_read_data_by_id(0, 0, 0x0216, &len);
}

void test_user_read_0001_mul(void)
{
    uint16_t len;
    touch_data.key1_raw[0] = 50;
    touch_data.key_val = 2;
    user_read_data_by_id(1, 5, 0x0001, &len);
    TEST_ASSERT_EQUAL_UINT16(27, len);
    TEST_ASSERT_EQUAL_UINT8(0, diagnosticTxBuffer[6]);
}

void test_user_read_F184_mul(void)
{
    uint16_t len;
    store_slow_smart_read_fake.return_val = true;
    user_read_data_by_id(1, 0, 0xF184, &len);
    TEST_ASSERT_EQUAL_UINT16(10, len);
}

void test_user_read_F089_mul(void)
{
    uint16_t len;
    ll_flash_read_fake.return_val = 0;
    user_read_data_by_id(1, 0, 0xF089, &len);
    TEST_ASSERT_EQUAL_UINT16(8, len);
}

void test_user_read_F180_mul(void)
{
    uint16_t len;
    ll_flash_read_fake.return_val = 0;
    user_read_data_by_id(1, 0, 0xF180, &len);
    TEST_ASSERT_EQUAL_UINT16(8, len);
}

void test_read_by_id_product_ident_match(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0xFF, 0x02, LEFT_FRONT_DOOR};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_diagservice_read_by_identifier(data, 6);
    TEST_ASSERT_EQUAL(1, lin_diag_positive_notify_fake.call_count);
    p->supplier_id = 0x0000;
}

void test_read_by_id_product_ident_config_not_match(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0xFF, 0x02, LEFT_FRONT_DOOR};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    g_user_info.config_word = LEFT_REAR_DOOR;
    lin_diagservice_read_by_identifier(data, 6);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    p->supplier_id = 0x0000;
}

void test_read_by_id_product_ident_invalid_door(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0xFF, 0x02, 0x04};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    lin_diagservice_read_by_identifier(data, 6);
    TEST_ASSERT_EQUAL(1, lin_diag_negative_notify_fake.call_count);
    p->supplier_id = 0x0000;
}

void test_lin_diagservice_read_by_identifier0(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0xFF, 0x00, 0x04};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    lin_diagservice_read_by_identifier(data, 6);
    p->supplier_id = 0x0000;
}

void test_lin_diagservice_read_by_identifier1(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0x00, 0x00, 0x04};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0x003F;
    pal_store_data_get_fake.return_val = true;
    lin_diagservice_read_by_identifier(data, 6);
    p->supplier_id = 0x0000;
}

void test_lin_diagservice_read_by_identifier2(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0xFF, 0x02, LEFT_REAR_DOOR};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    lin_diagservice_read_by_identifier(data, 6);
    p->supplier_id = 0x0000;
}

void test_lin_diagservice_read_by_identifier3(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0xFF, 0x02, RIGHT_FRONT_DOOR};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    lin_diagservice_read_by_identifier(data, 6);
    p->supplier_id = 0x0000;
}

void test_lin_diagservice_read_by_identifier4(void)
{
    uint8_t data[6] = {0x12, CUS_UDS_PRODUCT_IDENT, 0x3F, 0xFF, 0x02, RIGHT_REAR_DOOR};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    lin_diagservice_read_by_identifier(data, 6);
    p->supplier_id = 0x0000;
}

void test_lin_diagservice_read_by_identifier5(void)
{
    uint8_t data[6] = {0x12, 32, 0x3F, 0xFF, 0x02, RIGHT_FRONT_DOOR};
    lin_product_id *p = (lin_product_id *)&product_id;
    p->supplier_id = 0xFF3F;
    pal_store_data_get_fake.return_val = true;
    ld_read_by_id_callout_fake.custom_fake = ld_read_by_id_callout_custom_fake;
    lin_diagservice_read_by_identifier(data, 6);
    p->supplier_id = 0x0000;
}

void test_session_control_default_nad_68(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x04;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_session_control();
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
    TEST_ASSERT_EQUAL_UINT8(0x12, diagnosticTxBuffer[2]);
}

void test_session_control_82_no_02(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x82;
    diagRxSize = 2;
    program_condition_check = 0;
    uds_diagnostic_session_control();
}

void test_comm_control_enable_invalid_type_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x80;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_comm_control_disable_invalid_type(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_comm_control_disable_invalid_type_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_comm_control_disable_invalid_type_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_comm_control_enable_tx_invalid_type(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x82;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_communction_control();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_comm_control_enable_tx_invalid_type_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x82;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_comm_control_invalid_control_nad_7e(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x05;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_uds_communction_control0(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x80;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x70;
    uds_communction_control();
}

void test_uds_communction_control1(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x80;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
}

void test_uds_communction_control2(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x80;
    diagnosticRxBuffer[2] = 0x04;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_communction_control();
}

void test_uds_communction_control3(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
}

void test_uds_communction_control4(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x01;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_communction_control();
}

void test_uds_communction_control5(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x82;
    diagnosticRxBuffer[2] = 0x03;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_communction_control();
}

void test_uds_communction_control6(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x82;
    diagnosticRxBuffer[2] = 0x04;
    diagRxSize = 3;
    session_mode = SESSION_MODE_EXTEND;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_communction_control();
}

void test_clear_dtc_ff_ff_nonff(void)
{
    diagnosticRxBuffer[0] = 0x14;
    diagnosticRxBuffer[1] = 0xFF;
    diagnosticRxBuffer[2] = 0xFF;
    diagnosticRxBuffer[3] = 0x00;
    diagRxSize = 4;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_clear_dtc_info();
    TEST_ASSERT_EQUAL(1, ld_send_message_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(0x7F, diagnosticTxBuffer[0]);
}

void test_ecu_reset_subfunc_01_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 0;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_rest();
    TEST_ASSERT_EQUAL(1, ll_wdg_enable_fake.call_count);
}

void test_uds_diagnostic_rest0(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x01;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 1u;
    door_cmd.VehicleSpeed = 0x25u;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_rest();
}

void test_uds_diagnostic_rest1(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x81;
    diagRxSize = 2;
    door_cmd.VehicleSpeedValid = 1u;
    door_cmd.VehicleSpeed = 0x25u;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_rest();
}

void test_uds_diagnostic_rest2(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x02;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_rest();
}

void test_uds_diagnostic_rest3(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x03;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_rest();
}

void test_uds_diagnostic_rest4(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x04;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    uds_diagnostic_rest();
}

void test_uds_diagnostic_rest5(void)
{
    diagnosticRxBuffer[0] = 0x11;
    diagnosticRxBuffer[1] = 0x04;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_rest();
}

void test_assign_nad_supplier_match_not_id(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x00;
    diagnosticRxBuffer[3] = 0x01;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_INIT, g_config_word_state);
}

void test_assign_nad_save_wrong_state(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x03;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_START, g_config_word_state);
}

void test_assign_nad_save_nad_7f(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x03;
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    lin_current_rcvd_nad_fake.return_val = 0x7F;
    uds_diagnostic_assign_NAD();
    TEST_ASSERT_EQUAL_UINT8(CONFIGURE_WORD_STATE_ASIGN, g_config_word_state);
}

void test_uds_diagnostic_assign_NAD0(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x02;
    diagnosticRxBuffer[5] = RIGHT_REAR_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    lin_current_rcvd_nad_fake.return_val = 0x00;
    uds_diagnostic_assign_NAD();
}

void test_uds_diagnostic_assign_NAD1(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x02;
    diagnosticRxBuffer[5] = LEFT_REAR_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    lin_current_rcvd_nad_fake.return_val = 0x00;
    uds_diagnostic_assign_NAD();
}

void test_uds_diagnostic_assign_NAD2(void)
{
    diagnosticRxBuffer[0] = 0xB5;
    diagnosticRxBuffer[1] = 0xF3;
    diagnosticRxBuffer[2] = 0x3F;
    diagnosticRxBuffer[3] = 0x02;
    diagnosticRxBuffer[5] = 0x04;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    lin_current_rcvd_nad_fake.return_val = 0x00;
    uds_diagnostic_assign_NAD();
}

void test_lin_handle_uds_program_condition_nad_mismatch(void)
{
    program_condition_check = 1;
    lin_configured_NAD = 0x68;
    diagnosticRxBuffer[0] = SERVICE_TESTER_PRESENT;
    diagnosticRxBuffer[1] = 0x00;
    diagRxSize = 2;
    lin_current_rcvd_nad_fake.return_val = 0x69;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(1, program_condition_check);
}

void test_lin_handle_uds_service_2e_nad_7e(void)
{
    diagnosticRxBuffer[0] = SERVICE_WRITE_DATA_BY_IDENTIFY;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_lin_handle_uds_service_27_nad_7e(void)
{
    diagnosticRxBuffer[0] = SERVICE_SECURITY_ACCESS;
    lin_current_rcvd_nad_fake.return_val = 0x7E;
    lin_handle_uds();
    TEST_ASSERT_EQUAL(0, ld_send_message_fake.call_count);
}

void test_lin_handle_uds0(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x02;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

void test_lin_handle_uds1(void)
{
    diagnosticRxBuffer[0] = 0x10;
    diagnosticRxBuffer[1] = 0x00;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

void test_lin_handle_uds2(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x80;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

void test_lin_handle_uds3(void)
{
    diagnosticRxBuffer[0] = 0x3E;
    diagnosticRxBuffer[1] = 0x82;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

void test_lin_handle_uds4(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x82;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

void test_lin_handle_uds5(void)
{
    diagnosticRxBuffer[0] = 0x85;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x03;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

void test_lin_handle_uds6(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x83;
    diagnosticRxBuffer[2] = 0x00;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

void test_lin_handle_uds7(void)
{
    diagnosticRxBuffer[0] = 0x28;
    diagnosticRxBuffer[1] = 0x00;
    diagnosticRxBuffer[2] = 0x00;
    lin_configured_NAD = 0x68;
    lin_current_rcvd_nad_fake.return_val = 0x68;
    program_condition_check = 1;
    lin_handle_uds();
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_send_negative_response_tx_completed);
    RUN_TEST(test_send_negative_response_tx_timeout);
    RUN_TEST(test_send_negative_response_tx_other);
    RUN_TEST(test_send_positive_response_tx_completed);
    RUN_TEST(test_send_positive_response_tx_timeout);
    RUN_TEST(test_send_positive_response_tx_other);

    RUN_TEST(test_session_control_default_session);
    RUN_TEST(test_session_control_extend_session);
    RUN_TEST(test_session_control_program_session_no_condition);
    RUN_TEST(test_session_control_program_session_with_condition);
    RUN_TEST(test_session_control_wrong_length);
    RUN_TEST(test_session_control_nad_7e_no_response);
    RUN_TEST(test_session_control_nad_7f_no_response);
    RUN_TEST(test_session_control_program_session_nad_7e);
    RUN_TEST(test_session_control_unknown_subfunc_nad_7e);
    RUN_TEST(test_session_control_program_session_nad_7f);
    RUN_TEST(test_session_control_default_nad_7e);
    RUN_TEST(test_session_control_default_nad_7f);
    RUN_TEST(test_session_control_default_nad_68);
    RUN_TEST(test_session_control_82_no_02);

    RUN_TEST(test_route_control_nad_7e);
    RUN_TEST(test_route_control_nad_7f);
    RUN_TEST(test_route_control_wrong_length);
    RUN_TEST(test_route_control_wrong_subfunc);
    RUN_TEST(test_route_control_valid_route_not_extend);
    RUN_TEST(test_route_control_valid_route_extend_good_speed);
    RUN_TEST(test_route_control_valid_route_extend_bad_speed);
    RUN_TEST(test_route_control_routine_ff01);
    RUN_TEST(test_route_control_subfunc_81);
    RUN_TEST(test_route_control_routine_ff00);
    RUN_TEST(test_route_control_routine_dd02);
    RUN_TEST(test_route_control_other_routine);
    RUN_TEST(test_route_control_extend_speed_high);
    RUN_TEST(test_route_control_extend_speed_invalid);

    RUN_TEST(test_dtc_control_not_extend);
    RUN_TEST(test_dtc_control_extend_subfunc_01);
    RUN_TEST(test_dtc_control_extend_subfunc_02);
    RUN_TEST(test_dtc_control_extend_unknown_subfunc);
    RUN_TEST(test_dtc_control_wrong_length);
    RUN_TEST(test_dtc_control_extend_subfunc_01_nad_7e);
    RUN_TEST(test_dtc_control_extend_subfunc_01_nad_7f);
    RUN_TEST(test_dtc_control_not_extend_nad_7e);
    RUN_TEST(test_dtc_control_extend_subfunc_02_nad_7e);
    RUN_TEST(test_dtc_control_not_extend_nad_7f);
    RUN_TEST(test_dtc_control_extend_subfunc_02_nad_7f);
    RUN_TEST(test_dtc_control_extend_unknown_subfunc_nad_7f);
    RUN_TEST(test_uds_diagnostic_dtc_control0);
    RUN_TEST(test_uds_diagnostic_dtc_control1);
    RUN_TEST(test_uds_diagnostic_dtc_control2);

    RUN_TEST(test_clear_dtc_valid);
    RUN_TEST(test_clear_dtc_invalid_group);
    RUN_TEST(test_clear_dtc_wrong_length);
    RUN_TEST(test_clear_dtc_valid_nad_7e);
    RUN_TEST(test_clear_dtc_invalid_group_nad_7f);
    RUN_TEST(test_clear_dtc_invalid_bytes);
    RUN_TEST(test_clear_dtc_ff_ff_nonff);

    RUN_TEST(test_tester_present_subfunc_80);
    RUN_TEST(test_tester_present_subfunc_00);
    RUN_TEST(test_tester_present_wrong_length);
    RUN_TEST(test_tester_present_unknown_subfunc);
    RUN_TEST(test_tester_present_subfunc_00_nad_7e);
    RUN_TEST(test_tester_present_subfunc_00_nad_7f);
    RUN_TEST(test_tester_present_unknown_subfunc_nad_7f);
    RUN_TEST(test_uds_tester_present_control0);

    RUN_TEST(test_ecu_reset_subfunc_01_speed_valid);
    RUN_TEST(test_ecu_reset_subfunc_01_speed_invalid);
    RUN_TEST(test_ecu_reset_subfunc_02);
    RUN_TEST(test_ecu_reset_wrong_length);
    RUN_TEST(test_ecu_reset_subfunc_81_speed_valid);
    RUN_TEST(test_ecu_reset_subfunc_81_speed_invalid);
    RUN_TEST(test_ecu_reset_subfunc_03);
    RUN_TEST(test_ecu_reset_subfunc_82_83);
    RUN_TEST(test_ecu_reset_unknown_subfunc);
    RUN_TEST(test_ecu_reset_subfunc_01_speed_high);
    RUN_TEST(test_ecu_reset_subfunc_01_nad_7f);
    RUN_TEST(test_ecu_reset_subfunc_02_nad_7f);
    RUN_TEST(test_ecu_reset_subfunc_03_nad_7f);
    RUN_TEST(test_ecu_reset_subfunc_83);
    RUN_TEST(test_ecu_reset_subfunc_82_nad_7f);
    RUN_TEST(test_uds_diagnostic_rest0);
    RUN_TEST(test_uds_diagnostic_rest1);
    RUN_TEST(test_uds_diagnostic_rest2);
    RUN_TEST(test_uds_diagnostic_rest3);
    RUN_TEST(test_uds_diagnostic_rest4);
    RUN_TEST(test_uds_diagnostic_rest5);

    RUN_TEST(test_readdata_by_id_nad_7e);
    RUN_TEST(test_readdata_by_id_single_did_valid);
    RUN_TEST(test_readdata_by_id_single_did_invalid);
    RUN_TEST(test_readdata_by_id_wrong_length);
    RUN_TEST(test_readdata_by_id_did_F18A);
    RUN_TEST(test_readdata_by_id_did_F189);
    RUN_TEST(test_readdata_by_id_did_0216);
    RUN_TEST(test_readdata_by_id_did_F190);
    RUN_TEST(test_readdata_by_id_did_F0FA);
    RUN_TEST(test_readdata_by_id_did_0001);
    RUN_TEST(test_readdata_by_id_multi_did);
    RUN_TEST(test_readdata_by_id_multi_F197);
    RUN_TEST(test_readdata_by_id_multi_F189);
    RUN_TEST(test_readdata_by_id_multi_0216);
    RUN_TEST(test_readdata_by_id_multi_F184);
    RUN_TEST(test_readdata_by_id_multi_F089);
    RUN_TEST(test_readdata_by_id_multi_F180);
    RUN_TEST(test_readdata_by_id_multi_did_invalid);
    RUN_TEST(test_readdata_by_id_multi_did_partial_valid);
    RUN_TEST(test_readdata_by_id_multi_did_long);
    RUN_TEST(test_readdata_by_id_multi_include_0001);
    RUN_TEST(test_readdata_by_id_nad_7f);
    RUN_TEST(test_readdata_by_id_did_F197);
    RUN_TEST(test_readdata_by_id_did_F184);
    RUN_TEST(test_readdata_by_id_did_F089);
    RUN_TEST(test_readdata_by_id_did_F180);
    RUN_TEST(test_readdata_by_id_F197_RL);
    RUN_TEST(test_readdata_by_id_F197_FR);
    RUN_TEST(test_readdata_by_id_F197_RR);
    RUN_TEST(test_readdata_by_id_0216_RL);
    RUN_TEST(test_readdata_by_id_0216_FR);
    RUN_TEST(test_readdata_by_id_0216_RR);
    RUN_TEST(test_uds_diagnostic_readdata_by_id0);

    RUN_TEST(test_pal_store_data_set);

    RUN_TEST(test_lin_custom_diag_service_handle);
    RUN_TEST(test_lin_diag_service_hook);
    RUN_TEST(test_store_system_data_init);
    RUN_TEST(test_store_system_data_init_lock_index_clamped);

    RUN_TEST(test_session_check_default);
    RUN_TEST(test_session_check_not_expired);
    RUN_TEST(test_session_check_expired);
    RUN_TEST(test_session_check_lock_failed_decrement);
    RUN_TEST(test_session_check_lock_failed_not_exceeded);

    RUN_TEST(test_sys_do_flash_routine_flag_set);
    RUN_TEST(test_sys_do_flash_routine_flag_clear);

    RUN_TEST(test_lin_customized_operation_flag_set);
    RUN_TEST(test_lin_customized_operation_flag_clear);

    RUN_TEST(test_configword_remap_left_front);
    RUN_TEST(test_configword_remap_left_rear);
    RUN_TEST(test_configword_remap_right_front);
    RUN_TEST(test_configword_remap_right_rear);
    RUN_TEST(test_uds_diagnostic_configword_remap_nad0);

    RUN_TEST(test_assign_nad_nad_7e);
    RUN_TEST(test_assign_nad_fuc_id_01);
    RUN_TEST(test_assign_nad_fuc_id_02_valid_door);
    RUN_TEST(test_assign_nad_fuc_id_03);
    RUN_TEST(test_assign_nad_fuc_id_04);
    RUN_TEST(test_assign_nad_wrong_supplier);
    RUN_TEST(test_assign_nad_door_right_front);
    RUN_TEST(test_assign_nad_fuc_id_default);
    RUN_TEST(test_assign_nad_fuc_id_02_wrong_state);
    RUN_TEST(test_assign_nad_fuc_id_03_wrong_state_nad_7f);
    RUN_TEST(test_assign_nad_save_wrong_state);
    RUN_TEST(test_assign_nad_save_nad_7f);
    RUN_TEST(test_assign_nad_supplier_match_not_id);
    RUN_TEST(test_uds_diagnostic_assign_NAD0);
    RUN_TEST(test_uds_diagnostic_assign_NAD1);
    RUN_TEST(test_uds_diagnostic_assign_NAD2);

    RUN_TEST(test_comm_control_not_extend);
    RUN_TEST(test_comm_control_wrong_length);
    RUN_TEST(test_comm_control_enable_rx);
    RUN_TEST(test_comm_control_enable_rx_subfunc_00);
    RUN_TEST(test_comm_control_enable_rx_type_03);
    RUN_TEST(test_comm_control_enable_rx_nad_7f);
    RUN_TEST(test_comm_control_disable_rx);
    RUN_TEST(test_comm_control_disable_rx_type_03);
    RUN_TEST(test_comm_control_invalid_control);
    RUN_TEST(test_comm_control_invalid_control_nad_7e);
    RUN_TEST(test_comm_control_enable_invalid_type_nad_7e);
    RUN_TEST(test_comm_control_disable_invalid_type);
    RUN_TEST(test_comm_control_disable_invalid_type_nad_7e);
    RUN_TEST(test_comm_control_disable_invalid_type_nad_7f);
    RUN_TEST(test_comm_control_enable_tx_invalid_type);
    RUN_TEST(test_comm_control_enable_tx_invalid_type_nad_7e);
    RUN_TEST(test_comm_control_unknown_subfunc);
    RUN_TEST(test_comm_control_subfunc_02);
    RUN_TEST(test_comm_control_subfunc_82);
    RUN_TEST(test_comm_control_subfunc_82_nad_7f);
    RUN_TEST(test_uds_communction_control0);
    RUN_TEST(test_uds_communction_control1);
    RUN_TEST(test_uds_communction_control2);
    RUN_TEST(test_uds_communction_control3);
    RUN_TEST(test_uds_communction_control4);
    RUN_TEST(test_uds_communction_control5);
    RUN_TEST(test_uds_communction_control6);

    RUN_TEST(test_user_read_default);
    RUN_TEST(test_user_read_F197_RL_mul);
    RUN_TEST(test_user_read_F197_FR_mul);
    RUN_TEST(test_user_read_F197_RR_mul);
    RUN_TEST(test_user_read_F197_NN_mul);
    RUN_TEST(test_user_read_0216_RL_mul);
    RUN_TEST(test_user_read_0216_FR_mul);
    RUN_TEST(test_user_read_0216_RR_mul);
    RUN_TEST(test_user_read_0216_NN_mul);
    RUN_TEST(test_user_read_0216_NN_mul0);
    RUN_TEST(test_user_read_0001_mul);
    RUN_TEST(test_user_read_F184_mul);
    RUN_TEST(test_user_read_F089_mul);
    RUN_TEST(test_user_read_F180_mul);
    RUN_TEST(test_comm_control_unknown_subfunc_nad_7f);
    RUN_TEST(test_comm_control_unknown_subfunc_nad_7e);

    RUN_TEST(test_lin_handle_uds_service_10);
    RUN_TEST(test_lin_handle_uds_service_14);
    RUN_TEST(test_lin_handle_uds_service_22);
    RUN_TEST(test_lin_handle_uds_service_85);
    RUN_TEST(test_lin_handle_uds_service_28);
    RUN_TEST(test_lin_handle_uds_service_31);
    RUN_TEST(test_lin_handle_uds_service_3e);
    RUN_TEST(test_lin_handle_uds_service_11);
    RUN_TEST(test_lin_handle_uds_service_2e);
    RUN_TEST(test_lin_handle_uds_service_27);
    RUN_TEST(test_lin_handle_uds_service_b5);
    RUN_TEST(test_lin_handle_uds_default);
    RUN_TEST(test_lin_handle_uds_program_condition_active);
    RUN_TEST(test_lin_handle_uds_program_condition_cleared);
    RUN_TEST(test_lin_handle_uds_program_condition_service_28);
    RUN_TEST(test_lin_handle_uds_service_2e_nad_7f);
    RUN_TEST(test_lin_handle_uds_service_27_nad_7f);
    RUN_TEST(test_lin_handle_uds_default_nad_7f);
    RUN_TEST(test_lin_handle_uds_program_condition_service_10_82);
    RUN_TEST(test_lin_handle_uds_program_condition_nad_mismatch);
    RUN_TEST(test_lin_handle_uds_service_2e_nad_7e);
    RUN_TEST(test_lin_handle_uds_service_27_nad_7e);
    RUN_TEST(test_lin_handle_uds_default_nad_7e);
    RUN_TEST(test_lin_handle_uds0);
    RUN_TEST(test_lin_handle_uds1);
    RUN_TEST(test_lin_handle_uds2);
    RUN_TEST(test_lin_handle_uds3);
    RUN_TEST(test_lin_handle_uds4);
    RUN_TEST(test_lin_handle_uds5);
    RUN_TEST(test_lin_handle_uds6);
    RUN_TEST(test_lin_handle_uds7);

    RUN_TEST(test_read_by_id_supplier_not_match);
    RUN_TEST(test_read_by_id_id_in_range_callout_positive);
    RUN_TEST(test_read_by_id_id_in_range_callout_negative);
    RUN_TEST(test_read_by_id_id_out_of_range);
    RUN_TEST(test_read_by_id_product_ident_format_mismatch);
    RUN_TEST(test_read_by_id_product_ident_match);
    RUN_TEST(test_read_by_id_product_ident_config_not_match);
    RUN_TEST(test_read_by_id_product_ident_invalid_door);
    RUN_TEST(test_read_by_id_supplier_match);
    RUN_TEST(test_read_by_id_supplier_any);
    RUN_TEST(test_read_by_id_id_between_ranges);
    RUN_TEST(test_read_by_id_callout_no_response);
    RUN_TEST(test_read_by_id_callout_all_ff);
    RUN_TEST(test_lin_diagservice_read_by_identifier0);
    RUN_TEST(test_lin_diagservice_read_by_identifier1);
    RUN_TEST(test_lin_diagservice_read_by_identifier2);
    RUN_TEST(test_lin_diagservice_read_by_identifier3);
    RUN_TEST(test_lin_diagservice_read_by_identifier4);
    RUN_TEST(test_lin_diagservice_read_by_identifier5);

    return UNITY_END();
}
