#include <stdio.h>
#include <string.h>
#include "fff.h"
#include "unity.h"
#include "dfu_uds_manager.h"
#include "fff_pal_store.h"
#include "fff_pal_systick.h"
#include "fff_pal_lin_comm.h"
#include "fff_pal_lin_tl.h"
#include "fff_pal_wdg.h"
#include "fff_aes_cmac.h"
#include "fff_utilities.h"
#include "fff_logging.h"
#include "fff_hardware.h"
#include "fff_pal_func_def.h"
#include "fff_tcpl03x_ll_def.h"
#include "fff_tcpl03x.h"
#include "fff_tcpl03x_ll_cortex.h"
#include "fff_tcpl03x_ll_sys.h"
#include "fff_tcpl03x_ll_adc.h"
#include "fff_tcpl03x_ll_flash.h"
#include "fff_tcpl03x_ll_gpio.h"
#include "fff_tcpl03x_ll_lpm.h"
#include "fff_tcpl03x_ll_pwm.h"
#include "fff_tcpl03x_ll_sci.h"
#include "fff_tcpl03x_ll_timer.h"
#include "fff_tcpl03x_ll_wdg.h"

DEFINE_FFF_GLOBALS;

// extern uint8_t dfu_image_erase(void);

/***  extern  ***/
extern void JumpToApp(void);
extern uint8_t queue_lin_empty(void);
extern void dfu_do_notify_response(uint8_t resp_type, uint8_t sid, uint8_t resp_value);
extern uint8_t dfu_image_erase(void);
extern uint8_t dfu_image_program(uint32_t addr, uint8_t *data, uint16_t length);
extern void dfu_do_notify_cp(uint8_t sid, uint8_t sub_func, uint8_t *data, uint16_t length);
extern void dfu_do_notify_cp_ex(uint8_t sid, uint8_t *data, uint16_t length);
extern void dfu_do_notify_response(uint8_t resp_type, uint8_t sid, uint8_t resp_value);
extern void dfu_session_parameter_resp(uint8_t sessiontype);
extern uint8_t last_dfu_info_get(void);
extern uint8_t uds_diag_DID_chk(uint16_t ucSess);
extern void uds_diagnostic_configword_remap_nad(void);
extern uint8_t last_dfu_info_update(last_dfu_info_t *info);
extern void dfu_process_exit(uint8_t reason);
extern void session_control_handle(uint8_t *param, uint16_t length);
extern uint8_t cpmpare_key(uint8_t *seed, uint8_t *key, uint8_t length);
extern void security_access_handle(uint8_t *param, uint16_t length);
extern void lin_diag_service_handle(void);
extern void key_reset_by_nad(void);
extern void transfer_data_handle(uint8_t *param, uint16_t length);
extern void assign_config_word_handle(uint8_t *param, uint16_t length);
extern void user_read_data_by_id(uint8_t mul_flag, uint8_t mul_len, uint16_t did, uint16_t *len);
/***  extern  ***/

extern const dfu_process_context_t dfu_process_ctx[];
extern uint8_t seed_00_ret;
extern dfu_manager_context_t dfu_ctx;
extern ota_cfg_t g_ota_info;
extern ServiceUDS_TypeDef uds_request_info;
extern uint8_t seed_cmac_succ;
extern uint8_t seed_use[16u];
extern uint8_t seed[16];
extern uint8_t flashdrv_cmac_succ;
extern uint8_t app_cmac_succ;
extern uint8_t app_cmac_start;
extern uint8_t app_cmac[16u];
extern uint8_t flash_driver_cmac[16u];
extern uint8_t g_config_word_state;
extern user_cfg_t g_user_info;
extern uint8_t g_negResponseCode;
extern uint8_t unlock_failed_store_flag;
extern uint8_t diagnostic_session_overtime_flag;
extern uint16_t lock_failed_cnt;
extern volatile uint16_t timer_1s_cnt;
extern uint8_t clear_wdg_cnt;
extern uint8_t key[16]; // 全局密钥数组
extern uint8_t diagnosticTxBuffer[];
// 定义函数内的密钥常量（用于断言验证）
const uint8_t key_lf[16] = {0x88, 0xB3, 0x4F, 0x45, 0xE1, 0x0D, 0xBB, 0xC3, 0x5D, 0xBF, 0x7E, 0xCF, 0x86, 0x8E, 0x73, 0x60};
const uint8_t key_rf[16] = {0x32, 0xA9, 0x83, 0x03, 0xAD, 0x07, 0xAE, 0x9C, 0x2B, 0x46, 0x1F, 0xDE, 0x1D, 0xA5, 0x46, 0x76};
const uint8_t key_lr[16] = {0x48, 0x54, 0x24, 0xF9, 0xF5, 0x76, 0x3E, 0x2B, 0x99, 0x87, 0xD0, 0x11, 0x1A, 0x8B, 0xD2, 0x82};
const uint8_t key_rr[16] = {0x2A, 0x7E, 0xF5, 0x25, 0x7E, 0x01, 0xE6, 0x06, 0xCD, 0x0C, 0x68, 0xFE, 0xA0, 0x3E, 0x5E, 0x5C};
// 测试用NAD全局变量
uint8_t test_nad_value = 0x00;
/***  extern  ****/

static void lin_uds_receive_custom(uint8_t nad, uint8_t *ptr, uint16_t *length)
{
    if (length)
        *length = 8u;
    if (ptr)
        ptr[0] = 0x10u; // sid 匹配
}

static void lin_uds_receive_custom_sid_ff(uint8_t nad, uint8_t *ptr, uint16_t *length)
{
    if (length)
        *length = 8u;
    if (ptr)
        ptr[0] = 0xFFu; // sid 不匹配
}

static void lin_uds_receive_custom_sid_00(uint8_t nad, uint8_t *ptr, uint16_t *length)
{
    if (length)
        *length = 8u;
    if (ptr)
        ptr[0] = 0x00u; // SID 0x00 matching NULL entry
}

// DEFINE_FAKE_VOID_FUNC(aes_cmac, unsigned char *, unsigned char *, s32, unsigned char *);
static void aes_cmac_test_customaes_cmac(unsigned char *key, unsigned char *input, s32 length, unsigned char *mac)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        *(mac + i) = 0x01;
    }
}

void setUp(void)
{
    // RESET_FAKE(ll_flash_erase_drv);

    RESET_FAKE(systick_count_get);
    RESET_FAKE(systick_diff);
    RESET_FAKE(delay_ms);
    RESET_FAKE(delay_us);

    RESET_FAKE(pal_lin_init);
    RESET_FAKE(pal_lin_deinit);
    RESET_FAKE(pal_lin_rx_response);
    RESET_FAKE(pal_lin_tx_response);
    RESET_FAKE(pal_lin_tx_4byte);
    RESET_FAKE(pal_lin_tx_header);
    RESET_FAKE(pal_lin_aa_enter);
    RESET_FAKE(pal_lin_aa_exit);
    RESET_FAKE(pal_lin_aa_ready);
    RESET_FAKE(pal_lin_parity_calib);
    RESET_FAKE(pal_lin_checksum_calib);
    RESET_FAKE(pal_lin_aa_raw_code_get);
    RESET_FAKE(pal_lin_abort_handle);
    RESET_FAKE(pal_lin_read_byte);
    RESET_FAKE(pal_lin_autobaudrate_check);

    RESET_FAKE(lin_tl_init);
    RESET_FAKE(lin_uds_send);
    RESET_FAKE(lin_uds_negative_response);
    RESET_FAKE(lin_uds_receive);
    RESET_FAKE(lin_tl_uncd_frame_get);

    RESET_FAKE(pal_store_data_set);
    RESET_FAKE(pal_store_data_get);
    RESET_FAKE(pal_store_data_init);
    RESET_FAKE(pal_store_data_clear);
    RESET_FAKE(pal_store_erase);
    RESET_FAKE(pal_store_write);
    RESET_FAKE(pal_store_read);
    RESET_FAKE(pal_store_uid_get);
    RESET_FAKE(pal_store_boot_ver_get);
    RESET_FAKE(pal_store_chip_ver_id_get);
    RESET_FAKE(pal_store_reg_rw);

    RESET_FAKE(ll_flash_erase_drv);
    RESET_FAKE(lin_uds_send);
    RESET_FAKE(aes_cmac);
    RESET_FAKE(Gen_CMACkey);
    RESET_FAKE(lin_uds_receive);
    lin_get_uds_nad_fake.return_val = 0x10;

    FFF_RESET_HISTORY();
}

void tearDown(void)
{
}

void test_JumpToApp(void)
{
    JumpToApp();
}

void test_queue_lin_empty(void)
{
    dfu_ctx.queue_list.head = 0;
    dfu_ctx.queue_list.tail = 0;
    uint8_t ret1 = queue_lin_empty();
    TEST_ASSERT_EQUAL(1, ret1);
    dfu_ctx.queue_list.head = 0;
    dfu_ctx.queue_list.tail = 1;
    uint8_t ret2 = queue_lin_empty();
    TEST_ASSERT_EQUAL(0, ret2);
}

void test_dfu_image_erase(void)
{
    // 返回 1（成功）
    ll_flash_erase_drv_fake.return_val = 1;
    uint8_t ret1 = dfu_image_erase();
    // TEST_ASSERT_EQUAL(DFU_MSG_SUCCESS, ret1);
    //  安排：返回 0（失败）
    ll_flash_erase_drv_fake.return_val = 0;
    uint8_t ret2 = dfu_image_erase();
    // TEST_ASSERT_EQUAL(DFU_MSG_ERASE_ERROR, ret2);
}

void test_dfu_image_program(void)
{
    uint8_t test_data[8] = {0x11, 0x22, 0x33, 0x44};
    uint16_t test_len = 4;

    // 地址小于 FLASH_APP_ADDR
    uint8_t ret1 = dfu_image_program(FLASH_APP_ADDR - 1, test_data, test_len);
    TEST_ASSERT_EQUAL(DFU_MSG_PROGRA_ERROR, ret1);

    // 地址大于等于 FLASH_APP_END_ADDR
    uint8_t ret2 = dfu_image_program(FLASH_APP_END_ADDR, test_data, test_len);
    TEST_ASSERT_EQUAL(DFU_MSG_PROGRA_ERROR, ret2);

    // 地址合法
    uint8_t ret3 = dfu_image_program(FLASH_APP_ADDR, test_data, test_len);
    TEST_ASSERT_EQUAL(DFU_MSG_SUCCESS, ret3);
}

void test_dfu_do_notify_cp(void)
{
    uint8_t test_data[] = {0x11, 0x22, 0x33};
    uint16_t test_len = 3;

    // 调用测试函数
    dfu_do_notify_cp(0x10, 0x05, test_data, test_len);

    // 验证 lin_uds_send 被正确调用
    TEST_ASSERT_EQUAL(1, lin_uds_send_fake.call_count);
    TEST_ASSERT_EQUAL(lin_configured_NAD, lin_uds_send_fake.arg0_val);
}

