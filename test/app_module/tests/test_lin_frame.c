#include "unity.h"
#include "fff.h"
#include "fff_lin_frame.h"
#include "fff_lin_cfg.h"
#include "fff_lin.h"
#include "fff_custom_diagnosticIII.h"

DEFINE_FFF_GLOBALS;

extern void AppSetLinErrByNad(uint8_t err_flag);
extern void App_LinSendDoorState(void);
extern void App_LinReceiveDoorState(void);

extern user_cfg_t g_user_info;
extern DoorSt_T door_st;
extern DoorCmd_T door_cmd;
extern volatile uint8_t lin_error;

static void clear_frame_buffer(void)
{
    memset(lin_pFrameBuf, 0, LIN_FRAME_BUF_SIZE);
    memset(lin_flag_handle_tbl, 0, LIN_FLAG_BUF_SIZE);
    memset(lin_frame_flag_tbl, 0, LIN_NUM_OF_FRMS);
    memset(&door_st, 0, sizeof(door_st));
    memset(&door_cmd, 0, sizeof(door_cmd));
}

static void set_flag_fl_state(void)
{
    lin_frame_flag_tbl[LI0_EHIS_FL_State] = 1;
}
static void set_flag_rl_state(void)
{
    lin_frame_flag_tbl[LI0_EHIS_RL_State] = 1;
}
static void set_flag_fr_state(void)
{
    lin_frame_flag_tbl[LI0_EHIS_FR_State] = 1;
}
static void set_flag_rr_state(void)
{
    lin_frame_flag_tbl[LI0_EHIS_RR_State] = 1;
}
static void set_flag_viu_dws(void)
{
    lin_frame_flag_tbl[LI0_VIU_DWS] = 1;
}

void setUp(void)
{
    RESET_FAKE(uds_diagnostic_configword_remap_nad);
    clear_frame_buffer();
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_error = 0;
}

void tearDown(void)
{
}

/* ========== AppSetLinErrByNad ========== */

void test_set_err_fl_sets_bit(void)
{
    g_user_info.config_word = LEFT_FRONT_DOOR;
    AppSetLinErrByNad(1);
    TEST_ASSERT_BIT_HIGH(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError]);
}

void test_set_err_fl_clears_bit(void)
{
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError] = 0xFF;
    AppSetLinErrByNad(0);
    TEST_ASSERT_BIT_LOW(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError]);
}

void test_set_err_rl_sets_bit(void)
{
    g_user_info.config_word = LEFT_REAR_DOOR;
    AppSetLinErrByNad(1);
    TEST_ASSERT_BIT_HIGH(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RL_ResponseError]);
}

void test_set_err_fr_sets_bit(void)
{
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    AppSetLinErrByNad(1);
    TEST_ASSERT_BIT_HIGH(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FR_ResponseError]);
}

void test_set_err_rr_sets_bit(void)
{
    g_user_info.config_word = RIGHT_REAR_DOOR;
    AppSetLinErrByNad(1);
    TEST_ASSERT_BIT_HIGH(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError]);
}

void test_set_err_rr_clears_bit(void)
{
    g_user_info.config_word = RIGHT_REAR_DOOR;
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError] = 0xFF;
    AppSetLinErrByNad(0);
    TEST_ASSERT_BIT_LOW(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError]);
}

void test_set_err_unknown_door_does_nothing(void)
{
    g_user_info.config_word = 0xFF;
    clear_frame_buffer();
    AppSetLinErrByNad(1);
    for (uint8_t i = 0; i < LIN_FRAME_BUF_SIZE; i++)
    {
        TEST_ASSERT_EQUAL_UINT8(0, lin_pFrameBuf[i]);
    }
}

/* ========== App_LinSendDoorState ========== */

void test_send_lin_error_set_applies_error_flag(void)
{
    lin_error = 1;
    App_LinSendDoorState();
    TEST_ASSERT_BIT_HIGH(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError]);
}

void test_send_lin_error_clear_applies_no_error_flag(void)
{
    lin_error = 0;
    lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError] = 0xFF;
    App_LinSendDoorState();
    TEST_ASSERT_BIT_LOW(7, lin_pFrameBuf[LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError]);
}

/* ========== App_LinSendDoorState: FL state block ========== */

void test_send_fl_state_config_match_processes_signals(void)
{
    lin_error = 1;
    g_user_info.config_word = LEFT_FRONT_DOOR;
    set_flag_fl_state();
    door_st.FltSt = 1;
    door_st.SwtSt = 2;
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_FL_State]);
    TEST_ASSERT_EQUAL_UINT8(0, lin_error);
    TEST_ASSERT_EQUAL(0, uds_diagnostic_configword_remap_nad_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(LEFT_FRONT_DOOR, g_user_info.config_word);
}

