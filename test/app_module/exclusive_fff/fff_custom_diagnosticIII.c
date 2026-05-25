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
#include "fff_diagnosticIII.h"
#include "fff_tc_log.h"
#include "fff_lin.h"
#include "fff_pal_store.h"
#include "fff_store_manager.h"
#include "fff_utilities.h"
#include "fff_touch_include.h"
#include "fff_lin_frame.h"
#include "fff_custom_diagnosticIII.h"
#include "fff_touch_config.h"

static const char *TAG = "custom";

#if defined APP_MATCH_BOOT
__attribute__((section(".ARM.__at_0x00004000"))) const char g_lin_sequence_num_version[24] = "SW:1.01.B_260327_3197_05";                                                                                        /* 24 byte*/
__attribute__((section(".ARM.__at_0x00004018"))) const char g_seres_app_software_version[21] = {'S', 'W', ':', 'E', 'H', 'I', 'S', 'F', 'L', '.', '1', '.', '3', 'C', '.', '0', '5', 0x20, 0x20, 0x20, 0x20};   /* 21 byte*/
#else
const char g_lin_sequence_num_version[24] = "SW:1.01.B_260313_3197_04";                                                                                        /* 24 byte*/
const char g_seres_app_software_version[21] = {'S', 'W', ':', 'E', 'H', 'I', 'S', 'F', 'L', '.', '1', '.', '3', 'C', '.', '0', '5', 0x20, 0x20, 0x20, 0x20};   /* 21 byte*/
#endif

/* general defines for UDS Session*/
#define UDS_TRUE 0x01u
#define UDS_FALSE 0x00u

#define P2_SERVER_MAX (500)
#define P2E_SERVER_MAX (200)

#define FLASH_SECTOR_SIZE (512)
#define FLASH_BASE_ADDR (0x00000000UL)
#define FLASH_END_ADDR (0x00010000UL)
#define CUS_CFG_WORD_BASE_ADDR (0x0000F800UL)                   /* 0.5k */
#define CUS_SYSTEM_PARAM_BASE_ADDR (0x0000FA00UL)               /* 0.5k */
#define FLASH_BOOT_SIZE (0x00003E00UL)                          /* 15.5k */
#define FLASH_DFU_INFO_ADDR (FLASH_BASE_ADDR + FLASH_BOOT_SIZE) /* 15.5k */
#define FLASH_DFU_INFO_SIZE (FLASH_SECTOR_SIZE)                 /* 0.5k */
// #define FLASH_APP_ADDR                  (FLASH_DFU_INFO_ADDR + FLASH_DFU_INFO_SIZE) /* 16k */
#define FLASH_APP_PARAM_SIZE (0x00000800UL)                                           /* 2K*/
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

#define MULT_DID_MAX            5

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

DEFINE_FAKE_VOID_FUNC(uds_diagnostic_configword_remap_nad);