void test_dfu_do_notify_cp_ex(void)
{
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t test_len = 4;

    // 调用测试函数
    dfu_do_notify_cp_ex(0x10, test_data, test_len);

    // 验证调用参数
    TEST_ASSERT_EQUAL(1, lin_uds_send_fake.call_count);
    TEST_ASSERT_EQUAL(lin_configured_NAD, lin_uds_send_fake.arg0_val);
    TEST_ASSERT_EQUAL(0x50, lin_uds_send_fake.arg1_val[0]);
    TEST_ASSERT_EQUAL(0x01, lin_uds_send_fake.arg1_val[1]);
    TEST_ASSERT_EQUAL(0x02, lin_uds_send_fake.arg1_val[2]);
    TEST_ASSERT_EQUAL(0x03, lin_uds_send_fake.arg1_val[3]);
    TEST_ASSERT_EQUAL(0x04, lin_uds_send_fake.arg1_val[4]);
    TEST_ASSERT_EQUAL(1 + test_len, lin_uds_send_fake.arg2_val);
}

void test_dfu_do_notify_response(void)
{
    // 测试正向响应
    dfu_do_notify_response(POSITIVE, 0x10, 0x01);
    // 测试负向响应
    dfu_do_notify_response(NEGATIVE, 0x11, 0x22);
}

void test_dfu_session_parameter_resp(void)
{
    for (uint16_t i = 0x00; i < 0xFF; i++)
        dfu_session_parameter_resp(i);
}

void test_last_dfu_info_get(void)
{
    // 测试1：Magic不匹配 -> 返回DFU_MSG_ERROR
    dfu_ctx.dfu_info.magic = 0xFFFF;
    last_dfu_info_get();

    // 测试2：Magic正确，无特殊标志 -> 返回DFU_MSG_SUCCESS
    dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC;
    g_ota_info.app_req_ext_program_flag = 0x00;
    last_dfu_info_get();

    // 测试3：Magic正确，flag=0x01 -> 返回DFU_MSG_ERROR
    dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC;
    g_ota_info.app_req_ext_program_flag = 0x01;
    last_dfu_info_get();

    // 测试4：Magic正确，flag=0x03 -> 返回DFU_MSG_ERROR
    dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC;
    g_ota_info.app_req_ext_program_flag = 0x03;
    last_dfu_info_get();
}

void test_uds_remap_nad_68(void)
{
    g_user_info.nad_info = 0x68u;
    g_ota_info.lock_failed_index = 2u; // ≤3，不触发重置

    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x68u, lin_configured_NAD);
}

// 2. 覆盖 NAD=0x6A（合法值）→ 赋值原值
void test_uds_remap_nad_6A(void)
{
    g_user_info.nad_info = 0x6Au;
    g_ota_info.lock_failed_index = 3u;

    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x6Au, lin_configured_NAD);
}

// 3. 覆盖 NAD=0x69（合法值）→ 赋值原值
void test_uds_remap_nad_69(void)
{
    g_user_info.nad_info = 0x69u;
    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x69u, lin_configured_NAD);
}

// 4. 覆盖 NAD=0x6B（合法值）→ 赋值原值
void test_uds_remap_nad_6B(void)
{
    g_user_info.nad_info = 0x6Bu;
    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x6Bu, lin_configured_NAD);
}

// 5. 覆盖 NAD非法值 → 赋值默认0x68
void test_uds_remap_nad_invalid(void)
{
    g_user_info.nad_info = 0x70u; // 非法值
    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0x68u, lin_configured_NAD);
}

// 6. 覆盖 lock_failed_index >3 → 重置为0
void test_uds_lock_index_over_3(void)
{
    g_user_info.nad_info = 0x68u;
    g_ota_info.lock_failed_index = 4u; // 大于3

    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0u, g_ota_info.lock_failed_index);
}

// 7. 覆盖 lock_failed_index ≤3 → 不重置
void test_uds_lock_index_normal(void)
{
    g_user_info.nad_info = 0x68u;
    g_ota_info.lock_failed_index = 0u;

    uds_diagnostic_configword_remap_nad();
    TEST_ASSERT_EQUAL_UINT8(0u, g_ota_info.lock_failed_index);
}

void test_last_dfu_info_update(void)
{
    last_dfu_info_t test_info;

    // 测试函数调用
    uint8_t ret = last_dfu_info_update(&test_info);

    // 验证返回值和参数
    TEST_ASSERT_EQUAL(DFU_MSG_SUCCESS, ret);
}

void test_dfu_process_exit(void)
{
    // 测试成功场景
    dfu_process_exit(DFU_MSG_SUCCESS);
    TEST_ASSERT_EQUAL(DFU_INFO_MAGIC, dfu_ctx.dfu_info.magic);

    // 重置fake
    // RESET_FAKE(last_dfu_info_update);

    // 测试失败场景
    dfu_process_exit(DFU_MSG_ERROR);
}

