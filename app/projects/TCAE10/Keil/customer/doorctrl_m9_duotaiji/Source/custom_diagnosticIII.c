/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 ******************************************************************************
 * @brief  application main file.
 *
 * @file   custom_diagnosticlll.c
 * @author AE/FAE team
 * @date
 ******************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
 *
 ******************************************************************************
 */
#ifdef ENABLE_TEST_MODE
#include "fff_custom_diagnosticIII.h"
#include "fff_diagnosticIII.h"
#include "fff_tc_log.h"
#include "fff_lin.h"
#include "fff_pal_store.h"
#include "fff_store_manager.h"
#include "fff_utilities.h"
#include "fff_touch_include.h"
#include "fff_lin_frame.h"
#include "fff_touch_config.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "custom_diagnosticIII.h"
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "tc_log.h"
#include "lin.h"
#include "pal_store.h"
#include "store_manager.h"
#include "utilities.h"
#include "touch_include.h"
#include "lin_frame.h"
#include "touch_config.h"
#endif

static const char *TAG = "custom";

#if defined APP_MATCH_BOOT
/* PRQA S 1514 4 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/* PRQA S 3408 3 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 2071 2 #3269 - Language extension used for compiler and hardware optimization */
__attribute__((section(".ARM.__at_0x00004000"))) const char g_lin_sequence_num_version[24] = "SW:1.01.B_260525_3197_06";                                                                                                              /* 24 byte*/
__attribute__((section(".ARM.__at_0x00004018"))) const char g_seres_app_software_version[21] = {'S', 'W', ':', 'E', 'H', 'I', 'S', 'F', 'L', '.', '1', '.', '3', 'C', '.', '0', '5', (char)0x20, (char)0x20, (char)0x20, (char)0x20}; /* 21 byte*/
#else
const char g_lin_sequence_num_version[24] = "SW:1.01.B_260525_3197_06";                                                                                      /* 24 byte*/
const char g_seres_app_software_version[21] = {'S', 'W', ':', 'E', 'H', 'I', 'S', 'F', 'L', '.', '1', '.', '3', 'C', '.', '0', '5', 0x20, 0x20, 0x20, 0x20}; /* 21 byte*/
#endif

/* general defines for UDS Session*/
#define UDS_TRUE 0x01u
#define UDS_FALSE 0x00u

#define P2_SERVER_MAX (500)
#define P2E_SERVER_MAX (200)

#define FLASH_SECTOR_SIZE (512)
#define FLASH_BASE_ADDR (0x00000000U)
/* PRQA S 1534 ++ #3261 - Unused macro defined for future extension and configuration compatibility */
#define FLASH_END_ADDR (0x00010000UL)
#define CUS_CFG_WORD_BASE_ADDR (0x0000F800U)                    /* 0.5k */
#define CUS_SYSTEM_PARAM_BASE_ADDR (0x0000FA00U)                /* 0.5k */
#define FLASH_BOOT_SIZE (0x00003E00UL)                          /* 15.5k */
#define FLASH_DFU_INFO_ADDR (FLASH_BASE_ADDR + FLASH_BOOT_SIZE) /* 15.5k */
#define FLASH_DFU_INFO_SIZE (FLASH_SECTOR_SIZE)                 /* 0.5k */

#define FLASH_APP_PARAM_SIZE (0x00000800U)                                            /* 2K*/
#define FLASH_APP_IMAGE_SIZE (FLASH_END_ADDR - FLASH_APP_ADDR - FLASH_APP_PARAM_SIZE) /* 46k */
#define FLASH_APP_END_ADDR (FLASH_END_ADDR - FLASH_APP_PARAM_SIZE)
#define FLASH_BOOT_VERSION_ADDR (0x00003900u)
#define FLASH_HW_VERSION_ADDR (0x00003908u)

#define CUS_UDS_RECEIVE_BUFFER_SIZE (20)
#define CUS_UDS_SEND_BUFFER_SIZE (66)
#define UDS_SESSION_POSIT_RESP_LEN 3u
#define UDS_SESSION_NEG_RESP_LEN 3u
#define UDS_SUPPRESS_POS_RESP_INDIC_BIT 0x80u
#define UDS_SID_MASK_WO_RESP_IND_BIT 0x7Fu
#define UDS_NEG_RESP_RSID 0x7Fu
#define UDS_POS_RESP_CODE 0x40u
#define UDS_DIAG_SESSION_POSIT_RESP_LEN 6u
#define UDS_DIAG_SESSION_REQ_LEN 2u
#define UDS_DIAG_ROUTE_POSIT_RESP_LEN 5u
#define UDS_DIAG_ROUTE_REQ_LEN 4u
#define UDS_READ_BY_DID_MIN_RESP_LEN 3u
#define UDS_READ_BY_DID_REQ_LEN 3u

#define CUS_UDS_PRODUCT_IDENT 0xF3u

/* negative response code for UDS services */
#define UDS_SERVICE_NOT_SUPPORTED_11 0x11u /* This response code is in general supported by each diagnostic service */
#define UDS_SUBFUNC_NOT_SUPP_12 0x12u
#define UDS_INCOR_LEN_INVALID_FORMAT_13 0x13u
#define UDS_RESPONSE_TOO_LONG_14 0x14u
#define UDS_BUSY_REPEAT_REQUEST_21 0x21u
#define UDS_COND_NOT_CORRECT_22 0x22u
#define UDS_REQU_SEQU_ERROR_24 0x24u
#define UDS_REQUEST_OUT_OF_RANGE_31 0x31u
#define UDS_DID_SEC_ERR_33 0x33u
#define UDS_GENERAL_PROGRAMMING_FAIL_72 0x72u
#define UDS_REQ_CORR_REC_RESP_PEND_78 0x78u
#define UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F 0x7Fu
#define UDS_COND_NOT_SUPPORT_7E 0x7Eu

#define CONFIGURE_WORD_STATE_INIT 0
#define CONFIGURE_WORD_STATE_START 1
#define CONFIGURE_WORD_STATE_ASIGN 2
#define CONFIGURE_WORD_STATE_SAVE 3
#define CONFIGURE_WORD_STATE_END 4
/* PRQA S 1534 -- */
#define MULT_DID_MAX 5

/* PRQA S 1514 ++ #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/* PRQA S 3408 ++ #3218 - External linkage function defined without prior declaration, intentional design */
lin_touch_data touch_data;

/**
 * Note:ota dfu info struct, this should be align with bootloader
 */
typedef struct
{
    uint8_t fingerprint[10];
    uint32_t magic;
    uint32_t image_size;
} last_dfu_info_t;

typedef struct
{
    uint8_t did_num;
    uint16_t data_len;
    uint16_t did_valid_flag;
    uint16_t did_array[MULT_DID_MAX];
} mult_did_data_t;

/**
 * @brief ota sys info
 */
typedef struct
{
    uint8_t app_req_ext_program_flag;
    uint8_t app_need_res_flag; /* boot to app flag */
    uint8_t lock_failed_index; /* 27 sid lock failed index */
    /* PRQA S 1504 16 #3220 -  Object used only in local translation unit, intentional design */
    /* PRQA S 2071 15 #3269 - Language extension used for compiler and hardware optimization */
} ota_cfg_t __attribute__((aligned(1)));

