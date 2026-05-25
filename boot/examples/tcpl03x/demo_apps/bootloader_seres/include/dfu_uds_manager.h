/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   dfu_uds header file.
 *
 * @file    dfu_uds_manager.c
 * @author  AE/FAE team
 * @date    2024.01.01
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */

/* PRQA S 1534 ++ #3261 - Unused macro defined for future extension and configuration compatibility */
#ifndef DFU_UDS_MANAGER_H__
#define DFU_UDS_MANAGER_H__

#include <stdint.h>
#include "test_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Define diagnostic services id */
#define SERVICE_SESSION_CONTROL 0x10u       /**< service session control */
#define SERVICE_CLEAR_DTC_INFO 0x14u        /**< service clear dtc info */
#define SERVICE_SECURITY_ACCESS 0x27u       /**< service security access */
#define SERVICE_ECU_RESET 0x11u             /**< service ecu reset */
#define SERVICE_FIRMWARE_INFO_SYNC 0x2Eu    /**< service firmware info sync */
#define SERVICE_ROUTINE_CONTROL 0x31u       /**< service routine control */
#define SERVICE_REQUEST_DOWNLOAD 0x34u      /**< service request download */
#define SERVICE_TRANSFER_DATA 0x36u         /**< service transfer data */
#define SERVICE_REQUEST_TRANSFER_EXIT 0x37u /**< service transfer exit */
#define SERVICE_LINK_CONTROL 0x87u          /**< service link control */
#define SERVICE_READ_BY_IDENTIFY 0xB2u      /**< read by identify service */
#define SERVICE_ASSIGN_NAD_VIA_SNPD 0xB5u   /**< assign NAD via SN service */
#define SERVICE_READ_DATA_BY_IDENTIFY 0x22u /**< service read data by identifier */
#define SERVICE_COMMUNCATION_CONTROL 0x28u
#define SERVICE_DTC_CONTROL 0x85u
#define SERVICE_TESTER_PRESENT 0x3Eu /**< service tester present */

#define CUS_UDS_PRODUCT_IDENT 0xF3u
#define UDS_READ_BY_DID_MIN_RESP_LEN 0x3u
#define UDS_READ_BY_DID_REQ_LEN 0x3u

/* request communcation session mode */
#define DEFALUT_SESSION 1u
#define PROGRAM_SESSION 2u
#define EXTEND_SESSION 4u

#define DFU_INFO_MAGIC (0xDEADBEEFU)
#define LIN_UDS_TIMEOUT (5000u) // 5s,unit =1ms
#define DFU_ERASE_WAITTIME (1000u)
#define DFU_PROGRAM_WAITTIME (60u)
#define DFU_PROGRAM_LENGTH (512u)
#define DFU_PROGRAM_WORDS (DFU_PROGRAM_LENGTH >> 2u)

#define FLASH_SECTOR_SIZE (512)
#define FLASH_BASE_ADDR (0x00000000U)
#define FLASH_END_ADDR (0x00010000U)
#define FLASHDRV_ADDR (0x10000000u)

#define CFG_WORD_BASE_ADDR (0x0000F800U)                                              /* 0.5k */
#define SYSTEM_PARAM_BASE_ADDR (0x0000FA00U)                                          /* 0.5k */
#define FLASH_BOOT_SIZE (0x00003E00U)                                                 /* 15.5k */
#define FLASH_DFU_INFO_ADDR (FLASH_BASE_ADDR + FLASH_BOOT_SIZE)                       /* 15.5k */
#define FLASH_DFU_INFO_SIZE ((uint32_t)FLASH_SECTOR_SIZE)                             /* 0.5k */
#define FLASH_APP_ADDR (FLASH_DFU_INFO_ADDR + FLASH_DFU_INFO_SIZE)                    /* 16k */
#define FLASH_APP_PARAM_SIZE (0x00000800UL)                                           /* 2K*/
#define FLASH_APP_IMAGE_SIZE (FLASH_END_ADDR - FLASH_APP_ADDR - FLASH_APP_PARAM_SIZE) /* 46k */
#define FLASH_APP_END_ADDR (FLASH_END_ADDR - FLASH_APP_PARAM_SIZE)
#define FLASH_BOOT_VERSION_ADDR (0x00003900u)
#define FLASH_HW_VERSION_ADDR (0x00003908u)
#define FLASH_SEQ_NUM_ADDR (0x00004000u)
#define FLASH_SERES_APP_SOFTVER_ADDR (0x00004018u)
#define FLASH_DRIVER_ADDR (0x10000000u)
#define FLASH_DRIVER_LENGTH (0x40u)