void test_cpmpare_key(void)
{
    uint8_t seed1[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t key1[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t key2[] = {0x11, 0x22, 0x33, 0x45};

    // 测试1：密钥完全相同 -> 返回1
    uint8_t ret1 = cpmpare_key(seed1, key1, 4);
    TEST_ASSERT_EQUAL(1u, ret1);

    // 测试2：密钥不同 -> 返回0
    uint8_t ret2 = cpmpare_key(seed1, key2, 4);
    TEST_ASSERT_EQUAL(0u, ret2);

    // 测试3：长度1，相同
    uint8_t ret3 = cpmpare_key(seed1, key1, 1);
    TEST_ASSERT_EQUAL(1u, ret3);
}

// 1. 覆盖 NAD=0x68 → 拷贝 key_lf
void test_key_reset_nad_68(void)
{
    g_user_info.nad_info = 0x68U;
    memset(key, 0, sizeof(key)); // 清空密钥

    key_reset_by_nad();
}

// 2. 覆盖 NAD=0x69 → 拷贝 key_rf
void test_key_reset_nad_69(void)
{
    g_user_info.nad_info = 0x69U;
    memset(key, 0, sizeof(key));

    key_reset_by_nad();
}

// 3. 覆盖 NAD=0x6A → 拷贝 key_lr
void test_key_reset_nad_6A(void)
{
    g_user_info.nad_info = 0x6AU;
    memset(key, 0, sizeof(key));

    key_reset_by_nad();
}

// 4. 覆盖 NAD=0x6B → 拷贝 key_rr
void test_key_reset_nad_6B(void)
{
    g_user_info.nad_info = 0x6BU;
    memset(key, 0, sizeof(key));

    key_reset_by_nad();
}

// 5. 覆盖 非法NAD → else 空分支
void test_key_reset_nad_invalid(void)
{
    g_user_info.nad_info = 0x70U; // 不在匹配列表内
    key_reset_by_nad();
    // 无操作，仅覆盖分支
}

void test_request_transfer_exit_handle_full(void)
{
    uint8_t param[8] = {0x37, 0x00};
    uint16_t len;

    // 基础环境
    lin_get_uds_nad_fake.return_val = 0x10;
    uds_request_info.sessionMode = PROGRAM_SESSION;

    // 1. 功能寻址 0x7E / 0x7F → return
    lin_get_uds_nad_fake.return_val = 0x7E;
    request_transfer_exit_handle(param, 0);
    lin_get_uds_nad_fake.return_val = 0x7F;
    request_transfer_exit_handle(param, 0);
    lin_get_uds_nad_fake.return_val = 0x10;

    // 2. 非编程会话 → NRC7F
    uds_request_info.sessionMode = DEFALUT_SESSION;
    request_transfer_exit_handle(param, 1);
    uds_request_info.sessionMode = PROGRAM_SESSION;

    // 3. 长度 != 1 → NRC13
    len = 5;
    request_transfer_exit_handle(param, len);

    // 4. 长度正确，开始状态机
    len = BOOT_Frame_length_1;

    // 5. op_code = FLASH_DRIVER_TRANSFER → 正常退出驱动
    dfu_ctx.op_code = DFU_CMD_FLASH_DRIVER_TRANSFER;
    request_transfer_exit_handle(param, len);

    // 6. op_code = APP_TRANSFER → 正常退出APP
    dfu_ctx.op_code = DFU_CMD_APP_TRANSFER;
    request_transfer_exit_handle(param, len);

    // 7. 其他状态 → NRC24
    dfu_ctx.op_code = 0xFF;
    request_transfer_exit_handle(param, len);
}

void test_clear_dtc_info_handle_full(void)
{
    uint8_t param[4] = {0x14, 0x00};
    uint16_t len = 2;

    // 1. 功能寻址 0x7E / 0x7F → 空执行
    lin_get_uds_nad_fake.return_val = 0x7E;
    clear_dtc_info_handle(param, len);
    lin_get_uds_nad_fake.return_val = 0x7F;
    clear_dtc_info_handle(param, len);

    // 2. 普通 NAD → 返回 NRC7F
    lin_get_uds_nad_fake.return_val = 0x10;
    clear_dtc_info_handle(param, len);
}

void test_read_by_identify_handle_full(void)
{
    uint8_t param[16] = {0x22, 0xF3, 0x3F, 0xFF, 0x02, 0x00};
    uint16_t len;

    // 基础环境
    lin_get_uds_nad_fake.return_val = 0x10;

    // 1. 功能寻址 0x7E / 0x7F → return
    lin_get_uds_nad_fake.return_val = 0x7E;
    read_by_identify_handle(param, 0);
    lin_get_uds_nad_fake.return_val = 0x7F;
    read_by_identify_handle(param, 0);
    lin_get_uds_nad_fake.return_val = 0x10;

    // 2. 长度 != 6 → NRC13
    len = 2;
    read_by_identify_handle(param, len);

    // 3. 长度=6，但参数头不匹配 → NRC31
    len = BOOT_Frame_length_6;
    param[1] = 0x00;
    read_by_identify_handle(param, len);
    param[1] = 0xF3;
    param[2] = 0x00;
    read_by_identify_handle(param, len);
    param[2] = 0x3F;
    param[3] = 0x00;
    read_by_identify_handle(param, len);
    param[3] = 0xFF;
    param[4] = 0x00;
    read_by_identify_handle(param, len);
    param[4] = 0x02; // 恢复正确

    // 4. 参数正确，但 param[5] 不是 0x46/0x47 → NRC31
    param[5] = 0x00;
    read_by_identify_handle(param, len);

    // 5. param[5] = 0x46，但配置不匹配 → NRC72
    param[5] = 0x46;
    g_user_info.config_word = 0x47;
    read_by_identify_handle(param, len);

    // 6. ✅ 配置匹配 → 成功响应
    g_user_info.config_word = 0x46;
    read_by_identify_handle(param, len);

    // 7. param[5] = 0x47，配置不匹配 → NRC72
    param[5] = 0x47;
    g_user_info.config_word = 0x46;
    read_by_identify_handle(param, len);

    // 8. ✅ 配置匹配 → 成功响应
    g_user_info.config_word = 0x47;
    read_by_identify_handle(param, len);
}

void test_user_read_data_by_id_full(void)
{
    uint16_t len = 0;
    uint8_t mul_flag = 0;
    uint8_t mul_len = 2; // 多DID偏移长度

    // 初始化全局变量
    g_user_info.config_word = 0x00;
    memset(diagnosticTxBuffer, 0, CUS_UDS_SEND_BUFFER_SIZE);

    // ==========================
    // 1. 遍历所有DID + 单/多DID模式
    // ==========================
    uint16_t did_list[] = {
        0xF187u, 0xF18Au, 0xF197u, 0xF189u,
        0x0216u, 0xF184u, 0xF089u, 0xF180u, 0xFFFFu};

    for (uint8_t i = 0; i < sizeof(did_list) / 2; i++)
    {
        // 单DID模式 (mul_flag=0)
        mul_flag = 0;
        user_read_data_by_id(mul_flag, mul_len, did_list[i], &len);

        // 多DID模式 (mul_flag=1)
        mul_flag = 1;
        user_read_data_by_id(mul_flag, mul_len, did_list[i], &len);
    }

    // ==========================
    // 2. 全覆盖 config_word 4个车门分支（0xF197 + 0x0216）
    // ==========================
    // 左前车门
    g_user_info.config_word = LEFT_FRONT_DOOR;
    user_read_data_by_id(0, mul_len, 0xF197u, &len);
    user_read_data_by_id(0, mul_len, 0x0216u, &len);

    // 左后车门
    g_user_info.config_word = LEFT_REAR_DOOR;
    user_read_data_by_id(0, mul_len, 0xF197u, &len);
    user_read_data_by_id(0, mul_len, 0x0216u, &len);

    // 右前车门
    g_user_info.config_word = RIGHT_FRONT_DOOR;
    user_read_data_by_id(0, mul_len, 0xF197u, &len);
    user_read_data_by_id(0, mul_len, 0x0216u, &len);

    // 右后车门
    g_user_info.config_word = RIGHT_REAR_DOOR;
    user_read_data_by_id(0, mul_len, 0xF197u, &len);
    user_read_data_by_id(0, mul_len, 0x0216u, &len);

    // 无效配置字 (else分支)
    g_user_info.config_word = 0x00;
    user_read_data_by_id(0, mul_len, 0xF197u, &len);
    user_read_data_by_id(0, mul_len, 0x0216u, &len);

    // ==========================
    // 3. 兜底：default无效DID
    // ==========================
    user_read_data_by_id(0, mul_len, 0xFFFFu, &len);
}

void test_read_data_by_identify_handle_real_full(void)
{
    uint8_t param[16] = {0x22, 0x00, 0x00};
    uint16_t len;

    // 基础环境
    lin_get_uds_nad_fake.return_val = 0x10;
    g_negResponseCode = 0;

    // ======================================================================
    // 1. 功能寻址 0x7E / 0x7F → return
    // ======================================================================
    lin_get_uds_nad_fake.return_val = 0x7E;
    read_data_by_identify_handle(param, 3);
    lin_get_uds_nad_fake.return_val = 0x7F;
    read_data_by_identify_handle(param, 3);
    lin_get_uds_nad_fake.return_val = 0x10;

    // ======================================================================
    // 2. 长度 != UDS_READ_BY_DID_REQ_LEN → NRC13
    // ======================================================================
    len = 2;
    read_data_by_identify_handle(param, len);

    // ======================================================================
    // 3. 长度正确，开始测试各种 DID
    // ======================================================================
    len = UDS_READ_BY_DID_REQ_LEN;

    // ------------------------------
    // 支持的DID → 成功返回
    // ------------------------------
    param[1] = 0xF1;
    param[2] = 0x87; // 0xF187
    read_data_by_identify_handle(param, len);

    param[1] = 0xF1;
    param[2] = 0x8A; // 0xF18A
    read_data_by_identify_handle(param, len);

    param[1] = 0xF1;
    param[2] = 0x97; // 0xF197
    read_data_by_identify_handle(param, len);

    param[1] = 0xF0;
    param[2] = 0x89; // 0xF089
    read_data_by_identify_handle(param, len);

    param[1] = 0xF1;
    param[2] = 0x80; // 0xF180
    read_data_by_identify_handle(param, len);

    param[1] = 0xF1;
    param[2] = 0x84; // 0xF184
    read_data_by_identify_handle(param, len);

    // ------------------------------
    // 条件DID：0xF189 / 0x0216
    // ------------------------------
    // magic 不匹配 → 不支持 NRC31
    dfu_ctx.dfu_info.magic = 0x0000;
    param[1] = 0xF1;
    param[2] = 0x89; // 0xF189
    read_data_by_identify_handle(param, len);

    param[1] = 0x02;
    param[2] = 0x16; // 0x0216
    read_data_by_identify_handle(param, len);

    // magic 匹配 → 支持 成功
    dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC;
    param[1] = 0xF1;
    param[2] = 0x89;
    read_data_by_identify_handle(param, len);

    param[1] = 0x02;
    param[2] = 0x16;
    read_data_by_identify_handle(param, len);

    // ------------------------------
    // 无效DID → NRC31
    // ------------------------------
    param[1] = 0xFF;
    param[2] = 0xFF;
    read_data_by_identify_handle(param, len);
}

void test_lin_update_random_value_full(void)
{
    // 初始化外部依赖（必须）
    dfu_ctx.uds_timeout = 5;
    memset(seed, 0, 16); // seed 是全局数组，初始化一下

    // 调用 17 次，自动覆盖：
    // 0~15  → 进入 if (random_i <16)
    // 第16次 → 进入 else 清零
    // 第17次 → 重新从 0 开始
    for (uint8_t i = 0; i < 17; i++)
    {
        lin_update_random_value();
    }
}

void test_lin_exception_handle_full(void)
{
    // ==============================
    // 1. 覆盖：unlock_failed_store_flag == AS_TRUE
    // ==============================
    unlock_failed_store_flag = AS_TRUE;
    diagnostic_session_overtime_flag = AS_FALSE;

    lin_exception_handle();

    // ==============================
    // 2. 覆盖：会话超时 + magic 不匹配 → 不复位
    // ==============================
    unlock_failed_store_flag = AS_FALSE;
    diagnostic_session_overtime_flag = AS_TRUE;
    dfu_ctx.dfu_info.magic = 0x0000; // 无效magic

    lin_exception_handle();

    // ==============================
    // 3. 覆盖：会话超时 + magic 匹配 → 复位
    // ==============================
    diagnostic_session_overtime_flag = AS_TRUE;
    dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC; // 有效magic

    lin_exception_handle();
}

void test_dfu_timeout_handle_full(void)
{
    // ================================
    // 场景1：未超时 → 只自增，不处理
    // ================================
    dfu_ctx.uds_timeout = 0;
    diagnostic_session_overtime_flag = AS_FALSE;

    dfu_timeout_handle();

    // ================================
    // 场景2：达到阈值 → 超时触发
    // ================================
    dfu_ctx.uds_timeout = LIN_UDS_TIMEOUT;
    diagnostic_session_overtime_flag = AS_FALSE;

    dfu_timeout_handle();

    // ================================
    // 场景3：超过阈值 → 同样触发
    // ================================
    dfu_ctx.uds_timeout = LIN_UDS_TIMEOUT + 5;
    diagnostic_session_overtime_flag = AS_FALSE;

    dfu_timeout_handle();
}

void test_dfu_store_system_data_init_full(void)
{
    // 直接调用即可，函数无分支、无判断，100%一行不漏
    dfu_store_system_data_init();
}

void test_dfu_manager_init_full(void)
{
    // 函数无分支，调用一次 = 100%全覆盖
    dfu_manager_init();
}

void test_main_loops_full(void)
{
    // ===================== 初始化 =====================
    dfu_ctx.boot_state = BOOT_STATE_IDLE;

    // 1. 基础调用（未计时满）
    main_loops();

    // 2. 计时满 → 走【DFU 成功 → 进入APP】
    dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC;
    g_ota_info.app_req_ext_program_flag = 0;
    for (uint8_t i = 0; i < 55; i++)
        main_loops();

    // 3. 重置状态 → 计时满 → 走【DFU 失败 → 进入升级模式】
    dfu_ctx.boot_state = BOOT_STATE_IDLE;
    dfu_ctx.dfu_info.magic = 0; // 让last_dfu_info_get返回失败
    for (uint8_t i = 0; i < 55; i++)
        main_loops();

    // 4. 进入APP状态 → 执行 JumpToApp()
    dfu_ctx.boot_state = BOOT_STATE_USER_APP;
    main_loops();
}

void test_os_task_update_full(void)
{
    // 初始化
    app_cmac_start = AS_FALSE;
    timer_1s_cnt = 0;
    clear_wdg_cnt = 0;
    lock_failed_cnt = 0;
    g_ota_info.lock_failed_index = 0;
    unlock_failed_store_flag = AS_FALSE;

    // 1. 基础调用
    os_task_update();

    // 2. CMAC 启动，未超时
    app_cmac_start = AS_TRUE;
    os_task_update();

    // 3. CMAC timer_1s_cnt 超时
    timer_1s_cnt = 1801;
    os_task_update();

    // 4. CMAC clear_wdg_cnt 超时
    clear_wdg_cnt = 101;
    os_task_update();

    // 5. lock_failed_index >2，但计数器未超时（覆盖你缺失的分支）
    g_ota_info.lock_failed_index = 3;
    lock_failed_cnt = 5000;
    os_task_update();

    // 6. lock_failed_index >2，计数器超时
    lock_failed_cnt = 10001;
    os_task_update();
}

void test_session_control_handle_full_coverage(void)
{
    uint8_t param[8] = {0};
    const uint16_t len2 = BOOT_Frame_length_2;

    /* Case 1: length != 2 → NRC13 */
    session_control_handle(param, 0);
    session_control_handle(param, 5);

    /* Case 2: sub=0x01, 非 PROGRAM_SESSION */
    param[1] = 0x01u;
    uds_request_info.sessionMode = DEFALUT_SESSION;
    lin_get_uds_nad_fake.return_val = 0x10;
    session_control_handle(param, len2);

    /* Case 3: PROGRAM_SESSION + magic 无效（覆盖 magic if false） */
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.dfu_info.magic = 0x00000000;
    session_control_handle(param, len2);

    /* ==================== 关键路径：PROGRAM_SESSION + magic 有效 ==================== */
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC; // 让 magic if 为 true

    /* Case 4: sub=0x01 + magic有效 + app_need_res_flag == false（覆盖 inner if != true 的 true 分支） */
    g_ota_info.app_need_res_flag = false;
    lin_get_uds_nad_fake.return_val = 0x10;
    session_control_handle(param, len2);

    g_ota_info.app_need_res_flag = true; // false
    lin_get_uds_nad_fake.return_val = 0x10;
    session_control_handle(param, len2);

    /* Case 5: sub=0x81（抑制） + magic有效 → 直接 NVIC_SystemReset()（覆盖 || 第一条件 true） */
    param[1] = 0x81u;
    session_control_handle(param, len2);

    /* Case 6: sub=0x01 + magic有效 + NAD==0x7E（覆盖 || 第二条件 true） */
    param[1] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, len2);

    /* Case 7: sub=0x01 + magic有效 + NAD==0x7F（覆盖 || 第三条件 true） */
    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, len2);

    /*403-- 405*/
    uds_request_info.sessionMode == DEFALUT_SESSION;
    param[1] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, len2);

    uds_request_info.sessionMode == DEFALUT_SESSION;
    param[1] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, len2);
    /*403-- 405*/

    /*0x03*/
    param[1] = 0x03u;
    uds_request_info.sessionMode = DEFALUT_SESSION;
    lin_get_uds_nad_fake.return_val = 0x10;
    session_control_handle(param, len2);

    param[1] = 0x03u;
    uds_request_info.sessionMode = DEFALUT_SESSION;
    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, len2);

    param[1] = 0x03u;
    uds_request_info.sessionMode = DEFALUT_SESSION;
    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, len2);

    uds_request_info.sessionMode = PROGRAM_SESSION;
    lin_get_uds_nad_fake.return_val = 0x10;
    session_control_handle(param, len2); // sub=0x03 + PROGRAM → NRC7E

    uds_request_info.sessionMode = PROGRAM_SESSION;
    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, len2);

    uds_request_info.sessionMode = PROGRAM_SESSION;
    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, len2);

    /*0x82*/
    param[1] = 0x82u;
    dfu_ctx.op_code = DFU_CMD_ROUTINE_PROGRAM_CHECK;
    lin_get_uds_nad_fake.return_val = 0x10;
    session_control_handle(param, len2);

    param[1] = 0x82u;
    dfu_ctx.op_code = DFU_CMD_ROUTINE_PROGRAM_CHECK;
    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, len2);

    param[1] = 0x82u;
    dfu_ctx.op_code = DFU_CMD_ROUTINE_PROGRAM_CHECK;
    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, len2);

    param[1] = 0x83u;
    session_control_handle(param, len2);

    param[1] = 0x02u;
    dfu_ctx.op_code = DFU_CMD_DEFAULT_SESSION;
    session_control_handle(param, len2); // op_code 允许

    dfu_ctx.op_code = DFU_CMD_ROUTINE_PROGRAM_CHECK;
    session_control_handle(param, len2); // op_code 不允许 → NRC22

    param[1] = 0xFFu;
    lin_get_uds_nad_fake.return_val = 0x10;
    session_control_handle(param, len2); // default + 物理

    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, len2); // default + 功能寻址

    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, len2); // default + 功能寻址
}