uint8_t diagnosticRxBuffer[CUS_UDS_RECEIVE_BUFFER_SIZE] = {0};
uint8_t diagnosticTxBuffer[CUS_UDS_SEND_BUFFER_SIZE] = {0};
uint8_t diagRxSize = 0;
uint8_t negResponseCode = 0;
last_dfu_info_t g_dfu_info = {0};
user_cfg_t g_user_info = {0};
ota_cfg_t g_ota_info = {0};
uint8_t g_config_word_state = 0;
uint8_t session_mode = SESSION_MODE_DEFAULT;
uint8_t program_condition_check = 0;
uint16_t lock_failed_cnt = 0;
uint8_t unlock_failed_store_flag = 0;
uint32_t diagnostic_session_cnt = 0;
/* PRQA S 1514 -- */
/* PRQA S 3408 -- */

STATIC void guserinfo_save(void);
STATIC void gsysinfo_save(void);
STATIC void enable_swd(void); // Enable SWD interface

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void send_negative_response_message(uint8_t negrespcode)
{
    uint16_t msglen = UDS_SESSION_NEG_RESP_LEN;
    diagnosticTxBuffer[0u] = UDS_NEG_RESP_RSID;
    diagnosticTxBuffer[1u] = diagnosticRxBuffer[0u];
    diagnosticTxBuffer[2u] = negrespcode;
    if (ld_tx_status() == (uint8_t)LD_COMPLETED)
    {
        ld_send_message(msglen, diagnosticTxBuffer);
    }
    else if (ld_tx_status() == (uint8_t)LD_N_AS_TIMEOUT)
    {
        ld_send_message(msglen, diagnosticTxBuffer);
    }
    else
    {
        (void)0;
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void send_positive_response_message(uint16_t msglen)
{
    diagnosticTxBuffer[0u] = diagnosticRxBuffer[0u] + UDS_POS_RESP_CODE;
    if (ld_tx_status() == (uint8_t)LD_COMPLETED)
    {
        ld_send_message(msglen, diagnosticTxBuffer);
    }
    else if (ld_tx_status() == (uint8_t)LD_N_AS_TIMEOUT)
    {
        ld_send_message(msglen, diagnosticTxBuffer);
    }
    else
    {
        (void)0;
    }
}

STATIC uint8_t uds_diag_DID_chk(uint16_t ucSess)
{
    uint8_t ucRet;
    switch (ucSess)
    {
    /* Seres part num:6106150-RQ01 */
    case 0xF187:
    /* Seres Supplier code:3233 */
    case 0xF18A:
    /* Seres ECU name:EHIS_FL */
    case 0xF197:
    case 0xF189:
    case 0xF089:
    case 0xF180:
    case 0xF184:
    case 0x0216:
    case 0xF190:
    case 0xF0FA:
    case 0x0001:
        ucRet = UDS_TRUE;
        break;
    default:
        ucRet = UDS_FALSE;
        break;
    }
    return ucRet;
}

STATIC void user_read_data_by_id(uint8_t mul_flag, uint8_t mul_len, uint16_t did, uint16_t *len)
{
    uint8_t loc;
    static const char *const seres_part_numbers[] = {"4280310-RW02"};
    const uint8_t seres_supplier_code[10] = {0x33, 0x31, 0x39, 0x37, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
    uint8_t seres_ecu_name[10] = {(uint8_t)'E', (uint8_t)'H', (uint8_t)'I', (uint8_t)'S', (uint8_t)'_', (uint8_t)'F', (uint8_t)'L', 0x20, 0x20, 0x20};
    uint8_t boot_version[8] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
    uint8_t hard_version[8] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};

    switch (did)
    {
    /* Seres part num:6106150-RQ01 */
    case 0xF187:
        *len = 12;
        for (loc = 0u; loc < 12u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(seres_part_numbers[0][loc]); /*mult did*/
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(seres_part_numbers[0][loc]); /*single did*/
            }
        }
        break;
    /* Seres Supplier code:3233 */
    case 0xF18A:
        *len = 10;
        for (loc = 0u; loc < 10u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(seres_supplier_code[loc]); /*mult did*/
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(seres_supplier_code[loc]); /*single did*/
            }
        }
        break;
    /* Seres ECU name:EHIS_FL */
    case 0xF197:
        *len = 10;
        if (g_user_info.config_word == (uint8_t)LEFT_FRONT_DOOR) // Left front door handle
        {
            seres_ecu_name[5] = (uint8_t)'F';
            seres_ecu_name[6] = (uint8_t)'L';
        }
        else if (g_user_info.config_word == (uint8_t)LEFT_REAR_DOOR) // Left rear door handle
        {
            seres_ecu_name[5] = (uint8_t)'R';
            seres_ecu_name[6] = (uint8_t)'L';
        }
        else if (g_user_info.config_word == (uint8_t)RIGHT_FRONT_DOOR) // Right front door handle
        {
            seres_ecu_name[5] = (uint8_t)'F';
            seres_ecu_name[6] = (uint8_t)'R';
        }
        else if (g_user_info.config_word == (uint8_t)RIGHT_REAR_DOOR) // Right rear door handle
        {
            seres_ecu_name[5] = (uint8_t)'R';
            seres_ecu_name[6] = (uint8_t)'R';
        }
        else
        {
            (void)0;
        }
        for (loc = 0u; loc < 10u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(seres_ecu_name[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(seres_ecu_name[loc]);
            }
        }
        break;
    /* Seres LIN slave sequence num:SW:1.01.A_250606_3233_00 */
    case 0xF189:
        *len = 24;
        for (loc = 0u; loc < 24u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(g_lin_sequence_num_version[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(g_lin_sequence_num_version[loc]);
            }
        }
        break;
    /* Seres software version*/
    case 0x0216:
        *len = 21;

        if (mul_flag != 0u)
        {
            /* copy seres_software_version[] */
            for (loc = 0u; loc < 21u; loc++)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(g_seres_app_software_version[loc]);
            }
            if (g_user_info.config_word == (uint8_t)LEFT_FRONT_DOOR) // Left front door handle
            {
                diagnosticTxBuffer[mul_len + 7u] = (uint8_t)'F';
                diagnosticTxBuffer[mul_len + 8u] = (uint8_t)'L';
            }
            else if (g_user_info.config_word == (uint8_t)LEFT_REAR_DOOR) // Left rear door handle
            {
                diagnosticTxBuffer[mul_len + 7u] = (uint8_t)'R';
                diagnosticTxBuffer[mul_len + 8u] = (uint8_t)'L';
            }
            else if (g_user_info.config_word == (uint8_t)RIGHT_FRONT_DOOR) // Right front door handle
            {
                diagnosticTxBuffer[mul_len + 7u] = (uint8_t)'F';
                diagnosticTxBuffer[mul_len + 8u] = (uint8_t)'R';
            }
            else if (g_user_info.config_word == (uint8_t)RIGHT_REAR_DOOR) // Right rear door handle
            {
                diagnosticTxBuffer[mul_len + 7u] = (uint8_t)'R';
                diagnosticTxBuffer[mul_len + 8u] = (uint8_t)'R';
            }
            else
            {
                (void)0;
            }
        }
        else
        {
            /* copy seres_software_version[] */
            for (loc = 0u; loc < 21u; loc++)
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(g_seres_app_software_version[loc]);
            }
            if (g_user_info.config_word == (uint8_t)LEFT_FRONT_DOOR) // Left front door handle
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 7u] = (uint8_t)'F';
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 8u] = (uint8_t)'L';
            }
            else if (g_user_info.config_word == (uint8_t)LEFT_REAR_DOOR) // Left rear door handle
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 7u] = (uint8_t)'R';
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 8u] = (uint8_t)'L';
            }
            else if (g_user_info.config_word == (uint8_t)RIGHT_FRONT_DOOR) // Right front door handle
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 7u] = (uint8_t)'F';
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 8u] = (uint8_t)'R';
            }
            else if (g_user_info.config_word == (uint8_t)RIGHT_REAR_DOOR) // Right rear door handle
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 7u] = (uint8_t)'R';
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + 8u] = (uint8_t)'R';
            }
            else
            {
                (void)0;
            }
        }

        break;
    /* Seres fingerprint info*/
    case 0xF184:
        *len = 10;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        store_slow_smart_read(FLASH_DFU_INFO_ADDR, (uint8_t *)&g_dfu_info, sizeof(last_dfu_info_t));
        for (loc = 0u; loc < 10u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(g_dfu_info.fingerprint[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(g_dfu_info.fingerprint[loc]);
            }
        }
        break;
    /* hardware verison info */
    case 0xF089:
        *len = 8;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        ll_flash_read(FLASH_TYPE_NVM, FLASH_HW_VERSION_ADDR, (uint8_t *)hard_version, sizeof(hard_version));
        for (loc = 0u; loc < 8u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(hard_version[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(hard_version[loc]);
            }
        }
        break;
    /* bootloader verison info */
    case 0xF180:
        *len = 8;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        ll_flash_read(FLASH_TYPE_NVM, FLASH_BOOT_VERSION_ADDR, (uint8_t *)boot_version, sizeof(boot_version));
        for (loc = 0u; loc < 8u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(boot_version[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(boot_version[loc]);
            }
        }
        break;
    case 0xF190:
        *len = 0;
        break;
    case 0xF0FA:
        *len = 0;
        break;

    /*get raw base diff noise key value*/
    case 0x0001:
        *len = 27;
        diagnosticTxBuffer[1] = (uint8_t)((uint16_t)touch_data.key1_raw[0] >> 8);
        diagnosticTxBuffer[2] = (uint8_t)touch_data.key1_raw[0];
        diagnosticTxBuffer[3] = (uint8_t)((uint16_t)touch_data.key1_base[0] >> 8);
        diagnosticTxBuffer[4] = (uint8_t)touch_data.key1_base[0];
        diagnosticTxBuffer[5] = (uint8_t)((uint16_t)touch_data.key1_diff[0] >> 8);
        diagnosticTxBuffer[6] = (uint8_t)touch_data.key1_diff[0];

        diagnosticTxBuffer[7] = (uint8_t)((uint16_t)touch_data.key1_raw[1] >> 8);
        diagnosticTxBuffer[8] = (uint8_t)touch_data.key1_raw[1];
        diagnosticTxBuffer[9] = (uint8_t)((uint16_t)touch_data.key1_base[1] >> 8);
        diagnosticTxBuffer[10] = (uint8_t)touch_data.key1_base[1];
        diagnosticTxBuffer[11] = (uint8_t)((uint16_t)touch_data.key1_diff[1] >> 8);
        diagnosticTxBuffer[12] = (uint8_t)touch_data.key1_diff[1];

        diagnosticTxBuffer[13] = (uint8_t)((uint16_t)touch_data.key2_raw[0] >> 8);
        diagnosticTxBuffer[14] = (uint8_t)touch_data.key2_raw[0];
        diagnosticTxBuffer[15] = (uint8_t)((uint16_t)touch_data.key2_base[0] >> 8);
        diagnosticTxBuffer[16] = (uint8_t)touch_data.key2_base[0];
        diagnosticTxBuffer[17] = (uint8_t)((uint16_t)touch_data.key2_diff[0] >> 8);
        diagnosticTxBuffer[18] = (uint8_t)touch_data.key2_diff[0];

        diagnosticTxBuffer[19] = (uint8_t)((uint16_t)touch_data.key2_raw[1] >> 8);
        diagnosticTxBuffer[20] = (uint8_t)touch_data.key2_raw[1];
        diagnosticTxBuffer[21] = (uint8_t)((uint16_t)touch_data.key2_base[1] >> 8);
        diagnosticTxBuffer[22] = (uint8_t)touch_data.key2_base[1];
        diagnosticTxBuffer[23] = (uint8_t)((uint16_t)touch_data.key2_diff[1] >> 8);
        diagnosticTxBuffer[24] = (uint8_t)touch_data.key2_diff[1];

        diagnosticTxBuffer[25] = (uint8_t)((uint16_t)touch_data.noise_raw >> 8);
        diagnosticTxBuffer[26] = (uint8_t)touch_data.noise_raw;

        diagnosticTxBuffer[27] = (uint8_t)(touch_data.key_val & 0x3u);
        break;

    default:
        (void)0;
        break;
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_session_control(void)
{
    uint8_t ucSess = 0u;
    uint8_t usMsgLen = 0u;

    usMsgLen = diagRxSize;
    if (UDS_DIAG_SESSION_REQ_LEN == usMsgLen)
    {
        ucSess = diagnosticRxBuffer[1u];
        switch (ucSess)
        {
        case 0x01:
        case 0x81:
            session_mode = SESSION_MODE_DEFAULT;
            break;
        case 0x02:
        case 0x82:
            if (program_condition_check == 1) // 满足预编程条件
            {
                program_condition_check = 0;

                /* program req_ext_program_flag && reset*/
                if ((lin_current_rcvd_nad() == 0x7Eu) || (lin_current_rcvd_nad() == 0x7Fu))
                    g_ota_info.app_req_ext_program_flag = 3;
                else
                    g_ota_info.app_req_ext_program_flag = 1;
                gsysinfo_save();
                delay1ms(1);
                ll_wdg_enable(false);
                NVIC_SystemReset();
            }
            else // 不满足预编程条件（31）或之前是默认会话模式
            {
                if ((lin_current_rcvd_nad() == 0x7Eu) || (lin_current_rcvd_nad() == 0x7Fu))
                {
                }
                else
                {
                    if (ucSess == 0x02)
                    {
                        if (session_mode == SESSION_MODE_DEFAULT)
                        {
                            send_negative_response_message(UDS_COND_NOT_SUPPORT_7E); // NRC7E
                        }
                        else
                        {
                            send_negative_response_message(UDS_COND_NOT_CORRECT_22); // NRC22
                        }
                    }
                }
            }
            break;

        case 0x03:
        case 0x83:
            diagnostic_session_cnt = TcTimeGet();
            session_mode = SESSION_MODE_EXTEND;
            break;
        default:
            if (lin_current_rcvd_nad() == 0x7Eu)
            {
            }
            else if (lin_current_rcvd_nad() == 0x7Fu)
            {
            }
            else
            {
                send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12); // NRC12
            }

            break;
        }
        if ((ucSess == 0x01u) || (ucSess == 0x03u))
        {
            if (lin_current_rcvd_nad() == 0x7Eu)
            {
            }
            else if (lin_current_rcvd_nad() == 0x7Fu)
            {
            }
            else
            {
                diagnosticTxBuffer[1] = ucSess;
                diagnosticTxBuffer[2] = (uint8_t)((uint16_t)P2_SERVER_MAX >> 8);
                diagnosticTxBuffer[3] = (uint8_t)P2_SERVER_MAX;
                diagnosticTxBuffer[4] = (uint8_t)((uint16_t)P2E_SERVER_MAX >> 8);
                diagnosticTxBuffer[5] = (uint8_t)P2E_SERVER_MAX;
                usMsgLen = 6;
                send_positive_response_message(usMsgLen);
            }
        }
    }
    else
    {
        send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13
    }
}

/* PRQA S 2889 3 #3257 - Multiple return statements for logical clarity and efficiency */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_route_control(void)
{
    uint16_t ucSess = 0u;
    uint8_t usMsgLen = 0u;
    negResponseCode = 0;

    usMsgLen = diagRxSize;
    if (lin_current_rcvd_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_current_rcvd_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if (UDS_DIAG_ROUTE_REQ_LEN == usMsgLen)
    {
        if ((diagnosticRxBuffer[1] == 0x01u) || (diagnosticRxBuffer[1] == 0x81u))
        {
            ucSess = (((uint16_t)diagnosticRxBuffer[2] << 8) | diagnosticRxBuffer[3]);
            diagnosticRxBuffer[1] = 0x01;
            if (ucSess == 0x0203u)
            {
                if (session_mode == (uint8_t)SESSION_MODE_EXTEND)
                {

                    if (((door_cmd.VehicleSpeedValid == 1u) && (door_cmd.VehicleSpeed < 0x36u)) || (door_cmd.VehicleSpeedValid == 0u))
                    {
                        program_condition_check = 1;
                    }
                    else
                    {
                        send_negative_response_message(UDS_COND_NOT_CORRECT_22); // NRC22
                    }
                }
                else
                {
                    negResponseCode = UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F;
                }
            }
            else if ((ucSess == 0xFF01u) || (ucSess == 0xFF00u) || (ucSess == 0xDD02u))
            {
                negResponseCode = UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F;
            }
            else
            {
                negResponseCode = UDS_REQUEST_OUT_OF_RANGE_31;
            }
        }
        else
        {
            negResponseCode = UDS_SUBFUNC_NOT_SUPP_12;
        }
    }
    else
    {
        negResponseCode = UDS_INCOR_LEN_INVALID_FORMAT_13;
    }

    if (0u == negResponseCode) /* positive response */
    {
        usMsgLen = UDS_DIAG_ROUTE_POSIT_RESP_LEN;
        diagnosticTxBuffer[1u] = diagnosticRxBuffer[1];
        diagnosticTxBuffer[2u] = diagnosticRxBuffer[2];
        diagnosticTxBuffer[3u] = diagnosticRxBuffer[3];
        diagnosticTxBuffer[4u] = 0x00;
        send_positive_response_message(usMsgLen);
    }
    if (negResponseCode != 0u) /* negative response */
    {
        send_negative_response_message(negResponseCode);
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_dtc_control(void)
{
    if (session_mode == (uint8_t)SESSION_MODE_EXTEND)
    {
        if (diagRxSize == 2u)
        {
            if ((diagnosticRxBuffer[1] == 0x81u) || (diagnosticRxBuffer[1] == 0x82u)) // Prohibit affirmative response
            {
            }
            else if (diagnosticRxBuffer[1] == 0x01u)
            {
                if (lin_current_rcvd_nad() == 0x7Eu)
                {
                }
                else if (lin_current_rcvd_nad() == 0x7Fu)
                {
                }
                else
                {
                    diagnosticTxBuffer[1] = 0x01;
                    send_positive_response_message(2u);
                }
            }
            else if (diagnosticRxBuffer[1] == 0x02u)
            {
                if (lin_current_rcvd_nad() == 0x7Eu)
                {
                }
                else if (lin_current_rcvd_nad() == 0x7Fu)
                {
                }
                else
                {
                    diagnosticTxBuffer[1] = 0x02;
                    send_positive_response_message(2u);
                }
            }
            else
            {
                if (lin_current_rcvd_nad() == 0x7Eu)
                {
                }
                else if (lin_current_rcvd_nad() == 0x7Fu)
                {
                }
                else
                {
                    send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12); // NRC12
                }
            }
        }
        else
        {
            send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13
        }
    }
    else
    {
        if (lin_current_rcvd_nad() == 0x7Eu)
        {
        }
        else if (lin_current_rcvd_nad() == 0x7Fu)
        {
        }
        else
        {
            send_negative_response_message(UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F); // NRC7F
        }
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_clear_dtc_info(void)
{
    uint8_t usMsgLen = 0u;
    usMsgLen = diagRxSize;
    if (usMsgLen == 0x04u)
    {
        if (lin_current_rcvd_nad() == 0x7Eu)
        {
        }
        else if (lin_current_rcvd_nad() == 0x7Fu)
        {
        }
        else
        {
            if ((diagnosticRxBuffer[1u] == 0xFFu) && (diagnosticRxBuffer[2u] == 0xFFu) && (diagnosticRxBuffer[3u] == 0xFFu))
            {
                send_positive_response_message(1u);
            }
            else
            {
                send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31);
            }
        }
    }
    else
    {
        send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13);
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_communction_control(void)
{
    if (session_mode == (uint8_t)SESSION_MODE_EXTEND)
    {
        if (diagRxSize == 3u)
        {
            if ((diagnosticRxBuffer[1u] == 0x80u) || (diagnosticRxBuffer[1u] == 0x00u)) // Cancel mute for the application message
            {
                if ((diagnosticRxBuffer[2u] == 0x01u) || (diagnosticRxBuffer[2u] == 0x03u))
                {
                    lin_configuration_RAM[1] = 0x11;
                    lin_configuration_RAM[2] = 0x13;
                    lin_configuration_RAM[3] = 0x14;
                    lin_configuration_RAM[4] = 0x12;

                    lin_configuration_ROM[1] = 0x11;
                    lin_configuration_ROM[2] = 0x13;
                    lin_configuration_ROM[3] = 0x14;
                    lin_configuration_ROM[4] = 0x12;
                    if (lin_current_rcvd_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_current_rcvd_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        if (diagnosticRxBuffer[1u] == 0x00u)
                        {
                            diagnosticTxBuffer[1] = diagnosticRxBuffer[1u];
                            send_positive_response_message(2u);
                        }
                    }
                }
                else
                {
                    if (lin_current_rcvd_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_current_rcvd_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31); // NRC31
                    }
                }
            }
            else if ((diagnosticRxBuffer[1u] == 0x83u) || (diagnosticRxBuffer[1u] == 0x03u)) // Start muting the application messages
            {
                if ((diagnosticRxBuffer[2u] == 0x01u) || (diagnosticRxBuffer[2u] == 0x03u))
                {
                    for (uint8_t i = 0; i < 4u; i++)
                    {
                        lin_configuration_RAM[1u + i] = 0xFFu;
                        lin_configuration_ROM[1u + i] = 0xFFu;
                    }

                    if (lin_current_rcvd_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_current_rcvd_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        if (diagnosticRxBuffer[1u] == 0x03u)
                        {
                            diagnosticTxBuffer[1] = diagnosticRxBuffer[1u];
                            send_positive_response_message(2u);
                        }
                    }
                }
                else
                {
                    if (lin_current_rcvd_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_current_rcvd_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31); // NRC31
                    }
                }
            }
            else if ((diagnosticRxBuffer[1u] == 0x82u) || (diagnosticRxBuffer[1u] == 0x02u))
            {
                if ((diagnosticRxBuffer[2u] == 0x01u) || (diagnosticRxBuffer[2u] == 0x03u))
                {
                    if (lin_current_rcvd_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_current_rcvd_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        if (diagnosticRxBuffer[1u] == 0x02u)
                        {
                            diagnosticTxBuffer[1] = diagnosticRxBuffer[1u];
                            send_positive_response_message(2u);
                        }
                    }
                }
                else
                {
                    if (lin_current_rcvd_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_current_rcvd_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31); // NRC31
                    }
                }
            }
            else
            {
                if (lin_current_rcvd_nad() == 0x7Eu)
                {
                }
                else if (lin_current_rcvd_nad() == 0x7Fu)
                {
                }
                else
                {
                    send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12); // NRC12
                }
            }
        }
        else
        {
            send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13
        }
    }
    else
    {
        send_negative_response_message(UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F); // NRC7F
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_tester_present_control(void)
{
    if (diagRxSize == 2)
    {
        if (diagnosticRxBuffer[1u] == 0x80)
        {
            diagnostic_session_cnt = TcTimeGet();
        }
        else if (diagnosticRxBuffer[1u] == 0x00)
        {
            diagnostic_session_cnt = TcTimeGet();
            if ((lin_current_rcvd_nad() == 0x7Eu) || (lin_current_rcvd_nad() == 0x7Fu))
            {
            }
            else
            {
                diagnosticTxBuffer[1u] = diagnosticRxBuffer[1u];
                send_positive_response_message(2);
            }
        }
        else
        {
            if ((lin_current_rcvd_nad() == 0x7Eu) || (lin_current_rcvd_nad() == 0x7Fu))
            {
            }
            else
            {
                send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12); // NRC12
            }
        }
    }
    else
    {
        send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_rest(void)
{
    uint8_t mslen = 0;
    mslen = diagRxSize;
    if (mslen == 2u)
    {
        if (diagnosticRxBuffer[1u] == 0x01u)
        {
            if (((door_cmd.VehicleSpeedValid == 1u) && (door_cmd.VehicleSpeed < 0x36u)) || (door_cmd.VehicleSpeedValid == 0u))
            {
                if (lin_current_rcvd_nad() == 0x7Eu)
                {
                }
                else if (lin_current_rcvd_nad() == 0x7Fu)
                {
                }
                else
                {
                    diagnosticTxBuffer[1u] = diagnosticRxBuffer[1u];
                    send_positive_response_message(2);
                }
                for (uint8_t i = 0; i < 60u; i++)
                {
                    delay1ms(1);
                }

                ll_wdg_enable(false);
                NVIC_SystemReset();
            }
            else
            {
                send_negative_response_message(UDS_COND_NOT_CORRECT_22); // NRC22
            }
        }
        else if (diagnosticRxBuffer[1u] == 0x81u)
        {
            if (((door_cmd.VehicleSpeedValid == 1u) && (door_cmd.VehicleSpeed < 0x36u)) || (door_cmd.VehicleSpeedValid == 0u))
            {
                ll_wdg_enable(false);
                NVIC_SystemReset();
            }
            else
            {
                send_negative_response_message(UDS_COND_NOT_CORRECT_22); // NRC22
            }
        }
        else if (diagnosticRxBuffer[1u] == 0x02u)
        {
            if (lin_current_rcvd_nad() == 0x7Eu)
            {
            }
            else if (lin_current_rcvd_nad() == 0x7Fu)
            {
            }
            else
            {
                diagnosticTxBuffer[1u] = diagnosticRxBuffer[1u];
                send_positive_response_message(2);
            }
        }
        else if (diagnosticRxBuffer[1u] == 0x03u)
        {
            if (lin_current_rcvd_nad() == 0x7Eu)
            {
            }
            else if (lin_current_rcvd_nad() == 0x7Fu)
            {
            }
            else
            {
                diagnosticTxBuffer[1u] = diagnosticRxBuffer[1u];
                send_positive_response_message(2);
            }
        }
        else if ((diagnosticRxBuffer[1u] == 0x82u) || (diagnosticRxBuffer[1u] == 0x83u))
        {
        }
        else
        {
            if (lin_current_rcvd_nad() == 0x7Eu)
            {
            }
            else if (lin_current_rcvd_nad() == 0x7Fu)
            {
            }
            else
            {
                send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12); // NRC12
            }
        }
    }
    else
    {
        send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13
    }
}

/* PRQA S 2889 3 #3257 - Multiple return statements for logical clarity and efficiency */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_readdata_by_id(void)
{
    uint8_t result = UDS_FALSE;
    uint8_t positresp = UDS_FALSE;
    (void)positresp;
    uint16_t msglen = 0u, datalen;
    uint16_t locdid = 0xFFFFu;
    mult_did_data_t mult_did;

    if (lin_current_rcvd_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_current_rcvd_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }

    msglen = diagRxSize;

    /* message length correct check */
    if (UDS_READ_BY_DID_REQ_LEN == msglen)
    {
        locdid = ((uint16_t)diagnosticRxBuffer[1u] << 8u) + diagnosticRxBuffer[2u];
        result = uds_diag_DID_chk(locdid);
        /* DID supported */
        if (result != 0u)
        {
            /*call the user function to process the service after all checks are correct*/
            user_read_data_by_id(0, 0, locdid, &datalen);
            msglen = (datalen + UDS_READ_BY_DID_MIN_RESP_LEN);
            if (locdid == 0x0001u) /*The production line test RAW does not respond to DID*/
            {
                msglen = datalen + 1u;
            }
            else
            {
                diagnosticTxBuffer[1u] = (uint8_t)(locdid >> 8u);
                diagnosticTxBuffer[2u] = (uint8_t)locdid;
            }
            send_positive_response_message(msglen);
        }
        else
        {
            send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31); // NRC31
        }
    }
    else if ((msglen > UDS_READ_BY_DID_REQ_LEN) && (msglen <= (((uint16_t)MULT_DID_MAX * 2u) + 1u)) && (((msglen - 1u) % 2u) == 0u)) // Up to 5 dids
    {
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        memset(&mult_did, 0, sizeof(mult_did_data_t));
        mult_did.did_num = (uint8_t)((msglen - 1u) / 2u);

        for (uint8_t i = 0; i < mult_did.did_num; i++)
        {
            mult_did.did_array[i] = ((uint16_t)diagnosticRxBuffer[(2u * i) + 1u] << 8) + diagnosticRxBuffer[(2u * i) + 2u];
            result = uds_diag_DID_chk(mult_did.did_array[i]);
            /* DID supported */
            if ((result != 0u) && (mult_did.did_array[i] != 0x0001u))
            {
                mult_did.did_valid_flag |= (uint16_t)1 << i;
            }
        }
        /*if any did is valid, response true*/
        if (mult_did.did_valid_flag != 0u)
        {
            for (uint8_t i = 0; i < mult_did.did_num; i++)
            {
                /*this did is valid and read data*/
                if ((mult_did.did_valid_flag & ((uint16_t)1u << i)) != 0u)
                {
                    diagnosticTxBuffer[mult_did.data_len + 1u] = (uint8_t)(mult_did.did_array[i] >> 8u);
                    diagnosticTxBuffer[mult_did.data_len + 2u] = (uint8_t)mult_did.did_array[i];
                    /*call the user function to process the service after all checks are correct*/
                    user_read_data_by_id(1, (uint8_t)(mult_did.data_len + 3u), mult_did.did_array[i], &datalen);
                    mult_did.data_len += (datalen + 2u); /*data and 2 byte did*/
                }
            }
            msglen = (mult_did.data_len + 1u);
            send_positive_response_message(msglen);
        }
        else
        {
            send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31); // NRC31
        }
    }
    else
    {
        send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void uds_pal_store_data_set(uint32_t addr, uint8_t *data, uint16_t length)
{
    pal_store_data_set(addr, data, length);
}

void uds_diagnostic_configword_remap_nad(void)
{
    if ((uint8_t)LEFT_FRONT_DOOR == g_user_info.config_word) // Left front door handle
    {
        lin_configured_NAD = 0x68;
        lin_initial_NAD = 0x68;
        g_user_info.nad_info = 0x68;
#if (CONFIG_BYTE_WRITE_EN == 1)
        guserinfo_save(); // Write Acceleration
#endif
        lin_configuration_RAM[5] = 0x1;
        lin_configuration_ROM[5] = 0x1;
        LI0_response_error_signal = LI0_EHIS_FL_ResponseError;
        response_error = LI0_EHIS_FL_ResponseError;
        lin_response_error_byte_offset[1] = LIN_BYTE_OFFSET_LI0_EHIS_FL_ResponseError;
        lin_response_error_bit_offset[1] = LIN_BIT_OFFSET_LI0_EHIS_FL_ResponseError;
    }
    else if ((uint8_t)LEFT_REAR_DOOR == g_user_info.config_word) // Left rear door handle
    {
        lin_configured_NAD = 0x6A;
        lin_initial_NAD = 0x6A;
        g_user_info.nad_info = 0x6A;
#if (CONFIG_BYTE_WRITE_EN == 1)
        guserinfo_save(); // Write Acceleration
#endif
        lin_configuration_RAM[5] = 0x5;
        lin_configuration_ROM[5] = 0x5;
        LI0_response_error_signal = LI0_EHIS_RL_ResponseError;
        response_error = LI0_EHIS_RL_ResponseError;
        lin_response_error_byte_offset[1] = LIN_BYTE_OFFSET_LI0_EHIS_RL_ResponseError;
        lin_response_error_bit_offset[1] = LIN_BIT_OFFSET_LI0_EHIS_RL_ResponseError;
    }
    else if ((uint8_t)RIGHT_FRONT_DOOR == g_user_info.config_word) // Right front door handle
    {
        lin_configured_NAD = 0x69;
        lin_initial_NAD = 0x69;
        g_user_info.nad_info = 0x69;
#if (CONFIG_BYTE_WRITE_EN == 1)
        guserinfo_save(); // Write Acceleration
#endif
        lin_configuration_RAM[5] = 0x5;
        lin_configuration_ROM[5] = 0x5;
        LI0_response_error_signal = LI0_EHIS_FR_ResponseError;
        response_error = LI0_EHIS_FR_ResponseError;
        lin_response_error_byte_offset[1] = LIN_BYTE_OFFSET_LI0_EHIS_FR_ResponseError;
        lin_response_error_bit_offset[1] = LIN_BIT_OFFSET_LI0_EHIS_FR_ResponseError;
    }
    else if ((uint8_t)RIGHT_REAR_DOOR == g_user_info.config_word) // Right rear door handle
    {
        lin_configured_NAD = 0x6B;
        lin_initial_NAD = 0x6B;
        g_user_info.nad_info = 0x6B;
#if (CONFIG_BYTE_WRITE_EN == 1)
        guserinfo_save(); // Write Acceleration
#endif
        lin_configuration_RAM[5] = 0x2;
        lin_configuration_ROM[5] = 0x2;
        LI0_response_error_signal = LI0_EHIS_RR_ResponseError;
        response_error = LI0_EHIS_RR_ResponseError;
        lin_response_error_byte_offset[1] = LIN_BYTE_OFFSET_LI0_EHIS_RR_ResponseError;
        lin_response_error_bit_offset[1] = LIN_BIT_OFFSET_LI0_EHIS_RR_ResponseError;
    }
    else
    {
        (void)0;
    }
}

/* PRQA S 2889 3 #3257 - Multiple return statements for logical clarity and efficiency */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_assign_NAD(void)
{
    uint8_t fuc_id;

    if (lin_current_rcvd_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_current_rcvd_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if ((0xF3u == diagnosticRxBuffer[1u]) && (0x3Fu == diagnosticRxBuffer[2u]))
    {
        fuc_id = diagnosticRxBuffer[3u];
        switch (fuc_id)
        {
        case 0x01:
            /* start */
            g_config_word_state = CONFIGURE_WORD_STATE_START;
            break;

        case 0x02:
            /* assign */
            if (g_config_word_state == (uint8_t)CONFIGURE_WORD_STATE_START)
            {
                if ((diagnosticRxBuffer[5u] == (uint8_t)LEFT_FRONT_DOOR) || (diagnosticRxBuffer[5u] == (uint8_t)LEFT_REAR_DOOR) ||
                    (diagnosticRxBuffer[5u] == (uint8_t)RIGHT_FRONT_DOOR) || (diagnosticRxBuffer[5u] == (uint8_t)RIGHT_REAR_DOOR))
                {
#if (CONFIG_BYTE_WRITE_EN == 1)
                    g_user_info.config_word = diagnosticRxBuffer[5u];
                    uds_diagnostic_configword_remap_nad();
#endif
                    g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
                    /* PRQA S 3432 3 #3267 - Macro arguments are safely used without unintended operator precedence issues */
                    /* PRQA S 2742 2 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
                    /* PRQA S 2880 1 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
                    TC_LOGI(TAG, "cfg %x\n", g_user_info.config_word);
                }
            }
            break;

        case 0x03:
            /* save */
            if (g_config_word_state == (uint8_t)CONFIGURE_WORD_STATE_ASIGN)
            {
#if (CONFIG_BYTE_WRITE_EN == 1)
                guserinfo_save(); // Write Acceleration
#endif
                g_config_word_state = CONFIGURE_WORD_STATE_SAVE;
                /* PRQA S 1035 5 #1035 - Macro with variable arguments called without variable arguments (GNU extension), accepted by toolchain. */
                /* PRQA S 1036 4 #1036 - Comma before ## in variadic macro (GNU extension) is used to swallow comma when no variable args, supported by compiler. */
                /* PRQA S 3432 3 #3267 - Macro arguments are safely used without unintended operator precedence issues */
                /* PRQA S 2742 2 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
                /* PRQA S 2880 1 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
                TC_LOGI(TAG, "save\n");
                enable_swd(); // Enable SWD function
            }
            break;

        case 0x04:
            /* end */
            g_config_word_state = CONFIGURE_WORD_STATE_END;
            break;

        default:
            (void)0;
            break;
        }
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void lin_handle_uds(void)
{
    uint8_t idx = diagnosticRxBuffer[0u];

    negResponseCode = 0u;
    if (program_condition_check == 1u)
    {
        if (lin_current_rcvd_nad() == lin_configured_NAD)
        {
            if ((diagnosticRxBuffer[0] == 0x10u) && ((diagnosticRxBuffer[1] == 0x02u) || (diagnosticRxBuffer[1] == 0x82u)))
            {
                program_condition_check = 1;
            }
            else if ((diagnosticRxBuffer[0] == 0x3Eu) && ((diagnosticRxBuffer[1] == 0x00u) || (diagnosticRxBuffer[1] == 0x80u)))
            {
                program_condition_check = 1;
            }
            else
            {
                if (((diagnosticRxBuffer[0] == 0x85u) && (diagnosticRxBuffer[1] == 0x82u)) || ((diagnosticRxBuffer[0] == 0x28u) && (diagnosticRxBuffer[1] == 0x83u) && (diagnosticRxBuffer[2] == 0x03u)))
                {
                    program_condition_check = 1;
                }
                else
                {
                    program_condition_check = 0;
                }
            }
        }
    }
    switch (idx)
    {
    case SERVICE_CLEAR_DTC_INFO: // 14
        uds_diagnostic_clear_dtc_info();
        break;
    case SERVICE_SESSION_CONTROL: // 10
        uds_diagnostic_session_control();
        break;
    case SERVICE_ROUTINE_CONTROL: // 31
        uds_diagnostic_route_control();
        break;
    case SERVICE_READ_DATA_BY_IDENTIFY: // 22
        uds_diagnostic_readdata_by_id();
        break;
    case SERVICE_ASSIGN_NAD_VIA_SNPD: // B5
        uds_diagnostic_assign_NAD();
        break;
    case SERVICE_DTC_CONTROL: // 85
        uds_diagnostic_dtc_control();
        break;
    case SERVICE_COMMUNICATION_CONTROL: // 28
        uds_communction_control();
        break;
    case SERVICE_TESTER_PRESENT: // 3E
        uds_tester_present_control();
        break;
    case SERVICE_ECU_RESET: // 11
        uds_diagnostic_rest();
        break;
    case SERVICE_WRITE_DATA_BY_IDENTIFY: // 2E
        if (lin_current_rcvd_nad() == 0x7Eu)
        {
        }
        else if (lin_current_rcvd_nad() == 0x7Fu)
        {
        }
        else
        {
            send_negative_response_message(UDS_DID_SEC_ERR_33); // NRC33;
        }
        break;
    case SERVICE_REQUEST_DOWNLOAD:      // 34
    case SERVICE_TRANSFER_DATA:         // 36
    case SERVICE_REQUEST_TRANSFER_EXIT: // 37
    case SERVICE_SECURITY_ACCESS:       // 27
        if (lin_current_rcvd_nad() == 0x7Eu)
        {
        }
        else if (lin_current_rcvd_nad() == 0x7Fu)
        {
        }
        else
        {
            send_negative_response_message(UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F); // NRC7F;
        }
        break;
    default:
        if (lin_current_rcvd_nad() == 0x7Eu)
        {
        }
        else if (lin_current_rcvd_nad() == 0x7Fu)
        {
        }
        else
        {
            send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12);
        }
        break;
    }
}

/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
void lin_custom_diag_service_handle(uint8_t sid, uint8_t *ptr, uint16_t length)
{
    (void)sid;
    diagRxSize = (uint8_t)length;
    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    memcpy(diagnosticRxBuffer, ptr, length);
    diagnostic_session_cnt = TcTimeGet();
    lin_handle_uds();
}

/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
void lin_diagservice_read_by_identifier(uint8_t *ptr, uint16_t length)
{
    (void)length;
    uint8_t id;
    uint16_t supid, fid;

    /* Get supplier and function indentification in request */
    supid = ((uint16_t)ptr[3] << 8) | ptr[2];
    /* PRQA S 2983 1 #2983 - Assignment is intentional even though object is not subsequently used (e.g., for side-effect of volatile access, or to silence compiler warning). */
    fid = ((uint16_t)ptr[5] << 8) | ptr[4];
    /* Check Supplier ID and Function ID */

    if (((supid != product_id.supplier_id) && (supid != (uint16_t)LD_ANY_SUPPLIER)))
    {
        tl_slaveresp_cnt = 0;
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
        tl_service_status = LD_SERVICE_IDLE;
#endif  /* End (_TL_FRAME_SUPPORT_ = = _TL_MULTI_FRAME_) */
        /* PRQA S 3432 3 #3267 - Macro arguments are safely used without unintended operator precedence issues */
        /* PRQA S 2742 2 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
        /* PRQA S 2880 1 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
        TC_LOGI(TAG, "no reply %x-%x\n", supid, product_id.supplier_id);
        return;
    }

    id = ptr[1];

    switch (ptr[1])
    {
    case CUS_UDS_PRODUCT_IDENT:
        if ((ptr[2] == 0x3Fu) && (ptr[3] == 0xFFu) && (ptr[4] == 0x02u))
        {
            if ((ptr[5] == (uint8_t)LEFT_FRONT_DOOR) || (ptr[5] == (uint8_t)LEFT_REAR_DOOR) || (ptr[5] == (uint8_t)RIGHT_FRONT_DOOR) || (ptr[5] == (uint8_t)RIGHT_REAR_DOOR))
            {
                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                pal_store_data_get(CUS_SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
                if (g_user_info.config_word == ptr[5])
                {
                    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                    pal_store_data_get(CUS_SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
                    ptr[1] = 0x00;
                    ptr[2] = 0x00;
                    ptr[3] = 0x00;
                    ptr[4] = 0x02;
                    ptr[5] = g_user_info.config_word;
                    lin_diag_positive_notify(ptr[0], &ptr[1], 5);
                }
                else
                {
                    lin_diag_negative_notify(ptr[0], 0x72);
                }
            }
            else
            {
                lin_diag_negative_notify(ptr[0], 0x31);
            }
        }
        else
        {
            lin_diag_negative_notify(ptr[0], 0x31);
        }

        break;
    default:
        if ((id >= LIN_READ_USR_DEF_MIN) && (id <= LIN_READ_USR_DEF_MAX))
        {
            uint8_t data_callout[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            uint8_t data_len = 0;
            extern uint8_t ld_read_by_id_callout(uint8_t id, uint8_t *data);
            uint8_t retval = ld_read_by_id_callout(id, data_callout);

            /*If the User ID is supported, make positive response*/
                if (retval == (uint8_t)LD_POSITIVE_RESPONSE)
                {
                    for (uint8_t i = 0; i < 5u; i++)
                    {
                        if (data_callout[4u - i] != 0xFFu)
                        {
                            data_len = 5u - i;
                            break;
                        }
                    }

                    if (data_len > 0u)
                    {
                        lin_diag_positive_notify(ptr[0], &data_callout[0], data_len);
                    }
                    else
                    {
                        /* Make a negative slave response PDU */
                        lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
                    }
                }
            else if (retval == (uint8_t)LD_NEGATIVE_RESPONSE)
            /*If the User ID is not supported, make negative response*/
            {
                /* Make a negative slave response PDU */
                lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
            }
            else if (retval == (uint8_t)LD_ID_NO_RESPONSE)
            {
            }
            else
            {
                (void)0;
            }
        }
        else
        {
            /* Make a negative slave response PDU */
            lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
        }

        break;
    } /* End of switch */
}

void lin_diag_service_hook(void)
{
    /* PRQA S 3205 1 #3205 - Identifier '%1s' is intentionally unused (e.g., for future expansion, API compatibility, or debug purpose). */
    uint8_t result;

    if ((uint8_t)LD_COMPLETED == ld_tx_status())
    {
#ifdef LINUDS_WRITEBYID
        if (pendWritebyID) /*request to set session is received?*/
        {
            if (pendWritebyID)
            {
                pendWritebyID = UDS_FALSE;
            }
            /*call the user function to process the service after all checks are correct*/
            result = user_WriteDataById(writedid, writelen, &writedatarecord[0]);
            /* successfully write data? */
            if (result)
            {
                diagnosticTxBuffer[1u] = (uds_uc8)(writedid >> 8u);
                diagnosticTxBuffer[2u] = (uds_uc8)writedid;
                send_positive_response_message(UDS_SESSION_POSIT_RESP_LEN);
            }
#ifdef LINUDS_WRITEBYID_NRC_GPF
            else
            {
                negResponseCode = UDS_GENERAL_PROGRAMMING_FAIL_72;
                send_negative_response_message(negResponseCode);
            }
#endif /* ifdef LINUDS_WRITEBYID_NRC_GPF */
        }
#endif /* ifdef LINUDS_WRITEBYID */
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void store_system_data_init(void)
{
    /* PRQA S 3200 2 #3264 - Return value ignored, verified safe for system operation */
    store_slow_smart_read(CUS_CFG_WORD_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
    store_slow_smart_read(CUS_SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
    uds_diagnostic_configword_remap_nad();
    if (g_ota_info.lock_failed_index > 3u)
    {
        g_ota_info.lock_failed_index = 3;
    }
}

STATIC void guserinfo_save(void)
{
    uint32_t wbuf[2] = {0xFFFFFFFFu, 0xFFFFFFFFu};
    uint8_t *pb = (uint8_t *)wbuf;

    uint32_t crc = crc16_calculate_func(0xFFFF, (uint8_t *)&g_user_info, sizeof(g_user_info));
    /* PRQA S 3200 5 #3264 - Return value ignored, verified safe for system operation */
    memcpy(pb, (uint8_t *)&g_user_info, sizeof(g_user_info));
    /* PRQA S 1495 1 #1495 - Destination and source types are intentionally incompatible (e.g., using union for type punning or hardware register access). */
    memcpy(&pb[sizeof(g_user_info)], &crc, sizeof(crc));
    pal_store_erase(STORE_TYPE_SEL, CUS_CFG_WORD_BASE_ADDR, FLASH_SECTOR_SIZE);
    pal_store_write(STORE_TYPE_SEL, CUS_CFG_WORD_BASE_ADDR, (uint8_t *)wbuf, sizeof(wbuf));
}

STATIC void gsysinfo_save(void)
{
    uint32_t wbuf[2] = {0xFFFFFFFFu, 0xFFFFFFFFu};
    uint8_t *pb = (uint8_t *)wbuf;

    uint32_t crc = crc16_calculate_func(0xFFFF, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
    /* PRQA S 3200 5 #3264 - Return value ignored, verified safe for system operation */
    memcpy(pb, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
    /* PRQA S 1495 1 #1495 - Destination and source types are intentionally incompatible (e.g., using union for type punning or hardware register access). */
    memcpy(&pb[sizeof(g_ota_info)], &crc, sizeof(crc));
    pal_store_erase(STORE_TYPE_SEL, CUS_SYSTEM_PARAM_BASE_ADDR, FLASH_SECTOR_SIZE);
    pal_store_write(STORE_TYPE_SEL, CUS_SYSTEM_PARAM_BASE_ADDR, (uint8_t *)wbuf, sizeof(wbuf));
}

STATIC void enable_swd(void) // Enable SWD interface
{
    ll_gpio_afio_config(GPIO_PIN_0, AFIO_MUX_0); // SWCLK
    ll_gpio_afio_config(GPIO_PIN_1, AFIO_MUX_0); // SWDIO

    TouchEnableSamp(0); // Turn off the touch function
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void lin_customized_operation(void)
{
    /* Check boot jump to app flag and process it */
    uint16_t usMsgLen = 6;

    if (g_ota_info.app_need_res_flag == 1u)
    {
        g_ota_info.app_need_res_flag = 0u;
        gsysinfo_save();
        diagnosticTxBuffer[0u] = (uint8_t)SERVICE_SESSION_CONTROL + (uint8_t)UDS_POS_RESP_CODE;
        diagnosticTxBuffer[1u] = 0x01;
        diagnosticTxBuffer[2u] = (uint8_t)((uint16_t)P2_SERVER_MAX >> 8);
        diagnosticTxBuffer[3u] = (uint8_t)P2_SERVER_MAX;
        diagnosticTxBuffer[4u] = (uint8_t)((uint16_t)P2E_SERVER_MAX >> 8);
        diagnosticTxBuffer[5u] = (uint8_t)P2E_SERVER_MAX;
        ld_send_message(usMsgLen, diagnosticTxBuffer);
    }
}

/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void LinDiagnosticSessionCheck(void)
{
    if (session_mode > (uint8_t)SESSION_MODE_DEFAULT)
    {
        diagnostic_session_cnt++;
        if ((TcTimeGet() - diagnostic_session_cnt) >= 5000) /* 5s*/
        {
            diagnostic_session_cnt = TcTimeGet();
            session_mode = SESSION_MODE_DEFAULT;
            lin_configuration_RAM[1] = 0x11;
            lin_configuration_RAM[2] = 0x13;
            lin_configuration_RAM[3] = 0x14;
            lin_configuration_RAM[4] = 0x12;

            lin_configuration_ROM[1] = 0x11;
            lin_configuration_ROM[2] = 0x13;
            lin_configuration_ROM[3] = 0x14;
            lin_configuration_ROM[4] = 0x12;
            program_condition_check = 0;
        }
    }
    else
    {
        diagnostic_session_cnt = TcTimeGet();
    }
    if (g_ota_info.lock_failed_index > 2u)
    {
        /* PRQA S 3440 1 #3221 - Use of increment or decrement result is safe and required by logic */
        if ((++lock_failed_cnt) > 5000u)
        {
            lock_failed_cnt = 0;
            g_ota_info.lock_failed_index--;
            /* Set a storage flag bit to update the number of failed attempts for unlocking 27, and operate on flash in the main loop. Only reduce it once when g_ota_info.lock_failed_index == 3, giving one chance, it will not keep reducing to 0.*/
            unlock_failed_store_flag = 1;
        }
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void SysDoFlashRoutine27Service(void)
{
    if (unlock_failed_store_flag == 1u)
    {
        unlock_failed_store_flag = 0;
        gsysinfo_save();
    }
}