#define BOOT_Frame_length_1 (1u)
#define BOOT_Frame_length_2 (2u)
#define BOOT_Frame_length_3 (3u)
#define BOOT_Frame_length_4 (4u)
#define BOOT_Frame_length_5 (5u)
#define BOOT_Frame_length_6 (6u)
#define BOOT_Frame_length_7 (7u)
#define BOOT_Frame_length_8 (8u)
#define BOOT_Frame_length_9 (9u)
#define BOOT_Frame_length_10 (10u)
#define BOOT_Frame_length_11 (11u)
#define BOOT_Frame_length_12 (12u)
#define BOOT_Frame_length_13 (13u)
#define BOOT_Frame_length_14 (14u)
#define BOOT_Frame_length_15 (15u)
#define BOOT_Frame_length_16 (16u)
#define BOOT_Frame_length_17 (17u)
#define BOOT_Frame_length_18 (18u)
#define BOOT_Frame_length_19 (19u)
#define BOOT_Frame_length_20 (20u)
#define BOOT_Frame_length_514 (514u)

#define QUEUE_LIN_LEN (2u)
#define LIN_BAUD_RATE (19200u)
#define P2_SERVER_MAX (500u)
#define P2E_SERVER_MAX (200u)
#define CONFIGURE_WORD_STATE_INIT 0u
#define CONFIGURE_WORD_STATE_START 1u
#define CONFIGURE_WORD_STATE_ASIGN 2u
#define CONFIGURE_WORD_STATE_SAVE 3u
#define CONFIGURE_WORD_STATE_END 4u
#define CUS_UDS_SEND_BUFFER_SIZE 66u