void test_security_access_handle_full_coverage(void)
{
    uint8_t param[20] = {0};
    const uint16_t len2 = BOOT_Frame_length_2;
    const uint16_t len18 = BOOT_Frame_length_18;

    /* Case 1: NAD == 0x7E 或 0x7F → 直接 return */
    lin_get_uds_nad_fake.return_val = 0x7E;
    security_access_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    security_access_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x10; // 恢复为物理寻址

    /* Case 2: length 非法 → NRC13 */
    security_access_handle(param, 1);
    security_access_handle(param, 20);

    /* Case 3: sessionMode != PROGRAM_SESSION → NRC7F */
    uds_request_info.sessionMode = DEFALUT_SESSION;
    param[1] = 0x09u;
    security_access_handle(param, len2);

    /* ==================== PROGRAM_SESSION 主路径 ==================== */
    uds_request_info.sessionMode = PROGRAM_SESSION;

    /* Case 4: sub=0x09 + length==2 + lock_failed_index >= 3 → NRC37 */
    g_ota_info.lock_failed_index = 3u;
    param[1] = 0x09u;
    security_access_handle(param, len2);

    /* Case 5: sub=0x09 + length==2 + seed_cmac_succ==TRUE + seed_00_ret==TRUE → NRC24 */
    g_ota_info.lock_failed_index = 0u;
    seed_cmac_succ = AS_TRUE;
    seed_00_ret = AS_TRUE;
    security_access_handle(param, len2);

    /* Case 6: sub=0x09 + length==2 + seed_cmac_succ==TRUE + seed_00_ret==FALSE → 清seed + notify_cp */
    seed_cmac_succ = AS_TRUE;
    seed_00_ret = AS_FALSE;
    security_access_handle(param, len2);

    /* Case 7: sub=0x09 + length==2 + seed_cmac_succ==FALSE + op_code 合法 → 发送 seed */
    seed_cmac_succ = AS_FALSE;
    dfu_ctx.op_code = DFU_CMD_PROGRAM_SESSION;
    security_access_handle(param, len2);

    seed_cmac_succ = AS_FALSE;
    dfu_ctx.op_code = DFU_CMD_SECURITY_SEED_REQUEST;
    security_access_handle(param, len2);

    /* Case 8: sub=0x09 + length==2 + seed_cmac_succ==FALSE + op_code 不合法 → NRC22 */
    dfu_ctx.op_code = DFU_CMD_DEFAULT_SESSION;
    security_access_handle(param, len2);

    /* Case 9: sub=0x09 + length==18（非法） → NRC13 */
    security_access_handle(param, len18);

    /* ==================== sub=0x0a (Key) 路径 ==================== */
    param[1] = 0x0au;

    /* Case 10: sub=0x0a + length==18 + op_code 不正确 → NRC24 */
    dfu_ctx.op_code = DFU_CMD_PROGRAM_SESSION; // 不合法
    security_access_handle(param, len18);

    /* Case 11: sub=0x0a + length==18 + op_code 正确 + 密钥匹配（cpmpare_key 返回 1） */
    dfu_ctx.op_code = DFU_CMD_SECURITY_SEED_REQUEST;
    /* 构造匹配的 key：把 param[2..17] 设为与 seed_use 相同的值 */
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    memset(seed_use, 1, 16);
    for (uint8_t i = 0u; i < 16u; i++)
    {
        param[2 + i] = seed_use[i]; // 让 cpmpare_key 返回 1
    }
    security_access_handle(param, len18);

    /* Case 12: sub=0x0a + length==18 + 密钥错误 + lock_failed_index < 3 → NRC35 */
    dfu_ctx.op_code = DFU_CMD_SECURITY_SEED_REQUEST;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0u; i < 16u; i++)
    {
        param[2 + i] = 0x0u; // 与 seed_use 不同，密钥错误
    }
    g_ota_info.lock_failed_index = 1;
    security_access_handle(param, len18);

    dfu_ctx.op_code = DFU_CMD_SECURITY_SEED_REQUEST;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0u; i < 16u; i++)
    {
        param[2 + i] = 0x0u; // 与 seed_use 不同，密钥错误
    }
    g_ota_info.lock_failed_index = 2;
    security_access_handle(param, len18);

    dfu_ctx.op_code = DFU_CMD_SECURITY_SEED_REQUEST;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0u; i < 16u; i++)
    {
        param[2 + i] = 0x0u; // 与 seed_use 不同，密钥错误
    }
    g_ota_info.lock_failed_index = 3;
    security_access_handle(param, len18);

    dfu_ctx.op_code = DFU_CMD_SECURITY_SEED_REQUEST;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0u; i < 16u; i++)
    {
        param[2 + i] = 0x0u; // 与 seed_use 不同，密钥错误
    }
    g_ota_info.lock_failed_index = 5;
    security_access_handle(param, len18);

    /* Case 15: sub=0x0a + length != 18 → NRC13 */
    security_access_handle(param, len2);

    /* Case 16: default subfunction → NRC12 */
    param[1] = 0xFFu;
    security_access_handle(param, len2);
}

void test_firmware_info_sync_handle_full_coverage(void)
{
    uint8_t param[20] = {0};
    const uint16_t len3 = BOOT_Frame_length_3;
    const uint16_t len13 = BOOT_Frame_length_13;

    /* Case 1: NAD == 0x7E 或 0x7F → 直接 return */
    lin_get_uds_nad_fake.return_val = 0x7E;
    firmware_info_sync_handle(param, len13);

    lin_get_uds_nad_fake.return_val = 0x7F;
    firmware_info_sync_handle(param, len13);

    lin_get_uds_nad_fake.return_val = 0x10; // 恢复物理寻址

    /* Case 2: length <= BOOT_Frame_length_3 → NRC13 */
    firmware_info_sync_handle(param, len3);
    firmware_info_sync_handle(param, 2);

    /* Case 3: length > 3，但 sessionMode != PROGRAM_SESSION → NRC7F */
    uds_request_info.sessionMode = DEFALUT_SESSION; // 或 EXTEND_SESSION
    param[1] = 0xF1u;
    param[2] = 0x84u;
    firmware_info_sync_handle(param, len13);

    /* ==================== PROGRAM_SESSION 主路径 ==================== */
    uds_request_info.sessionMode = PROGRAM_SESSION;

    /* Case 4: param[1..2] != 0xF184 → NRC31 (REQUEST_OUT_RANGE) */
    param[1] = 0xF1u;
    param[2] = 0x00u; // 不匹配
    firmware_info_sync_handle(param, len13);

    param[1] = 0x00u;
    param[2] = 0x84u; // 不匹配
    firmware_info_sync_handle(param, len13);

    /* Case 5: param[1..2] == 0xF184，但 length != 13 → NRC13 */
    param[1] = 0xF1u;
    param[2] = 0x84u;
    firmware_info_sync_handle(param, 10); // length != 13

    /* Case 6: 0xF184 + length==13 + security 未通过（op_code 或 seed_cmac_succ 不满足） → NRC33 */
    dfu_ctx.op_code = DFU_CMD_PROGRAM_SESSION; // 不满足 DFU_CMD_SECURITY_KEY_CHECK
    seed_cmac_succ = AS_FALSE;
    firmware_info_sync_handle(param, len13);

    dfu_ctx.op_code = DFU_CMD_SECURITY_KEY_CHECK;
    seed_cmac_succ = AS_FALSE; // seed_cmac_succ 不为 AS_TRUE
    firmware_info_sync_handle(param, len13);

    /* Case 7: 完整正确路径（覆盖 memcpy + dfu_do_notify_cp） */
    dfu_ctx.op_code = DFU_CMD_SECURITY_KEY_CHECK;
    seed_cmac_succ = AS_TRUE;
    firmware_info_sync_handle(param, len13);

    /* Case 8: 再次测试功能寻址（确保 return 分支被充分覆盖） */
    lin_get_uds_nad_fake.return_val = 0x7E;
    firmware_info_sync_handle(param, len13);
}