void test_send_fl_state_config_mismatch_updates_config(void)
{
    g_user_info.config_word = LEFT_REAR_DOOR;
    set_flag_fl_state();
    door_st.FltSt = 1;
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL(1, uds_diagnostic_configword_remap_nad_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(LEFT_FRONT_DOOR, g_user_info.config_word);
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_FL_State]);
}

void test_send_fl_state_fltst0(void)
{
    g_user_info.config_word = LEFT_FRONT_DOOR;
    set_flag_fl_state();
    door_st.FltSt = 0;
    App_LinSendDoorState();
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_FL_State]);
}

/* ========== App_LinSendDoorState: RL state block ========== */

void test_send_rl_state_config_match_processes_signals(void)
{
    g_user_info.config_word = LEFT_REAR_DOOR;
    set_flag_rl_state();
    door_st.SW_MajorVersA = 5;
    lin_error = 1;
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_RL_State]);
    TEST_ASSERT_EQUAL_UINT8(0, lin_error);
    TEST_ASSERT_EQUAL(0, uds_diagnostic_configword_remap_nad_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(LEFT_REAR_DOOR, g_user_info.config_word);
}

void test_send_rl_state_config_mismatch_updates_config(void)
{
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    set_flag_rl_state();
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL(1, uds_diagnostic_configword_remap_nad_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(LEFT_REAR_DOOR, g_user_info.config_word);
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_RL_State]);
}

void test_send_rl_state_fltst1(void)
{
    g_user_info.config_word = LEFT_REAR_DOOR;
    set_flag_rl_state();
    door_st.FltSt = 1;
    App_LinSendDoorState();
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_RL_State]);
}

/* ========== App_LinSendDoorState: FR state block ========== */

void test_send_fr_state_config_match_processes_signals(void)
{
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    set_flag_fr_state();
    door_st.HW_MajorVersB = 0x0A;
    lin_error = 1;
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_FR_State]);
    TEST_ASSERT_EQUAL_UINT8(0, lin_error);
    TEST_ASSERT_EQUAL(0, uds_diagnostic_configword_remap_nad_fake.call_count);
}

void test_send_fr_state_config_mismatch_updates_config(void)
{
    g_user_info.config_word = LEFT_REAR_DOOR;
    set_flag_fr_state();
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL(1, uds_diagnostic_configword_remap_nad_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(RIGHT_FRONT_DOOR, g_user_info.config_word);
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_FR_State]);
}

void test_send_fr_state_fltst1(void)
{
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    set_flag_fr_state();
    door_st.FltSt = 1;
    App_LinSendDoorState();
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_FR_State]);
}

/* ========== App_LinSendDoorState: RR state block ========== */

void test_send_rr_state_config_match_processes_signals(void)
{
    g_user_info.config_word = RIGHT_REAR_DOOR;
    set_flag_rr_state();
    door_st.SN_SupplierCod = 3;
    lin_error = 1;
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_RR_State]);
    TEST_ASSERT_EQUAL_UINT8(0, lin_error);
    TEST_ASSERT_EQUAL(0, uds_diagnostic_configword_remap_nad_fake.call_count);
}

void test_send_rr_state_config_mismatch_updates_config(void)
{
    g_user_info.config_word = LEFT_FRONT_DOOR;
    set_flag_rr_state();
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL(1, uds_diagnostic_configword_remap_nad_fake.call_count);
    TEST_ASSERT_EQUAL_UINT8(RIGHT_REAR_DOOR, g_user_info.config_word);
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_RR_State]);
}

/* ========== App_LinSendDoorState: multiple flags ========== */

void test_send_multiple_state_flags_all_processed(void)
{
    set_flag_fl_state();
    set_flag_rr_state();
    door_st.FltSt = 1;
    door_st.HW_MinorVersB = 5;
    lin_error = 1;
    App_LinSendDoorState();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_FL_State]);
    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_EHIS_RR_State]);
    TEST_ASSERT_EQUAL_UINT8(0, lin_error);
}

/* ========== App_LinReceiveDoorState ========== */

void test_recv_no_flag_does_nothing(void)
{
    App_LinReceiveDoorState();
    TEST_ASSERT_EQUAL_UINT8(0, door_cmd.UsageMode);
    TEST_ASSERT_EQUAL_UINT8(0, door_cmd.VehicleSpeedValid);
    TEST_ASSERT_EQUAL_UINT16(0, door_cmd.VehicleSpeed);
}

void test_recv_left_front_decodes_correctly(void)
{
    g_user_info.config_word = LEFT_FRONT_DOOR;
    set_flag_viu_dws();
    lin_pFrameBuf[32] = 0x4C;
    lin_pFrameBuf[33] = 0x00;
    lin_pFrameBuf[34] = 0x00;
    App_LinReceiveDoorState();

    TEST_ASSERT_EQUAL_UINT8(3, door_cmd.UsageMode);
    TEST_ASSERT_EQUAL_UINT8(1, door_cmd.VehicleSpeedValid);
    TEST_ASSERT_EQUAL_UINT16(0, door_cmd.VehicleSpeed);
}