#define MULT_DID_MAX 5

    /**
     * @brief dfu msg error code enumeration
     */
    /* PRQA S 1535 ++ #3262 - Unused typedef defined for future extension and type consistency */
    typedef enum
    {
        DFU_MSG_SUCCESS = 0,
        DFU_MSG_ERROR,
        DFU_MSG_ERASE_ERROR,
        DFU_MSG_SYNC_ERROR,
        DFU_MSG_TRANFER_REQUEST_ERROR,
        DFU_MSG_TRANFER_EXIT_ERROR,
        DFU_MSG_SEQ_ERROR,
        DFU_MSG_PACKET_LEN_ERROR,
        DFU_MSG_INDEX_ERROR,
        DFU_MSG_PROGRA_ERROR,
        DFU_MSG_CRC_ERROR,
        DFU_MSG_TIMEOUT,
        DFU_MSG_MAX,
    } dfu_msg_error_code_e;

    /**
     * @brief dfu cmd enumeration
     */
    typedef enum
    {
        DFU_CMD_DEFAULT_SESSION = 0,
        DFU_CMD_EXTEND_SESSION,
        DFU_CMD_ROUTINE_PROGRAM_CHECK,
        DFU_CMD_PROGRAM_SESSION,
        DFU_CMD_SECURITY_SEED_REQUEST,
        DFU_CMD_SECURITY_KEY_CHECK,
        DFU_CMD_WRITE_FINGER,
        DFU_CMD_FLASH_DRIVER_REQUEST,
        DFU_CMD_FLASH_DRIVER_TRANSFER,
        DFU_CMD_FLASH_DRIVER_EXIT,
        DFU_CMD_FLASH_DRIVER_CMAC_CHECK,
        DFU_CMD_APP_ERASE,
        DFU_CMD_APP_REQUEST,
        DFU_CMD_APP_TRANSFER,
        DFU_CMD_APP_EXIT,
        DFU_CMD_APP_CMAC_CHECK,
        DFU_CMD_APP_COMPATIBLE_CHECK
    } dfu_cmd_e;

    /**
     * @brief bootlaoder state enumeration
     */
    typedef enum
    {
        BOOT_STATE_IDLE = 0,
        BOOT_STATE_USER_APP,
        BOOT_STATE_UPGRADE,
    } boot_state_e;

    /**
     * @brief packet struct
     */
    /* PRQA S 2071 4 #3269 - Language extension used for compiler and hardware optimization */
    typedef struct
    {
        uint32_t data[DFU_PROGRAM_WORDS];
    } __attribute__((aligned(4))) packet_unit_t;

    /**
     * @brief queue list struct
     */
    typedef struct
    {
        packet_unit_t packet[QUEUE_LIN_LEN];
        uint8_t head;
        uint8_t tail;
    } queue_list_t;

    /**
     * @brief service uds struct
     */
    typedef struct
    {
        uint8_t sessionMode;
    } ServiceUDS_TypeDef;

    /**
     * @brief app sys info
     */
    /* PRQA S 2071 5 #3269 - Language extension used for compiler and hardware optimization */
    typedef struct
    {
        uint8_t config_word; /* user's config word */
        uint8_t nad_info;
    } user_cfg_t __attribute__((aligned(1)));

    /**
     * @brief ota sys info
     */
    /* PRQA S 2071 6 #3269 - Language extension used for compiler and hardware optimization */
    typedef struct
    {
        uint8_t app_req_ext_program_flag; /* app req flag */
        uint8_t app_need_res_flag;        /* boot to app flag */
        uint8_t lock_failed_index;        /* 27 sid lock failed index */
    } ota_cfg_t __attribute__((aligned(1)));

    /**
     * @brief last dfu info struct
     */
    typedef struct
    {
        uint8_t fingerprint[10];
        uint32_t magic; /* APP valid flag*/
        uint32_t image_size;
    } last_dfu_info_t;

    typedef void (*FUNC_PTR)(void);

    /**
     * @brief dfu manager struct
     */
    typedef struct
    {
        uint32_t flashdrv_write_addr;
        uint32_t write_addr;
        uint32_t write_length;
        uint32_t write_index;
        uint32_t receive_length;
        uint32_t recevice_index;
        uint32_t uds_timeout;
        uint8_t op_code;
        uint8_t resp_value;
        uint8_t program_flag;
        uint8_t boot_state;
        uint8_t flashdrv_write_size;
        last_dfu_info_t dfu_info;
        queue_list_t queue_list;
    } dfu_manager_context_t;

    typedef enum
    {
        LEFT_FRONT_DOOR = 0,
        LEFT_REAR_DOOR,
        RIGHT_FRONT_DOOR,
        RIGHT_REAR_DOOR,
    } T_Door;
    /* PRQA S 1535 -- */
    /**
     * @brief dfu process struct
     */
    /* PRQA S 1336 1 #3263 - Function declaration omits parameter identifiers for interface compatibility, intentional design */
    typedef void (*dfu_func)(uint8_t *, uint16_t);
    typedef struct
    {
        uint8_t sid;
        dfu_func func;
    } dfu_process_context_t;

    typedef struct
    {
        uint8_t did_num;
        uint16_t data_len;
        uint16_t did_valid_flag;
        uint16_t did_array[MULT_DID_MAX];
    } mult_did_data_t;

    void dfu_manager_init(void);
    void dfu_timeout_handle(void);
    void main_loops(void);
    extern uint8_t lin_get_uds_nad(void);
    extern dfu_manager_context_t dfu_ctx;
    extern uint8_t lin_configured_NAD;
#ifdef __cplusplus
}
#endif
#endif /* __DFU_UDS_MANAGER_H__ */
/* PRQA S 1534 -- */