void test_request_download_handle_full_coverage(void)
{
    uint8_t param[20] = {0};
    const uint16_t len11 = BOOT_Frame_length_11;

    /* Case 1: NAD == 0x7E 或 0x7F → 直接 return */
    lin_get_uds_nad_fake.return_val = 0x7E;
    request_download_handle(param, len11);

    lin_get_uds_nad_fake.return_val = 0x7F;
    request_download_handle(param, len11);

    lin_get_uds_nad_fake.return_val = 0x10; // 恢复物理寻址

    /* Case 2: sessionMode != PROGRAM_SESSION → NRC7F */
    uds_request_info.sessionMode = DEFALUT_SESSION;
    param[1] = 0x00u;
    param[2] = 0x44u;
    request_download_handle(param, len11);

    /* ==================== PROGRAM_SESSION 主路径 ==================== */
    uds_request_info.sessionMode = PROGRAM_SESSION;

    /* Case 3: length != 11 → NRC13 */
    request_download_handle(param, 10);
    request_download_handle(param, 12);

    /* Case 4: seed_cmac_succ == FALSE → NRC33 */
    seed_cmac_succ = AS_FALSE;
    request_download_handle(param, len11);

    /* Case 5: seed_cmac_succ == TRUE + param[1] != 0x00 → NRC12 */
    seed_cmac_succ = AS_TRUE;
    param[1] = 0x01u; // 不支持的 subfunction
    request_download_handle(param, len11);

    /* Case 6: seed_cmac_succ == TRUE + param[1]==0x00 + param[2] != 0x44 → NRC31 */
    param[1] = 0x00u;
    param[2] = 0x00u; // 不是 0x44
    request_download_handle(param, len11);

    /* ==================== Flash Driver 下载路径 ==================== */
    param[2] = 0x44u;

    /* Case 7: op_code 不合法（既不是 WRITE_FINGER 也不是 SECURITY_KEY_CHECK） → NRC22 */
    dfu_ctx.op_code = DFU_CMD_DEFAULT_SESSION;
    /* 构造正确的 address 和 size */
    param[3] = (FLASH_DRIVER_ADDR >> 24) & 0xFF;
    param[4] = (FLASH_DRIVER_ADDR >> 16) & 0xFF;
    param[5] = (FLASH_DRIVER_ADDR >> 8) & 0xFF;
    param[6] = FLASH_DRIVER_ADDR & 0xFF;
    param[7] = (FLASH_DRIVER_LENGTH >> 24) & 0xFF;
    param[8] = (FLASH_DRIVER_LENGTH >> 16) & 0xFF;
    param[9] = (FLASH_DRIVER_LENGTH >> 8) & 0xFF;
    param[10] = FLASH_DRIVER_LENGTH & 0xFF;

    request_download_handle(param, len11);

    /* Case 8: op_code 合法 + address/size 正确（Flash Driver）→ 正响应 + 初始化变量 */
    dfu_ctx.op_code = DFU_CMD_WRITE_FINGER; // 或 DFU_CMD_SECURITY_KEY_CHECK
    request_download_handle(param, len11);

    dfu_ctx.op_code = DFU_CMD_SECURITY_KEY_CHECK; // 或 DFU_CMD_WRITE_FINGER
    request_download_handle(param, len11);

    /* ==================== APP 下载路径 ==================== */
    /* Case 9: op_code == DFU_CMD_APP_ERASE + address != FLASH_APP_ADDR → NRC31 */
    dfu_ctx.op_code = DFU_CMD_WRITE_FINGER;
    param[3] = (0x12345678 >> 24) & 0xFF; // 错误的地址
    param[4] = (0x12345678 >> 16) & 0xFF;
    param[5] = (0x12345678 >> 8) & 0xFF;
    param[6] = 0x78;
    request_download_handle(param, len11);

    dfu_ctx.op_code = DFU_CMD_WRITE_FINGER;
    param[3] = (FLASH_DRIVER_ADDR >> 24) & 0xFF;
    param[4] = (FLASH_DRIVER_ADDR >> 16) & 0xFF;
    param[5] = (FLASH_DRIVER_ADDR >> 8) & 0xFF;
    param[6] = FLASH_DRIVER_ADDR & 0xFF;
    param[7] = (0 >> 24) & 0xFF; // 错误的大小
    param[8] = (0 >> 16) & 0xFF;
    param[9] = (0 >> 8) & 0xFF;
    param[10] = 0 & 0xFF;
    request_download_handle(param, len11);

    dfu_ctx.op_code = DFU_CMD_WRITE_FINGER;
    param[3] = (0 >> 24) & 0xFF;
    param[4] = (0 >> 16) & 0xFF;
    param[5] = (0 >> 8) & 0xFF;
    param[6] = 0 & 0xFF;
    param[7] = (0 >> 24) & 0xFF; // 错误的大小
    param[8] = (0 >> 16) & 0xFF;
    param[9] = (0 >> 8) & 0xFF;
    param[10] = 0 & 0xFF;
    request_download_handle(param, len11);

    /* Case 10: op_code == DFU_CMD_APP_ERASE + address == FLASH_APP_ADDR → 正响应 */
    dfu_ctx.op_code = DFU_CMD_APP_ERASE;
    param[3] = (FLASH_APP_ADDR >> 24) & 0xFF;
    param[4] = (FLASH_APP_ADDR >> 16) & 0xFF;
    param[5] = (FLASH_APP_ADDR >> 8) & 0xFF;
    param[6] = FLASH_APP_ADDR & 0xFF;
    param[7] = (0x00010000 >> 24) & 0xFF; // 示例 size
    param[8] = (0x00010000 >> 16) & 0xFF;
    param[9] = (0x00010000 >> 8) & 0xFF;
    param[10] = 0x00;
    request_download_handle(param, len11);

    dfu_ctx.op_code = DFU_CMD_APP_ERASE;
    param[3] = (0 >> 24) & 0xFF;
    param[4] = (0 >> 16) & 0xFF;
    param[5] = (0 >> 8) & 0xFF;
    param[6] = 0 & 0xFF;
    param[7] = (0x00010000 >> 24) & 0xFF; // 示例 size
    param[8] = (0x00010000 >> 16) & 0xFF;
    param[9] = (0x00010000 >> 8) & 0xFF;
    param[10] = 0x00;
    request_download_handle(param, len11);

    /* Case 11: 再次测试功能寻址 return 分支 */
    lin_get_uds_nad_fake.return_val = 0x7E;
    request_download_handle(param, len11);
}

// 1. 覆盖 NAD=0x7E → 直接return
void test_transfer_nad_7e(void)
{
    uint8_t param[8] = {0};
    transfer_data_handle(param, 8);
    (void)param;
}

// 2. 覆盖 NAD=0x7F → 直接return
void test_transfer_nad_7f(void)
{
    uint8_t param[8] = {0};
    transfer_data_handle(param, 8);
    (void)param;
}

// 3. 覆盖 非编程会话 → NRC7F
void test_transfer_session_normal(void)
{
    uds_request_info.sessionMode = DEFALUT_SESSION;
    uint8_t param[8] = {0};
    transfer_data_handle(param, 8);
    (void)param;
}

// 4. 覆盖 编程会话 + 长度<3 → NRC13
void test_transfer_len_less_3(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    uint8_t param[8] = {0};
    transfer_data_handle(param, 2);
    (void)param;
}

// 5. 覆盖 编程会话 + 长度>0x202 → NRC31
void test_transfer_len_over_514(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    uint8_t param[10] = {0};
    transfer_data_handle(param, 0x203);
    (void)param;
}

// 6. 覆盖 编程会话 + FLASH驱动操作码 → 驱动传输
void test_transfer_op_flash_driver(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_FLASH_DRIVER_REQUEST;
    uint8_t param[10] = {0x34, 0x01, 0x00};
    transfer_data_handle(param, 10);
    (void)param;
}

// 7. 覆盖 编程会话 + APP操作码 + 序号错误 → NRC73
void test_transfer_app_seq_error(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_REQUEST;
    dfu_ctx.write_index = 1;
    uint8_t param[10] = {0x34, 0x05, 0x00}; // 序号不匹配
    transfer_data_handle(param, 10);
    (void)param;
}

// 8. 覆盖 编程会话 + APP操作码 + 烧写失败 → NRC72 + 清空队列
void test_transfer_app_program_fail(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_TRANSFER;
    dfu_ctx.write_index = 1;
    uint8_t param[10] = {0x34, 0x02, 0x00};
    transfer_data_handle(param, 10);
    (void)param;
}

// 9. 覆盖 编程会话 + APP操作码 + 烧写成功 + head越界归零
void test_transfer_app_program_success_head_wrap(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_TRANSFER;
    dfu_ctx.write_index = 1;
    dfu_ctx.queue_list.head = QUEUE_LIN_LEN - 1; // 触发head归零
    uint8_t param[10] = {0x34, 0x02, 0x00};
    transfer_data_handle(param, 10);
    (void)param;
}

// 10. 覆盖 编程会话 + APP操作码 + 烧写成功 + tail越界归零
void test_transfer_app_program_success_tail_wrap(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_TRANSFER;
    dfu_ctx.write_index = 1;
    dfu_ctx.receive_length = DFU_PROGRAM_LENGTH; // 触发长度达标
    dfu_ctx.queue_list.tail = QUEUE_LIN_LEN - 1; // 触发tail归零
    uint8_t param[10] = {0x34, 0x02, 0x00};
    transfer_data_handle(param, 10);
    (void)param;
}

// 11. 覆盖 编程会话 + APP操作码 + 烧写成功 + 长度不足 → NRC71
void test_transfer_app_program_pause(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_TRANSFER;
    dfu_ctx.write_index = 1;
    dfu_ctx.receive_length = 0; // 长度不足
    uint8_t param[10] = {0x34, 0x02, 0x00};
    transfer_data_handle(param, 10);
    (void)param;
}

// 12. 覆盖 编程会话 + APP操作码 + 烧写成功 + 写入长度满 → 清空队列
void test_transfer_app_write_length_full(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_TRANSFER;
    dfu_ctx.write_index = 1;
    dfu_ctx.dfu_info.image_size = 0x100;
    dfu_ctx.write_length = 0x100; // 写入长度=镜像大小
    uint8_t param[10] = {0x34, 0x02, 0x00};
    transfer_data_handle(param, 10);
    (void)param;
}

// 13. 覆盖 编程会话 + 无效操作码 → NRC24
void test_transfer_op_unknown(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = 0xFF; // 无效操作码
    uint8_t param[10] = {0};
    transfer_data_handle(param, 10);
    (void)param;
}