void test_recv_left_rear_decodes_correctly(void)
{
    g_user_info.config_word = LEFT_REAR_DOOR;
    set_flag_viu_dws();
    lin_pFrameBuf[32] = 0xC0;
    lin_pFrameBuf[33] = 0x07;
    lin_pFrameBuf[34] = 0x00;
    App_LinReceiveDoorState();

    TEST_ASSERT_EQUAL_UINT8(15, door_cmd.UsageMode);
    TEST_ASSERT_EQUAL_UINT8(1, door_cmd.VehicleSpeedValid);
    TEST_ASSERT_EQUAL_UINT16(0, door_cmd.VehicleSpeed);
}

void test_recv_right_front_decodes_correctly(void)
{
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    set_flag_viu_dws();
    lin_pFrameBuf[32] = 0x00;
    lin_pFrameBuf[33] = 0x0F;
    lin_pFrameBuf[34] = 0x00;
    App_LinReceiveDoorState();

    TEST_ASSERT_EQUAL_UINT8(15, door_cmd.UsageMode);
    TEST_ASSERT_EQUAL_UINT8(0, door_cmd.VehicleSpeedValid);
    TEST_ASSERT_EQUAL_UINT16(0, door_cmd.VehicleSpeed);
}

void test_recv_right_rear_decodes_correctly(void)
{
    g_user_info.config_word = RIGHT_REAR_DOOR;
    set_flag_viu_dws();
    lin_pFrameBuf[32] = 0x00;
    lin_pFrameBuf[33] = 0x1F;
    lin_pFrameBuf[34] = 0x00;
    App_LinReceiveDoorState();

    TEST_ASSERT_EQUAL_UINT8(15, door_cmd.UsageMode);
    TEST_ASSERT_EQUAL_UINT8(1, door_cmd.VehicleSpeedValid);
    TEST_ASSERT_EQUAL_UINT16(0, door_cmd.VehicleSpeed);
}

void test_recv_flag_cleared(void)
{
    g_user_info.config_word = LEFT_FRONT_DOOR;
    set_flag_viu_dws();
    App_LinReceiveDoorState();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_VIU_DWS]);
}

void test_App_LinReceiveDoorState(void)
{
    g_user_info.config_word = 0x04;
    set_flag_viu_dws();
    App_LinReceiveDoorState();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_VIU_DWS]);
}

/* ========== App_LinControlMsg ========== */

void test_control_msg_calls_send_and_receive(void)
{
    set_flag_viu_dws();
    g_user_info.config_word = LEFT_FRONT_DOOR;
    lin_pFrameBuf[32] = 0x4C;
    lin_pFrameBuf[33] = 0x00;
    lin_pFrameBuf[34] = 0x00;
    App_LinControlMsg();

    TEST_ASSERT_EQUAL_UINT8(0, lin_frame_flag_tbl[LI0_VIU_DWS]);
    TEST_ASSERT_EQUAL_UINT8(1, door_cmd.VehicleSpeedValid);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_set_err_fl_sets_bit);
    RUN_TEST(test_set_err_fl_clears_bit);
    RUN_TEST(test_set_err_rl_sets_bit);
    RUN_TEST(test_set_err_fr_sets_bit);
    RUN_TEST(test_set_err_rr_sets_bit);
    RUN_TEST(test_set_err_rr_clears_bit);
    RUN_TEST(test_set_err_unknown_door_does_nothing);
    RUN_TEST(test_send_lin_error_set_applies_error_flag);
    RUN_TEST(test_send_lin_error_clear_applies_no_error_flag);
    RUN_TEST(test_send_fl_state_config_match_processes_signals);
    RUN_TEST(test_send_fl_state_config_mismatch_updates_config);
    RUN_TEST(test_send_fl_state_fltst0);
    RUN_TEST(test_send_rl_state_config_match_processes_signals);
    RUN_TEST(test_send_rl_state_config_mismatch_updates_config);
    RUN_TEST(test_send_rl_state_fltst1);
    RUN_TEST(test_send_fr_state_config_match_processes_signals);
    RUN_TEST(test_send_fr_state_config_mismatch_updates_config);
    RUN_TEST(test_send_fr_state_fltst1);
    RUN_TEST(test_send_rr_state_config_match_processes_signals);
    RUN_TEST(test_send_rr_state_config_mismatch_updates_config);
    RUN_TEST(test_send_multiple_state_flags_all_processed);
    RUN_TEST(test_recv_no_flag_does_nothing);
    RUN_TEST(test_recv_left_front_decodes_correctly);
    RUN_TEST(test_recv_left_rear_decodes_correctly);
    RUN_TEST(test_recv_right_front_decodes_correctly);
    RUN_TEST(test_recv_right_rear_decodes_correctly);
    RUN_TEST(test_recv_flag_cleared);
    RUN_TEST(test_control_msg_calls_send_and_receive);
    RUN_TEST(test_App_LinReceiveDoorState);
    return UNITY_END();
}