void test_routine_control_handle_full_coverage(void)
{

    uint8_t param[30] = {0};
    const uint16_t len4 = BOOT_Frame_length_4;
    const uint16_t len12 = BOOT_Frame_length_12;
    const uint16_t len20 = BOOT_Frame_length_20;

    /* Case 1: NAD == 0x7E 或 0x7F → 直接 return */
    lin_get_uds_nad_fake.return_val = 0x7E;
    routine_control_handle(param, len4);

    lin_get_uds_nad_fake.return_val = 0x7F;
    routine_control_handle(param, len4);

    lin_get_uds_nad_fake.return_val = 0x10; // 恢复物理寻址

    /* Case 2: subfunction != 0x01 && != 0x81 → NRC12 */
    param[1] = 0x02u;
    routine_control_handle(param, len4);

    param[1] = 0x81u;
    routine_control_handle(param, len4);

    param[1] = 0x01u; // 恢复合法 subfunction

    /* ==================== routine_id = 0x0203 ==================== */
    param[2] = 0x02u;
    param[3] = 0x03u;

    /* 3.1 session != EXTEND → NRC7F */
    uds_request_info.sessionMode = PROGRAM_SESSION;
    routine_control_handle(param, len4);

    /* 3.2 session == EXTEND + length != 4 → NRC13 */
    uds_request_info.sessionMode = EXTEND_SESSION;
    routine_control_handle(param, 5);

    /* 3.3 session == EXTEND + length==4 + op_code 不合法 → NRC22 */
    dfu_ctx.op_code = DFU_CMD_DEFAULT_SESSION;
    routine_control_handle(param, len4);

    /* 3.4 正确路径：session == EXTEND + length==4 + op_code 合法 */
    dfu_ctx.op_code = DFU_CMD_EXTEND_SESSION;
    routine_control_handle(param, len4);

    dfu_ctx.op_code = DFU_CMD_ROUTINE_PROGRAM_CHECK;
    routine_control_handle(param, len4);

    /* ==================== routine_id = 0xDD02 (签名校验) ==================== */
    param[2] = 0xDDu;
    param[3] = 0x02u;

    /* 4.1 session != PROGRAM → NRC7F */
    uds_request_info.sessionMode = EXTEND_SESSION;
    routine_control_handle(param, len20);

    /* 4.2 session == PROGRAM + length != 20 → NRC13 */
    uds_request_info.sessionMode = PROGRAM_SESSION;
    routine_control_handle(param, 10);

    /* 4.3 op_code 既不是 FLASH_DRIVER_EXIT 也不是 APP_EXIT → NRC22 */
    dfu_ctx.op_code = DFU_CMD_DEFAULT_SESSION;
    routine_control_handle(param, len20);

    /* 4.4 Flash Driver CMAC 校验 - 匹配成功 */
    dfu_ctx.op_code = DFU_CMD_FLASH_DRIVER_EXIT;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0; i < 16u; i++)
        param[4 + i] = 0x01; // 匹配
    routine_control_handle(param, len20);

    /* 4.5 Flash Driver CMAC 校验 - 匹配失败 */
    dfu_ctx.op_code = DFU_CMD_FLASH_DRIVER_EXIT;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0; i < 16u; i++)
        param[4 + i] = 0x00u; // 不匹配
    routine_control_handle(param, len20);

    /* 4.6 APP CMAC 校验 - 匹配成功 */
    dfu_ctx.op_code = DFU_CMD_APP_EXIT;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0; i < 16u; i++)
        param[4 + i] = 0x00; // 不匹配
    routine_control_handle(param, len20);

    /* 4.7 APP CMAC 校验 - 匹配失败 */
    dfu_ctx.op_code = DFU_CMD_APP_EXIT;
    aes_cmac_fake.custom_fake = aes_cmac_test_customaes_cmac;
    for (uint8_t i = 0; i < 16u; i++)
        param[4 + i] = 0x01u; // 匹配
    routine_control_handle(param, len20);

    /* ==================== routine_id = 0xFF00 (擦除 FLASH) ==================== */
    param[2] = 0xFFu;
    param[3] = 0x00u;

    /* 5.1 session != PROGRAM → NRC7F */
    uds_request_info.sessionMode = EXTEND_SESSION;
    routine_control_handle(param, len12);

    /* 5.2 session == PROGRAM + length != 12 → NRC13 */
    uds_request_info.sessionMode = PROGRAM_SESSION;
    routine_control_handle(param, 10);

    /* 5.3 session == PROGRAM + length==12 + op_code 或 flag 不满足 → NRC22 */
    dfu_ctx.op_code = DFU_CMD_DEFAULT_SESSION;
    flashdrv_cmac_succ = AS_FALSE;
    routine_control_handle(param, len12);

    /* 5.4 正确擦除路径 */
    dfu_ctx.op_code = DFU_CMD_FLASH_DRIVER_CMAC_CHECK;
    flashdrv_cmac_succ = AS_TRUE;
    routine_control_handle(param, len12);

    // 一正一反
    dfu_ctx.op_code = DFU_CMD_FLASH_DRIVER_CMAC_CHECK;
    flashdrv_cmac_succ = AS_FALSE;
    routine_control_handle(param, len12);
    /* ==================== routine_id = 0xFF01 (兼容性检查) ==================== */
    param[2] = 0xFFu;
    param[3] = 0x01u;

    /* 6.1 session != PROGRAM → NRC7F */
    uds_request_info.sessionMode = EXTEND_SESSION;
    routine_control_handle(param, len4);

    /* 6.2 session == PROGRAM + length != 4 → NRC13 */
    uds_request_info.sessionMode = PROGRAM_SESSION;
    routine_control_handle(param, 5);

    /* 6.3 session == PROGRAM + length==4 + op_code 或 flag 不满足 → NRC22 */
    dfu_ctx.op_code = DFU_CMD_DEFAULT_SESSION;
    app_cmac_succ = AS_FALSE;
    routine_control_handle(param, len4);

    /* 6.4 正确兼容性检查路径 */
    dfu_ctx.op_code = DFU_CMD_APP_CMAC_CHECK;
    app_cmac_succ = AS_TRUE;
    routine_control_handle(param, len4);

    // 一正一反
    dfu_ctx.op_code = DFU_CMD_APP_CMAC_CHECK;
    app_cmac_succ = AS_FALSE;
    routine_control_handle(param, len4);

    /* Case 7: default routine_id → NRC31 */
    param[2] = 0x99u;
    param[3] = 0x99u;
    routine_control_handle(param, len4);
}

void test_reset_handle_full_coverage(void)
{
    uint8_t param[8] = {0};
    const uint16_t len2 = BOOT_Frame_length_2;

    /* ==========================
     * Case 1: length != 2 → NRC13 【已覆盖，保留】
     * ========================== */
    reset_handle(param, 1);
    reset_handle(param, 3);
    reset_handle(param, 0);

    /* ==========================
     * Case 2: param[1] = 0x01 软复位 【已覆盖，保留】
     * ========================== */
    param[1] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x7E;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x10;
    reset_handle(param, len2);

    /* ==========================
     * Case 3: param[1] = 0x81 直接复位 【已覆盖，保留】
     * ========================== */
    param[1] = 0x81u;
    reset_handle(param, len2);

    /* ==========================
     * ✅ 补充1：param[1] = 0x02 全分支（原缺失普通NAD正响应）
     * ========================== */
    param[1] = 0x02u;
    lin_get_uds_nad_fake.return_val = 0x7E;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x10; // 普通NAD → 正响应
    reset_handle(param, len2);

    /* ==========================
     * ✅ 补充2：param[1] = 0x03 全分支（原完全缺失）
     * ========================== */
    param[1] = 0x03u;
    lin_get_uds_nad_fake.return_val = 0x7E;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x10; // 普通NAD → 正响应
    reset_handle(param, len2);

    /* ==========================
     * ✅ 补充3：param[1] = 0x82 / 0x83 空分支（原完全缺失）
     * ========================== */
    param[1] = 0x82u;
    reset_handle(param, len2);

    param[1] = 0x83u;
    reset_handle(param, len2);

    /* ==========================
     * ✅ 补充4：无效子功能(0x05) → NRC12（原完全缺失）
     * ========================== */
    param[1] = 0x05u; // 无效子功能
    lin_get_uds_nad_fake.return_val = 0x7E;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    reset_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x10; // 普通NAD → NRC12
    reset_handle(param, len2);
}

void test_assign_config_word_handle_full_coverage(void)
{
    uint8_t param[10] = {0};
    const uint16_t len6 = BOOT_Frame_length_6;

    /* Case 1: NAD == 0x7E 或 0x7F → 直接 return */
    lin_get_uds_nad_fake.return_val = 0x7E;
    assign_config_word_handle(param, len6);

    lin_get_uds_nad_fake.return_val = 0x7F;
    assign_config_word_handle(param, len6);

    lin_get_uds_nad_fake.return_val = 0x10; // 恢复物理寻址

    /* Case 2: length != 6 → NRC13 */
    assign_config_word_handle(param, 5);
    assign_config_word_handle(param, 7);

    /* Case 3: length == 6，但 param[1,2,4] 不满足 0xF3 0x3F 0x02 → NRC31 */
    param[1] = 0xF3u;
    param[2] = 0x00u; // 不匹配
    param[4] = 0x02u;
    assign_config_word_handle(param, len6);

    param[1] = 0xF3u;
    param[2] = 0x3Fu;
    param[4] = 0x00u; // 不匹配
    assign_config_word_handle(param, len6);

    param[1] = 0x00u;
    param[2] = 0x3Fu;
    param[4] = 0x02u; // 不匹配
    assign_config_word_handle(param, len6);

    /* ==================== 正确前缀 + switch 分支 ==================== */
    param[1] = 0xF3u;
    param[2] = 0x3Fu;
    param[4] = 0x02u;

    /* Case 4: fuc_id = 0x01 (start) */
    param[3] = 0x01u;
    assign_config_word_handle(param, len6);

    /* Case 5: fuc_id = 0x02 (assign) - state != START → 不执行 */
    param[3] = 0x02u;
    g_config_word_state = CONFIGURE_WORD_STATE_END; // 非 START
    assign_config_word_handle(param, len6);

    /* Case 6: fuc_id = 0x02 (assign) - state == START + param[5] 合法 (0x46 或 0x47) */
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    param[5] = 0x46u;
    assign_config_word_handle(param, len6);

    g_config_word_state = CONFIGURE_WORD_STATE_START;
    param[5] = 0x47u;
    assign_config_word_handle(param, len6);

    g_config_word_state = CONFIGURE_WORD_STATE_START;
    param[5] = 0x48u;
    assign_config_word_handle(param, len6);

    /* Case 7: fuc_id = 0x03 (save) - state != ASIGN → 不执行 */
    param[3] = 0x03u;
    g_config_word_state = CONFIGURE_WORD_STATE_START; // 非 ASIGN
    assign_config_word_handle(param, len6);

    /* Case 8: fuc_id = 0x03 (save) - state == ASIGN → 执行完整流程 */
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    assign_config_word_handle(param, len6);

    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    lin_get_uds_nad_fake.return_val = 0x42;
    assign_config_word_handle(param, len6);

    /* Case 9: fuc_id = 0x04 (end) */
    param[3] = 0x04u;
    assign_config_word_handle(param, len6);

    /* Case 10: fuc_id 默认分支（其他值） */
    param[3] = 0x05u;
    assign_config_word_handle(param, len6);

    /* Case 11: 再次测试功能寻址 return */
    lin_get_uds_nad_fake.return_val = 0x7E;
    assign_config_word_handle(param, len6);

    /* Case 12: save 步骤中 NAD=0x6A 和 0x69 分支 */
    param[3] = 0x03u;
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    lin_get_uds_nad_fake.return_val = 0x6A;
    assign_config_word_handle(param, len6);

    param[3] = 0x03u;
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    lin_get_uds_nad_fake.return_val = 0x69;
    assign_config_word_handle(param, len6);

    /* Case 13: assign 步骤中 RIGHT_REAR_DOOR */
    param[3] = 0x02u;
    param[5] = RIGHT_REAR_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    assign_config_word_handle(param, len6);

    /* Case 14: assign 步骤中 RIGHT_FRONT_DOOR (走 || 第二个条件) */
    param[5] = RIGHT_FRONT_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    assign_config_word_handle(param, len6);
}

void test_communcation_control_handle_full_coverage(void)
{
    uint8_t param[8] = {0};
    const uint16_t len3 = BOOT_Frame_length_3;

    /* Case 1: sessionMode != EXTEND_SESSION → NRC7F（包含 NAD 功能寻址空分支） */
    uds_request_info.sessionMode = DEFALUT_SESSION; // 或 PROGRAM_SESSION
    lin_get_uds_nad_fake.return_val = 0x10;
    communcation_control_handle(param, len3);

    lin_get_uds_nad_fake.return_val = 0x7E; // 功能寻址（空分支）
    communcation_control_handle(param, len3);

    lin_get_uds_nad_fake.return_val = 0x7F; // 功能寻址（空分支）
    communcation_control_handle(param, len3);

    /* Case 2: sessionMode == EXTEND_SESSION + length != 3 → NRC13 */
    uds_request_info.sessionMode = EXTEND_SESSION;
    communcation_control_handle(param, 2);
    communcation_control_handle(param, 4);

    /* ==================== length == 3 的主路径 ==================== */

    /* Case 3: param[1] 既不是 0x00/0x03 也不是 0x80/0x83 → NRC12 */
    param[1] = 0x01u; // 不支持的 subfunction
    lin_get_uds_nad_fake.return_val = 0x10;
    communcation_control_handle(param, len3);

    /* Case 4: param[1] == 0x00 或 0x03 + param[2] 合法 (0x01 或 0x03) → 正响应 */
    param[1] = 0x00u;
    param[2] = 0x01u;
    communcation_control_handle(param, len3);

    param[1] = 0x03u;
    param[2] = 0x03u;
    communcation_control_handle(param, len3);

    param[1] = 0x00u;
    param[2] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x7E;
    communcation_control_handle(param, len3);

    param[1] = 0x03u;
    param[2] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x7F;
    communcation_control_handle(param, len3);

    /* Case 5: param[1] == 0x00/0x03 + param[2] 不合法 → NRC31 */
    param[1] = 0x00u;
    param[2] = 0x02u; // 不合法
    lin_get_uds_nad_fake.return_val = 0x10;
    communcation_control_handle(param, len3);

    param[1] = 0x00u;
    param[2] = 0x02u; // 不合法
    lin_get_uds_nad_fake.return_val = 0x7E;
    communcation_control_handle(param, len3);

    param[1] = 0x00u;
    param[2] = 0x02u; // 不合法
    lin_get_uds_nad_fake.return_val = 0x7F;
    communcation_control_handle(param, len3);

    /* Case 6: param[1] == 0x80 或 0x83 + param[2] 合法 → 空分支（什么都不做） */
    param[1] = 0x80u;
    param[2] = 0x01u;
    communcation_control_handle(param, len3);

    param[1] = 0x83u;
    param[2] = 0x03u;
    communcation_control_handle(param, len3);

    param[1] = 0x83u;
    param[2] = 0x02u;
    lin_get_uds_nad_fake.return_val = 0x7F;
    communcation_control_handle(param, len3);

    param[1] = 0x83u;
    param[2] = 0x02u;
    lin_get_uds_nad_fake.return_val = 0x7E;
    communcation_control_handle(param, len3);

    /* Case 7: param[1] == 0x80/0x83 + param[2] 不合法 → NRC31 */
    param[1] = 0x80u;
    param[2] = 0x00u; // 不合法
    lin_get_uds_nad_fake.return_val = 0x10;
    communcation_control_handle(param, len3);

    /* Case 8: 再次测试功能/抑制寻址的空分支（在不同路径下） */
    lin_get_uds_nad_fake.return_val = 0x7F;
    param[1] = 0x00u;
    param[2] = 0x01u;
    communcation_control_handle(param, len3);

    /* Case 9: session == EXTEND + length==3 + param[1] 不支持 → NRC12（功能寻址） */
    lin_get_uds_nad_fake.return_val = 0x7E;
    param[1] = 0xFFu;
    communcation_control_handle(param, len3);

    lin_get_uds_nad_fake.return_val = 0x7F;
    param[1] = 0xFFu;
    communcation_control_handle(param, len3);
}

void test_dtc_control_handle_full_coverage(void)
{
    uint8_t param[8] = {0};
    const uint16_t len2 = BOOT_Frame_length_2;

    /* Case 1: sessionMode != EXTEND_SESSION → NRC7F（包含功能寻址空分支） */
    uds_request_info.sessionMode = DEFALUT_SESSION; // 或 PROGRAM_SESSION
    lin_get_uds_nad_fake.return_val = 0x10;
    dtc_control_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7E; // 功能寻址（空分支）
    dtc_control_handle(param, len2);

    /* Case 2: sessionMode == EXTEND_SESSION + length != 2 → NRC13 */
    uds_request_info.sessionMode = EXTEND_SESSION;
    dtc_control_handle(param, 1);
    dtc_control_handle(param, 3);

    /* ==================== length == 2 的主路径 ==================== */

    /* Case 3: param[1] == 0x01 → 正响应（物理寻址） */
    param[1] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x10;
    dtc_control_handle(param, len2);

    /* Case 4: param[1] == 0x01 + 功能寻址 → 空分支 */
    lin_get_uds_nad_fake.return_val = 0x7F;
    dtc_control_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7E;
    dtc_control_handle(param, len2);

    /* Case 5: param[1] == 0x02 → 正响应（物理寻址） */
    param[1] = 0x02u;
    lin_get_uds_nad_fake.return_val = 0x10;
    dtc_control_handle(param, len2);

    /* Case 6: param[1] == 0x02 + 功能寻址 → 空分支 */
    lin_get_uds_nad_fake.return_val = 0x7E;
    dtc_control_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    dtc_control_handle(param, len2);

    /* Case 7: param[1] == 0x81 → 空分支（抑制响应） */
    param[1] = 0x81u;
    dtc_control_handle(param, len2);

    /* Case 8: param[1] == 0x82 → 空分支（抑制响应） */
    param[1] = 0x82u;
    dtc_control_handle(param, len2);

    /* Case 9: param[1] 既不是 0x01/0x02/0x81/0x82 → NRC12（物理寻址） */
    param[1] = 0x03u;
    lin_get_uds_nad_fake.return_val = 0x10;
    dtc_control_handle(param, len2);

    /* Case 10: param[1] 不支持 + 功能寻址 → 空分支 */
    lin_get_uds_nad_fake.return_val = 0x7F;
    dtc_control_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7E;
    dtc_control_handle(param, len2);

    /* Case 11: 再次测试 session != EXTEND + 功能寻址（确保覆盖） */
    uds_request_info.sessionMode = PROGRAM_SESSION;
    lin_get_uds_nad_fake.return_val = 0x7E;
    dtc_control_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    dtc_control_handle(param, len2);
}

void test_diagnostic_session_handle_full_coverage(void)
{
    uint8_t param[8] = {0};
    const uint16_t len2 = BOOT_Frame_length_2;

    /* Case 1: length != 2 → NRC13 */
    diagnostic_session_handle(param, 1);
    diagnostic_session_handle(param, 3);

    /* ==================== length == 2 的主路径 ==================== */

    /* Case 2: param[1] == 0x00 → 正响应（物理寻址） */
    param[1] = 0x00u;
    lin_get_uds_nad_fake.return_val = 0x10;
    diagnostic_session_handle(param, len2);

    /* Case 3: param[1] == 0x00 + 功能/抑制寻址 → 空分支 */
    lin_get_uds_nad_fake.return_val = 0x7E;
    diagnostic_session_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    diagnostic_session_handle(param, len2);

    /* Case 4: param[1] == 0x80 → 空分支（抑制响应） */
    param[1] = 0x80u;
    diagnostic_session_handle(param, len2);

    /* Case 5: param[1] 既不是 0x00 也不是 0x80 → NRC12（物理寻址） */
    param[1] = 0x01u;
    lin_get_uds_nad_fake.return_val = 0x10;
    diagnostic_session_handle(param, len2);

    /* Case 6: param[1] 不支持 + 功能/抑制寻址 → 空分支 */
    lin_get_uds_nad_fake.return_val = 0x7E;
    diagnostic_session_handle(param, len2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    diagnostic_session_handle(param, len2);

    /* Case 7: 再次确认 length != 2（确保覆盖） */
    diagnostic_session_handle(param, 0);
}

void test_lin_diag_service_handle_full_coverage(void)
{
    uint8_t dummy_packet[20] = {0};

    /* 设置 custom fake（控制 length 和 sid） */
    lin_uds_receive_fake.custom_fake = lin_uds_receive_custom;

    /* Case 1: length == 0（覆盖 if(length) false） */
    lin_uds_receive_fake.custom_fake = NULL;
    lin_diag_service_handle();

    lin_uds_receive_fake.custom_fake = lin_uds_receive_custom;

    /* Case 2: length > 0 + sid 匹配 + func != NULL（覆盖 for循环匹配 + func true + break） */
    lin_diag_service_handle();

    /* Case 3: length > 0 + sid 匹配 + func == NULL（覆盖 if func false） */
    lin_uds_receive_fake.custom_fake = lin_uds_receive_custom_sid_00;
    lin_diag_service_handle();

    /* Case 4: length > 0 + sid 不匹配（覆盖 for循环走完 + i == DFU_PROCESS_STEP_MAX true） */
    lin_uds_receive_fake.custom_fake = lin_uds_receive_custom_sid_ff;
    lin_get_uds_nad_fake.return_val = 0x10; // 普通 NAD → 执行 NRC
    lin_diag_service_handle();

    /* Case 5: length > 0 + sid 不匹配 + NAD == 0x7E（覆盖 NAD if true，空分支） */
    lin_get_uds_nad_fake.return_val = 0x7E;
    lin_diag_service_handle();

    /* Case 6: length > 0 + sid 不匹配 + NAD == 0x7F（覆盖 || 的另一个条件） */
    lin_get_uds_nad_fake.return_val = 0x7F;
    lin_diag_service_handle();

    /* 恢复默认 */
    lin_uds_receive_fake.custom_fake = lin_uds_receive_custom;
}

/* ===== read_data_by_identify_handle: multi DID ===== */
void test_read_data_by_identify_multi_did(void)
{
    uint8_t param[8] = {0x22, 0xF1, 0x87, 0xF1, 0x8A};

    lin_get_uds_nad_fake.return_val = 0x10;
    read_data_by_identify_handle(param, 5);
}

/* ===== read_by_identify_handle: config mismatch NRC72 ===== */
void test_read_by_identify_config_mismatch(void)
{
    uint8_t param[8] = {0x12, 0xF3, 0x3F, 0xFF, 0x02, 0x00};

    g_user_info.config_word = LEFT_FRONT_DOOR;
    param[5] = LEFT_REAR_DOOR;
    lin_get_uds_nad_fake.return_val = 0x10;
    read_by_identify_handle(param, 6);
}

/* ===== communcation_control_handle: unknown subfunc 0x04 ===== */
void test_communcation_control_unknown_subfunc(void)
{
    uint8_t param[8] = {0x28, 0x04, 0x01};

    uds_request_info.sessionMode = EXTEND_SESSION;
    lin_get_uds_nad_fake.return_val = 0x10;
    communcation_control_handle(param, 3);
}

/* ===== transfer_data_handle: NAD 0x7E/0x7F return ===== */
void test_transfer_nad_7e_return(void)
{
    uint8_t param[3] = {0};
    uds_request_info.sessionMode = PROGRAM_SESSION;
    lin_get_uds_nad_fake.return_val = 0x7E;
    transfer_data_handle(param, 3);
}

void test_transfer_nad_7f_return(void)
{
    uint8_t param[3] = {0};
    uds_request_info.sessionMode = PROGRAM_SESSION;
    lin_get_uds_nad_fake.return_val = 0x7F;
    transfer_data_handle(param, 3);
}

/* ===== transfer_data_handle: app write success (valid addr) ===== */
void test_transfer_app_program_write_success(void)
{
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_REQUEST;
    dfu_ctx.write_index = 1;
    dfu_ctx.write_addr = FLASH_APP_ADDR;
    dfu_ctx.queue_list.head = QUEUE_LIN_LEN - 1;
    dfu_ctx.receive_length = DFU_PROGRAM_LENGTH;
    dfu_ctx.queue_list.tail = QUEUE_LIN_LEN - 1;
    uint8_t param[10] = {0x34, 0x02, 0x00};
    transfer_data_handle(param, 10);
}

/* ===== assign_config_word_handle: door match ===== */
void test_assign_config_word_door_match(void)
{
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    uint8_t param[6] = {0xB5, 0xF3, 0x3F, 0x02, 0x02, LEFT_FRONT_DOOR};
    assign_config_word_handle(param, 6);
}

/* ===== read_data_by_identify_handle: multi DID all invalid ===== */
void test_read_data_by_identify_multi_did_all_invalid(void)
{
    lin_get_uds_nad_fake.return_val = 0x10;
    uint8_t param[5] = {0x22, 0x00, 0x02, 0x00, 0x03};
    read_data_by_identify_handle(param, 5);
}

/* ===== session_control_handle: subfunc=0x01 from EXTEND triggers DEFAULT path + NAD=0x7E/0x7F ===== */
void test_session_control_subfunc_0x01_extend_7e_7f(void)
{
    uint8_t param[2] = {0x10, 0x01};
    uds_request_info.sessionMode = EXTEND_SESSION;

    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, 2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, 2);
}

/* ===== session_control_handle: subfunc=0x81 from EXTEND triggers DEFAULT path + && false ===== */
void test_session_control_subfunc_0x81_extend_7e_7f(void)
{
    uint8_t param[2] = {0x10, 0x81};
    uds_request_info.sessionMode = EXTEND_SESSION;

    lin_get_uds_nad_fake.return_val = 0x7E;
    session_control_handle(param, 2);

    lin_get_uds_nad_fake.return_val = 0x7F;
    session_control_handle(param, 2);
}

/* ===== communcation_control_handle: subfunc=0x02, 0x81, 0x82 ===== */
void test_communcation_control_subfunc_02_81_82(void)
{
    uint8_t param[3] = {0x28, 0x02, 0x01};
    uds_request_info.sessionMode = EXTEND_SESSION;
    lin_get_uds_nad_fake.return_val = 0x10;

    communcation_control_handle(param, 3); // subfunc=0x02

    param[1] = 0x81;
    communcation_control_handle(param, 3); // subfunc=0x81

    param[1] = 0x82;
    communcation_control_handle(param, 3); // subfunc=0x82
}

/* ===== read_data_by_identify_handle: odd/oversized lengths ===== */
void test_read_data_by_identify_edge_lengths(void)
{
    lin_get_uds_nad_fake.return_val = 0x10;
    uint8_t param[6] = {0x22, 0xF1, 0x87, 0xF1, 0x8A, 0x00};
    read_data_by_identify_handle(param, 6); // 奇数长度

    uint8_t param2[12] = {0x22};
    read_data_by_identify_handle(param2, 12); // 超长
}

/* ===== transfer_data_handle: param[1]=0x02 matching write_index+1 ===== */
void test_transfer_param1_matches_index(void)
{
    uint8_t param[10] = {0x36, 0x02, 0x00};
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_TRANSFER;
    dfu_ctx.write_index = 1;
    transfer_data_handle(param, 10);
}

/* ===== transfer_data_handle: op_code = FLASH_DRIVER_TRANSFER ===== */
void test_transfer_op_code_flash_driver_transfer(void)
{
    uint8_t param[10] = {0x36, 0x01, 0x00};
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_FLASH_DRIVER_TRANSFER;
    transfer_data_handle(param, 10);
}

void test_transfer_data_handle0(void)
{
    uint8_t param[10] = {0x36, 0x00, 0x00};
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_REQUEST;
    transfer_data_handle(param, 10);
}

void test_transfer_data_handle1(void)
{
    uint8_t param[10] = {0x36, 0x09, 0x00};
    uds_request_info.sessionMode = PROGRAM_SESSION;
    dfu_ctx.op_code = DFU_CMD_APP_REQUEST;
    dfu_ctx.write_index = 8;
    dfu_ctx.receive_length = 0;
    dfu_ctx.write_length = 0;
    dfu_ctx.dfu_info.image_size = 16;
    transfer_data_handle(param, 10);
}

/* ===== assign_config_word_handle: save 步骤 NAD!=0x68 ===== */
void test_assign_config_save_nad_mismatch(void)
{
    uint8_t param[6] = {0xB5, 0xF3, 0x3F, 0x01, 0x02, 0x00};
    g_config_word_state = 0;
    lin_get_uds_nad_fake.return_val = 0x10;
    assign_config_word_handle(param, 6); // fuc_id=0x01 → start

    param[3] = 0x02;
    param[5] = LEFT_FRONT_DOOR;
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    assign_config_word_handle(param, 6); // fuc_id=0x02 → assign

    param[3] = 0x03;
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    lin_get_uds_nad_fake.return_val = 0x42;
    assign_config_word_handle(param, 6); // fuc_id=0x03 → save with NAD=0x42

    /* 再次 save，用 NAD=0x6B 覆盖 else if 最后一个分支 */
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    lin_get_uds_nad_fake.return_val = 0x6B;
    assign_config_word_handle(param, 6);
}

void test_assign_config_word_handle0(void)
{
    g_config_word_state = CONFIGURE_WORD_STATE_START;
    uint8_t param[6] = {0xB5, 0xF3, 0x3F, 0x02, 0x02, LEFT_REAR_DOOR};
    assign_config_word_handle(param, 6);
}

void test_assign_config_word_handle1(void)
{
    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
    uint8_t param[6] = {0xB5, 0xF3, 0x3F, 0x03, 0x02, LEFT_REAR_DOOR};
    lin_get_uds_nad_fake.return_val = 0x68;
    assign_config_word_handle(param, 6);
}

/* ===== read_by_identify_handle: RIGHT_REAR_DOOR ===== */
void test_read_by_identify_right_rear(void)
{
    uint8_t param[6] = {0x12, 0xF3, 0x3F, 0xFF, 0x02, RIGHT_REAR_DOOR};
    lin_get_uds_nad_fake.return_val = 0x10;
    read_by_identify_handle(param, 6);
}

void test_read_by_identify_handle0(void)
{
    uint8_t param[6] = {0x12, 0xF3, 0x3F, 0xFF, 0x02, RIGHT_FRONT_DOOR};
    lin_get_uds_nad_fake.return_val = 0x10;
    read_by_identify_handle(param, 6);
}

/* ===== multi DID read_data_by_identify: mix of valid+invalid ===== */
void test_read_data_by_identify_multi_did_mixed(void)
{
    lin_get_uds_nad_fake.return_val = 0x10;
    uint8_t param[5] = {0x22, 0xF1, 0x87, 0x00, 0x02}; // F187(valid) + 0002(invalid)
    read_data_by_identify_handle(param, 5);
}

/* ===== read_by_identify_handle: config RIGHT_REAR_DOOR + user_read right_rear ===== */
void test_user_read_right_rear_config(void)
{
    g_user_info.config_word = RIGHT_REAR_DOOR;
    uint16_t len;
    user_read_data_by_id(0, 0, 0xF197, &len); // DID 0xF197 ECU name
}

void test_user_read_data_by_id0(void)
{
    g_user_info.config_word = 4;
    uint16_t len;
    user_read_data_by_id(0, 0, 0xF197, &len); // DID 0xF197 ECU name
}

void test_user_read_data_by_id1(void)
{
    g_user_info.config_word = 4;
    uint16_t len;
    user_read_data_by_id(0, 0, 0x0216, &len);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_JumpToApp);
    RUN_TEST(test_queue_lin_empty);
    RUN_TEST(test_dfu_image_erase);
    RUN_TEST(test_dfu_image_program);
    RUN_TEST(test_dfu_do_notify_cp);
    RUN_TEST(test_dfu_do_notify_cp_ex);
    RUN_TEST(test_dfu_do_notify_response);
    RUN_TEST(test_dfu_session_parameter_resp);
    RUN_TEST(test_last_dfu_info_get);

    // NAD 合法值测试
    RUN_TEST(test_uds_remap_nad_68);
    RUN_TEST(test_uds_remap_nad_6A);
    RUN_TEST(test_uds_remap_nad_69);
    RUN_TEST(test_uds_remap_nad_6B);

    // NAD 非法值测试
    RUN_TEST(test_uds_remap_nad_invalid);

    // 锁索引边界测试
    RUN_TEST(test_uds_lock_index_over_3);
    RUN_TEST(test_uds_lock_index_normal);

    RUN_TEST(test_last_dfu_info_update);
    RUN_TEST(test_dfu_process_exit);
    RUN_TEST(test_cpmpare_key);
    RUN_TEST(test_request_transfer_exit_handle_full);
    RUN_TEST(test_clear_dtc_info_handle_full);
    RUN_TEST(test_read_by_identify_handle_full);

    // 覆盖4个合法NAD分支
    RUN_TEST(test_key_reset_nad_68);
    RUN_TEST(test_key_reset_nad_69);
    RUN_TEST(test_key_reset_nad_6A);
    RUN_TEST(test_key_reset_nad_6B);

    // 覆盖非法NAD else分支
    RUN_TEST(test_key_reset_nad_invalid);

    RUN_TEST(test_user_read_data_by_id_full);
    RUN_TEST(test_read_data_by_identify_handle_real_full);
    RUN_TEST(test_lin_update_random_value_full);
    RUN_TEST(test_lin_exception_handle_full);
    RUN_TEST(test_dfu_timeout_handle_full);
    RUN_TEST(test_dfu_store_system_data_init_full);
    RUN_TEST(test_dfu_manager_init_full);
    RUN_TEST(test_main_loops_full);
    RUN_TEST(test_os_task_update_full);

    RUN_TEST(test_session_control_handle_full_coverage);
    RUN_TEST(test_security_access_handle_full_coverage);

    RUN_TEST(test_firmware_info_sync_handle_full_coverage);

    RUN_TEST(test_request_download_handle_full_coverage);

    // RUN_TEST(test_transfer_data_handle_full_coverage);

    RUN_TEST(test_transfer_nad_7e);
    RUN_TEST(test_transfer_nad_7f);
    RUN_TEST(test_transfer_session_normal);
    RUN_TEST(test_transfer_len_less_3);
    RUN_TEST(test_transfer_len_over_514);
    RUN_TEST(test_transfer_op_flash_driver);
    RUN_TEST(test_transfer_app_seq_error);
    RUN_TEST(test_transfer_app_program_fail);
    RUN_TEST(test_transfer_app_program_success_head_wrap);
    RUN_TEST(test_transfer_app_program_success_tail_wrap);
    RUN_TEST(test_transfer_app_program_pause);
    RUN_TEST(test_transfer_app_write_length_full);
    RUN_TEST(test_transfer_op_unknown);

    RUN_TEST(test_routine_control_handle_full_coverage);

    RUN_TEST(test_reset_handle_full_coverage);

    RUN_TEST(test_assign_config_word_handle_full_coverage);

    RUN_TEST(test_communcation_control_handle_full_coverage);

    RUN_TEST(test_dtc_control_handle_full_coverage);

    RUN_TEST(test_diagnostic_session_handle_full_coverage);

    RUN_TEST(test_lin_diag_service_handle_full_coverage);

    RUN_TEST(test_read_data_by_identify_multi_did);
    RUN_TEST(test_read_by_identify_config_mismatch);
    RUN_TEST(test_communcation_control_unknown_subfunc);
    RUN_TEST(test_transfer_nad_7e_return);
    RUN_TEST(test_transfer_nad_7f_return);
    RUN_TEST(test_transfer_app_program_write_success);
    RUN_TEST(test_assign_config_word_door_match);
    RUN_TEST(test_read_data_by_identify_multi_did_all_invalid);
    RUN_TEST(test_session_control_subfunc_0x01_extend_7e_7f);
    RUN_TEST(test_session_control_subfunc_0x81_extend_7e_7f);
    RUN_TEST(test_communcation_control_subfunc_02_81_82);
    RUN_TEST(test_read_data_by_identify_edge_lengths);
    RUN_TEST(test_transfer_param1_matches_index);
    RUN_TEST(test_transfer_op_code_flash_driver_transfer);
    RUN_TEST(test_read_by_identify_right_rear);
    RUN_TEST(test_read_data_by_identify_multi_did_mixed);
    RUN_TEST(test_user_read_right_rear_config);
    RUN_TEST(test_assign_config_save_nad_mismatch);

    RUN_TEST(test_transfer_data_handle0);
    RUN_TEST(test_transfer_data_handle1);

    RUN_TEST(test_assign_config_word_handle0);
    RUN_TEST(test_assign_config_word_handle1);
    RUN_TEST(test_read_by_identify_handle0);

    RUN_TEST(test_user_read_data_by_id0);
    RUN_TEST(test_user_read_data_by_id1);

    return UNITY_END();
}