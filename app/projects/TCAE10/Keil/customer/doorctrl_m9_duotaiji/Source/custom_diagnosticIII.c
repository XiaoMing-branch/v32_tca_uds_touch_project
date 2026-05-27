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

/** @brief 日志标签，用于TC_LOGI输出标识 */
static const char *TAG = "custom";

#if defined APP_MATCH_BOOT
/* PRQA S 1514 4 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/* PRQA S 3408 3 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 2071 2 #3269 - Language extension used for compiler and hardware optimization */
__attribute__((section(".ARM.__at_0x00004000"))) const char g_lin_sequence_num_version[24] = "SW:1.01.B_260525_3197_06"; /**< LIN序列号版本字符串（24字节），存放于Boot区固定地址0x4000 */                                                                                                             /* 24 byte*/
__attribute__((section(".ARM.__at_0x00004018"))) const char g_seres_app_software_version[21] = {'S', 'W', ':', 'E', 'H', 'I', 'S', 'F', 'L', '.', '1', '.', '3', 'C', '.', '0', '5', (char)0x20, (char)0x20, (char)0x20, (char)0x20}; /**< 应用软件版本字符串（21字节），存放于Boot区固定地址0x4018，含车门位置标识(FL/RL/FR/RR) */ /* 21 byte*/
#else
const char g_lin_sequence_num_version[24] = "SW:1.01.B_260525_3197_06"; /**< LIN序列号版本字符串（24字节）*/                                                                                      /* 24 byte*/
const char g_seres_app_software_version[21] = {'S', 'W', ':', 'E', 'H', 'I', 'S', 'F', 'L', '.', '1', '.', '3', 'C', '.', '0', '5', 0x20, 0x20, 0x20, 0x20}; /**< 应用软件版本字符串（21字节），含车门位置标识(FL/RL/FR/RR) */ /* 21 byte*/
#endif

/* general defines for UDS Session*/
#define UDS_TRUE 0x01u    /**< UDS布尔真值 */
#define UDS_FALSE 0x00u   /**< UDS布尔假值 */

#define P2_SERVER_MAX (500)   /**< P2定时器最大值（500ms），会话控制正响应中返回给Tester */
#define P2E_SERVER_MAX (200)  /**< P2扩展定时器最大值（200ms），用于编程/扩展会话 */

#define FLASH_SECTOR_SIZE (512)           /**< Flash扇区大小（512字节） */
#define FLASH_BASE_ADDR (0x00000000U)     /**< Flash基地址（起始地址） */
/* PRQA S 1534 ++ #3261 - Unused macro defined for future extension and configuration compatibility */
#define FLASH_END_ADDR (0x00010000UL)     /**< Flash结束地址（64KB） */
#define CUS_CFG_WORD_BASE_ADDR (0x0000F800U)                    /**< 用户配字存储基地址（Flash末尾0.5K） */ /* 0.5k */
#define CUS_SYSTEM_PARAM_BASE_ADDR (0x0000FA00U)                /**< 系统参数存储基地址（Flash末尾第二个0.5K） */ /* 0.5k */
#define FLASH_BOOT_SIZE (0x00003E00UL)                          /**< Bootloader固件大小（15.5KB） */ /* 15.5k */
#define FLASH_DFU_INFO_ADDR (FLASH_BASE_ADDR + FLASH_BOOT_SIZE) /**< DFU信息存储地址（Boot区之后） */ /* 15.5k */
#define FLASH_DFU_INFO_SIZE (FLASH_SECTOR_SIZE)                 /**< DFU信息存储大小（0.5KB，即1个扇区） */ /* 0.5k */

#define FLASH_APP_PARAM_SIZE (0x00000800U)                                            /**< 应用参数区大小（2KB） */ /* 2K*/
#define FLASH_APP_IMAGE_SIZE (FLASH_END_ADDR - FLASH_APP_ADDR - FLASH_APP_PARAM_SIZE) /**< 应用固件映像大小（46KB） */ /* 46k */
#define FLASH_APP_END_ADDR (FLASH_END_ADDR - FLASH_APP_PARAM_SIZE)                    /**< 应用区结束地址 */
#define FLASH_BOOT_VERSION_ADDR (0x00003900u)                                         /**< Bootloader版本号存储地址 */
#define FLASH_HW_VERSION_ADDR (0x00003908u)                                           /**< 硬件版本号存储地址 */

#define CUS_UDS_RECEIVE_BUFFER_SIZE (20) /**< UDS诊断接收缓冲区大小（20字节） */
#define CUS_UDS_SEND_BUFFER_SIZE (66)    /**< UDS诊断发送缓冲区大小（66字节） */
#define UDS_SESSION_POSIT_RESP_LEN 3u    /**< 会话控制正响应基本长度（3字节） */
#define UDS_SESSION_NEG_RESP_LEN 3u      /**< 负响应报文长度（3字节：SID+NRC+RSID） */
#define UDS_SUPPRESS_POS_RESP_INDIC_BIT 0x80u /**< 抑制正响应指示位掩码（SubFunction的bit7） */
#define UDS_SID_MASK_WO_RESP_IND_BIT 0x7Fu    /**< 去掉抑制位后的SID掩码 */
#define UDS_NEG_RESP_RSID 0x7Fu               /**< 负响应服务标识符（固定0x7F） */
#define UDS_POS_RESP_CODE 0x40u               /**< 正响应偏移码（请求SID+0x40=正响应SID） */
#define UDS_DIAG_SESSION_POSIT_RESP_LEN 6u    /**< 会话控制正响应完整长度（6字节：含P2/P2*定时器） */
#define UDS_DIAG_SESSION_REQ_LEN 2u           /**< 会话控制请求报文长度（2字节：SID+SubFunc） */
#define UDS_DIAG_ROUTE_POSIT_RESP_LEN 5u      /**< 例程控制正响应长度（5字节） */
#define UDS_DIAG_ROUTE_REQ_LEN 4u             /**< 例程控制请求报文长度（4字节：SID+SubFunc+RID_H+RID_L） */
#define UDS_READ_BY_DID_MIN_RESP_LEN 3u       /**< 读DID正响应最小长度（3字节：RespSID+DID_H+DID_L） */
#define UDS_READ_BY_DID_REQ_LEN 3u            /**< 读DID请求报文长度（3字节：SID+DID_H+DID_L） */

#define CUS_UDS_PRODUCT_IDENT 0xF3u /**< 产品标识符，用于LIN从节点供应商/功能ID匹配查询 */

/* negative response code for UDS services */
#define UDS_SERVICE_NOT_SUPPORTED_11 0x11u /**< NRC 0x11: 请求的服务不支持 */ /* This response code is in general supported by each diagnostic service */
#define UDS_SUBFUNC_NOT_SUPP_12 0x12u      /**< NRC 0x12: 子功能不支持 */
#define UDS_INCOR_LEN_INVALID_FORMAT_13 0x13u /**< NRC 0x13: 报文长度错误或格式无效 */
#define UDS_RESPONSE_TOO_LONG_14 0x14u     /**< NRC 0x14: 响应报文过长 */
#define UDS_BUSY_REPEAT_REQUEST_21 0x21u   /**< NRC 0x21: 忙，请重发请求 */
#define UDS_COND_NOT_CORRECT_22 0x22u      /**< NRC 0x22: 条件不满足（如车速过高） */
#define UDS_REQU_SEQU_ERROR_24 0x24u       /**< NRC 0x24: 请求顺序错误 */
#define UDS_REQUEST_OUT_OF_RANGE_31 0x31u  /**< NRC 0x31: 请求超出范围（DID/RID不支持） */
#define UDS_DID_SEC_ERR_33 0x33u           /**< NRC 0x33: DID安全访问被拒绝（需安全解锁） */
#define UDS_GENERAL_PROGRAMMING_FAIL_72 0x72u /**< NRC 0x72: 通用编程失败 */
#define UDS_REQ_CORR_REC_RESP_PEND_78 0x78u /**< NRC 0x78: 请求正确接收，响应待定 */
#define UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F 0x7Fu /**< NRC 0x7F: 当前会话模式下不支持该服务 */
#define UDS_COND_NOT_SUPPORT_7E 0x7Eu       /**< NRC 0x7E: 当前会话下条件不支持（如从默认会话直接切编程会话） */

#define CONFIGURE_WORD_STATE_INIT 0   /**< 配字状态机：初始状态 */
#define CONFIGURE_WORD_STATE_START 1  /**< 配字状态机：开始状态（接收到0xB5 0x01） */
#define CONFIGURE_WORD_STATE_ASIGN 2  /**< 配字状态机：分配状态（已分配车门位置） */
#define CONFIGURE_WORD_STATE_SAVE 3   /**< 配字状态机：保存状态（已写入Flash） */
#define CONFIGURE_WORD_STATE_END 4    /**< 配字状态机：结束状态（配字流程完成） */
/* PRQA S 1534 -- */
#define MULT_DID_MAX 5                /**< 多DID模式下支持的最大DID数量（最多5个） */

/* PRQA S 1514 ++ #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/* PRQA S 3408 ++ #3218 - External linkage function defined without prior declaration, intentional design */
/** @brief 触摸通道原始/基线/差值数据，用于DID 0x0001读取 */
lin_touch_data touch_data;

/**
 * @brief  OTA DFU信息结构体（与Bootloader对齐）
 * @note   存储上一次DFU升级的指纹信息，用于诊断DID 0xF184读取
 */
typedef struct
{
    uint8_t fingerprint[10]; /**< DFU指纹（10字节标识符） */
    uint32_t magic;          /**< 魔数，用于校验DFU信息有效性 */
    uint32_t image_size;     /**< DFU镜像大小（字节） */
} last_dfu_info_t;

/**
 * @brief  多DID读取数据结构体（用于服务0x22多DID模式）
 */
typedef struct
{
    uint8_t did_num;                /**< DID数量 */
    uint16_t data_len;              /**< 已组装的数据长度 */
    uint16_t did_valid_flag;        /**< DID有效标志位掩码（位0~4对应5个DID） */
    uint16_t did_array[MULT_DID_MAX]; /**< DID数组（最多5个） */
} mult_did_data_t;

/**
 * @brief OTA系统信息结构体
 * @note  持久化到Flash的系统参数，含安全访问失败计数和预编程状态
 */
typedef struct
{
    uint8_t app_req_ext_program_flag; /**< 预编程请求标志（1=请求进入编程会话，3=LIN从节点模式请求） */
    uint8_t app_need_res_flag;        /**< Boot跳转App标志（1=需要发送会话控制正响应） */
    uint8_t lock_failed_index;        /**< 27服务安全访问锁定失败计数索引（0~3，超过3次锁定） */
    /* PRQA S 1504 16 #3220 -  Object used only in local translation unit, intentional design */
    /* PRQA S 2071 15 #3269 - Language extension used for compiler and hardware optimization */
} ota_cfg_t __attribute__((aligned(1)));

/** @brief UDS诊断接收缓冲区，存储从LIN总线接收到的诊断请求报文 */
uint8_t diagnosticRxBuffer[CUS_UDS_RECEIVE_BUFFER_SIZE] = {0};
/** @brief UDS诊断发送缓冲区，存储待发送的诊断响应报文 */
uint8_t diagnosticTxBuffer[CUS_UDS_SEND_BUFFER_SIZE] = {0};
/** @brief 当前诊断请求报文长度（字节数） */
uint8_t diagRxSize = 0;
/** @brief 诊断负响应码，非0时表示需要发送NRC响应 */
uint8_t negResponseCode = 0;
/** @brief 上次DFU指纹信息，含fingerprint、magic和image_size */
last_dfu_info_t g_dfu_info = {0};
/** @brief 用户配置信息（含config_word和nad_info），持久化到Flash */
user_cfg_t g_user_info = {0};
/** @brief OTA系统信息（含预编程请求标志、安全访问失败计数等） */
ota_cfg_t g_ota_info = {0};
/** @brief 配字状态机状态：0=Init, 1=Start, 2=Assign, 3=Save, 4=End */
uint8_t g_config_word_state = 0;
/** @brief 当前UDS诊断会话模式：1=Default, 2=Programming, 3=Extended */
uint8_t session_mode = SESSION_MODE_DEFAULT;
/** @brief 预编程条件检查标志（由例程0x0203设置），满足后允许切编程会话 */
uint8_t program_condition_check = 0;
/** @brief 安全访问27服务锁定失败计数器（仅用于延时递减） */
uint16_t lock_failed_cnt = 0;
/** @brief 安全访问失败次数需写入Flash的标志位 */
uint8_t unlock_failed_store_flag = 0;
/** @brief 诊断会话活动时间戳（ms），用于5秒超时检测 */
uint32_t diagnostic_session_cnt = 0;
/* PRQA S 1514 -- */
/* PRQA S 3408 -- */

STATIC void guserinfo_save(void);
STATIC void gsysinfo_save(void);
STATIC void enable_swd(void); // Enable SWD interface

/**
 * @brief  发送UDS负响应报文
 * @note   构造NRC响应：SID=0x7F, RSID=请求SID, NRC=错误码
 *         使用ld_send_message发送。发送状态需为LD_COMPLETED或LD_N_AS_TIMEOUT
 * @param negrespcode 负响应码（NRC），参见UDS_xxx定义：
 *         - UDS_SERVICE_NOT_SUPPORTED_11(0x11): 请求的服务不支持
 *         - UDS_SUBFUNC_NOT_SUPP_12(0x12): 子功能不支持
 *         - UDS_INCOR_LEN_INVALID_FORMAT_13(0x13): 报文长度错误/格式无效
 *         - UDS_COND_NOT_CORRECT_22(0x22): 条件不满足
 *         - UDS_REQUEST_OUT_OF_RANGE_31(0x31): 请求超出范围
 *         - UDS_DID_SEC_ERR_33(0x33): 安全访问被拒绝
 *         - UDS_GENERAL_PROGRAMMING_FAIL_72(0x72): 编程失败
 *         - UDS_REQ_CORR_REC_RESP_PEND_78(0x78): 请求正确接收-响应待定
 *         - UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F(0x7F): 当前会话不支持
 *         - UDS_COND_NOT_SUPPORT_7E(0x7E): 当前会话下条件不支持
 * @retval 无
 */
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

/**
 * @brief  发送UDS正响应报文
 * @note   将诊断请求SID+0x40作为正响应SID，调用ld_send_message发送。
 *         LIN发送状态需为LD_COMPLETED或LD_N_AS_TIMEOUT
 * @param msglen 响应报文长度（字节）
 * @retval 无
 */
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

/**
 * @brief  校验DID是否在支持列表中
 * @note   支持的DID列表：
 *         0xF187(零件号), 0xF18A(供应商代码), 0xF197(ECU名称),
 *         0xF189(LIN序列号), 0xF089(硬件版本), 0xF180(Boot版本),
 *         0xF184(指纹), 0x0216(软件版本), 0xF190(保留), 0xF0FA(保留),
 *         0x0001(触摸原始数据)
 * @param ucSess DID值
 * @retval UDS_TRUE  DID在支持列表中
 * @retval UDS_FALSE DID不在支持列表中
 */
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

/**
 * @brief  读取DID数据（内部函数）
 * @note   根据DID类型组装对应数据到diagnosticTxBuffer。
 *         DID数据格式：
 *         - 0xF187: 12字节ASCII零件号字符串
 *         - 0xF18A: 10字节供应商代码（BCD码）
 *         - 0xF197: 10字节ECU名称（根据config_word动态调整FL/RL/FR/RR）
 *         - 0xF189: 24字节LIN序列号版本
 *         - 0x0216: 21字节应用软件版本（含车门位置标识）
 *         - 0xF184: 10字节指纹信息（从DFU Info区读取）
 *         - 0xF089: 8字节硬件版本（从Flash读取）
 *         - 0xF180: 8字节Bootloader版本（从Flash读取）
 *         - 0xF190/0xF0FA: 保留DID，无数据
 *         - 0x0001: 27字节触摸通道原始/基线/差值数据（产线测试用）
 * @param mul_flag 多DID模式标志（0=单DID, 1=多DID）
 * @param mul_len  多DID模式下当前数据偏移位置
 * @param did      数据标识符
 * @param len      输出参数：数据长度
 * @retval 无
 */
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
    /* DID 0xF187: Seres零件号 - 12字节ASCII字符串 "4280310-RW02" */
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
    /* DID 0xF18A: Seres供应商代码 - 10字节BCD码 "3233" */
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
    /* DID 0xF197: Seres ECU名称 - 10字节ASCII，根据车门位置动态填充 EHIS_FL/RL/FR/RR */
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
    /* DID 0xF189: LIN序列号版本 - 24字节版本字符串 "SW:1.01.B_260525_3197_06" */
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
    /* DID 0x0216: 应用软件版本 - 21字节，含车门位置标识(第7-8字符: FL/RL/FR/RR) */
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
    /* DID 0xF184: 指纹信息 - 10字节，从DFU Info区的fingerprint字段读取 */
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
    /* DID 0xF089: 硬件版本 - 8字节，从Flash的HW_VERSION区读取 */
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
    /* DID 0xF180: Bootloader版本 - 8字节，从Flash的BOOT_VERSION区读取 */
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

    /* DID 0x0001: 产线测试用触摸通道原始数据（27字节）
       数据布局：Key1[0] raw/base/diff(6B) + Key1[1] raw/base/diff(6B) +
                 Key2[0] raw/base/diff(6B) + Key2[1] raw/base/diff(6B) +
                 noise_raw(2B) + key_val(1B) = 27B */
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

        /* 噪声原始值（大端序2字节） */
        diagnosticTxBuffer[25] = (uint8_t)((uint16_t)touch_data.noise_raw >> 8);
        diagnosticTxBuffer[26] = (uint8_t)touch_data.noise_raw;

        /* 触摸按键状态标志（低2位有效） */
        diagnosticTxBuffer[27] = (uint8_t)(touch_data.key_val & 0x3u);
        break;

    default:
        (void)0;
        break;
    }
}

/**
 * @brief  诊断会话控制（服务0x10）
 * @note   会话切换逻辑：
 *         0x01/0x81 -> 默认会话(DEFAULT SESSION)
 *           - 切换到默认会话，恢复LIN通信配置
 *           - 所有非诊断通信正常运行
 *         0x02/0x82 -> 编程会话(PROGRAMMING SESSION)
 *           - 需先通过例程控制0x0203设置 program_condition_check=1
 *           - 满足条件时设置app_req_ext_program_flag并系统复位进入Bootloader
 *           - 不满足条件时：若从默认会话切过来返回NRC7E，否则返回NRC22
 *         0x03/0x83 -> 扩展会话(EXTENDED SESSION)
 *           - 无限制切换，允许DTC控制、通信控制等扩展诊断服务
 *           - 5秒无活动自动退回默认会话
 * @param  无（全局diagnosticRxBuffer/diagRxSize）
 * @retval 无
 */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void uds_diagnostic_session_control(void)
{
    uint8_t ucSess = 0u;
    uint8_t usMsgLen = 0u;

    usMsgLen = diagRxSize;
    /* 确认报文长度为2字节（SID + SubFunction） */
    if (UDS_DIAG_SESSION_REQ_LEN == usMsgLen)
    {
        ucSess = diagnosticRxBuffer[1u];
        switch (ucSess)
        {
        case 0x01:   /* 默认会话 - 正响应 */
        case 0x81:   /* 默认会话 - 抑制正响应 */
            session_mode = SESSION_MODE_DEFAULT;
            break;
        case 0x02:   /* 编程会话 - 正响应 */
        case 0x82:   /* 编程会话 - 抑制正响应 */
            /* 必须先通过例程0x0203设置 program_condition_check=1 */
            if (program_condition_check == 1)
            {
                program_condition_check = 0;

                /* NAD=0x7E/0x7F为LIN从节点诊断广播地址，此时ECU作为功能寻址从节点，
                   设置预编程标志为3（从节点模式），否则为标准诊断请求，设置为1 */
                if ((lin_current_rcvd_nad() == 0x7Eu) || (lin_current_rcvd_nad() == 0x7Fu))
                    g_ota_info.app_req_ext_program_flag = 3;
                else
                    g_ota_info.app_req_ext_program_flag = 1;
                gsysinfo_save();
                delay1ms(1);
                ll_wdg_enable(false);
                NVIC_SystemReset();
            }
            else
            {
                /* 不满足预编程条件 */
                if ((lin_current_rcvd_nad() == 0x7Eu) || (lin_current_rcvd_nad() == 0x7Fu))
                {
                    /* NAD=0x7E/0x7F时静默处理：不发送NRC，避免LIN从节点广播诊断冲突 */
                }
                else
                {
                    if (ucSess == 0x02)
                    {
                        if (session_mode == SESSION_MODE_DEFAULT)
                        {
                            /* 从默认会话直接切编程会话：条件不满足（需先进入扩展会话执行预编程条件检查） */
                            send_negative_response_message(UDS_COND_NOT_SUPPORT_7E); // NRC7E: 当前会话不支持
                        }
                        else
                        {
                            /* 扩展会话下但未调用预编程例程0x0203 */
                            send_negative_response_message(UDS_COND_NOT_CORRECT_22); // NRC22: 条件不满足
                        }
                    }
                }
            }
            break;

        case 0x03:   /* 扩展会话 - 正响应 */
        case 0x83:   /* 扩展会话 - 抑制正响应 */
            diagnostic_session_cnt = TcTimeGet();
            session_mode = SESSION_MODE_EXTEND;
            break;
        default:
            /* 不支持的会话类型 */
            if (lin_current_rcvd_nad() == 0x7Eu)
            {
            }
            else if (lin_current_rcvd_nad() == 0x7Fu)
            {
            }
            else
            {
                send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12); // NRC12: 子功能不支持
            }

            break;
        }
        /* 默认会话(0x01)和扩展会话(0x03)需要发送正响应（含P2定时器参数） */
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
        /* 报文长度!=2，格式错误 */
        send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13: 报文长度错误或格式无效
    }
}

/**
 * @brief  例程控制（服务0x31）
 * @note   远程调用特定例程(Routine)。RID定义：
 *         0x0203 -> 预编程条件检查（需在扩展会话下，车速≤54km/h）
 *                   设置 program_condition_check=1，后续10 02可跳转编程会话
 *         0xFF01/0xFF00/0xDD02 -> 不支持的RID，返回NRC7F
 *         其他RID -> 返回NRC31
 *         子功能：0x01/0x81=开始例程
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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
    /* 确认报文长度为4字节（SID + SubFunc + RID_H + RID_L） */
    if (UDS_DIAG_ROUTE_REQ_LEN == usMsgLen)
    {
        /* 子功能必须是0x01(开始)或0x81(抑制正响应开始) */
        if ((diagnosticRxBuffer[1] == 0x01u) || (diagnosticRxBuffer[1] == 0x81u))
        {
            ucSess = (((uint16_t)diagnosticRxBuffer[2] << 8) | diagnosticRxBuffer[3]);
            diagnosticRxBuffer[1] = 0x01;
            if (ucSess == 0x0203u)
            {
                /* RID 0x0203: 预编程条件检查例程 */
                if (session_mode == (uint8_t)SESSION_MODE_EXTEND)
                {
                    /* 条件：车速有效且≤54km/h(0x36)，或车速无效 */
                    if (((door_cmd.VehicleSpeedValid == 1u) && (door_cmd.VehicleSpeed < 0x36u)) || (door_cmd.VehicleSpeedValid == 0u))
                    {
                        program_condition_check = 1;
                    }
                    else
                    {
                        /* 车速>54km/h时不允许进入预编程 */
                        send_negative_response_message(UDS_COND_NOT_CORRECT_22); // NRC22: 条件不满足（车速过高）
                    }
                }
                else
                {
                    /* 预编程条件检查必须在扩展会话下执行 */
                    negResponseCode = UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F; // NRC7F: 当前会话不支持
                }
            }
            else if ((ucSess == 0xFF01u) || (ucSess == 0xFF00u) || (ucSess == 0xDD02u))
            {
                /* 已知但不支持的RID */
                negResponseCode = UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F; // NRC7F: 当前会话不支持
            }
            else
            {
                /* 未知RID */
                negResponseCode = UDS_REQUEST_OUT_OF_RANGE_31; // NRC31: 请求超出范围
            }
        }
        else
        {
            /* 仅支持子功能0x01/0x81 */
            negResponseCode = UDS_SUBFUNC_NOT_SUPP_12; // NRC12: 子功能不支持
        }
    }
    else
    {
        /* 报文长度!=4 */
        negResponseCode = UDS_INCOR_LEN_INVALID_FORMAT_13; // NRC13: 报文长度错误或格式无效
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

/**
 * @brief  DTC设置控制（服务0x85）
 * @note   必须在扩展会话(SESSION_MODE_EXTEND)下执行。
 *         NAD=0x7E/0x7F时静默处理。
 *         子功能：
 *         0x01 -> 开启DTC存储（On）
 *         0x02 -> 关闭DTC存储（Off）
 *         0x81/0x82 -> 抑制正响应
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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

/**
 * @brief  清除DTC信息（服务0x14）
 * @note   清除诊断故障码(DTC)。
 *         报文格式：[0x14][0xFF][0xFF][0xFF]（固定4字节）
 *         groupOfDTC=0xFFFFFF时清除所有DTC，否则返回NRC31
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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

/**
 * @brief  通信控制（服务0x28）
 * @note   控制LIN通信报文的收发状态。必须在扩展会话下执行。
 *         NAD=0x7E/0x7F时静默处理。
 *         子功能：
 *         0x00/0x80 -> 使能应用报文接收（取消静默），需在扩展会话下
 *         0x03/0x83 -> 禁用应用报文接收（启动静默），设置NAD filter 0xFF
 *         0x02/0x82 -> 启用应用报文发送（使能响应）
 *         控制类型：0x01=使能Rx，0x03=禁用Rx
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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

/**
 * @brief  TesterPresent（服务0x3E）
 * @note   诊断仪保持会话激活。子功能：
 *         0x80 -> 抑制正响应，仅刷新会话超时计数器
 *         0x00 -> 需要正响应，同时刷新会话超时计数器
 *         其他子功能返回NRC12
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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

/**
 * @brief  ECU复位（服务0x11）
 * @note   支持子功能：
 *         0x01 -> 硬复位（HardReset），车速≤54km/h或无效时执行
 *         0x81 -> 抑制正响应硬复位
 *         0x02 -> 钥匙断电重启（KeyOffOnReset）
 *         0x03 -> 软复位（SoftReset）
 *         0x82/0x83 -> 抑制正响应，仅记录会话时间
 *         复位条件：车速无效或≤54km/h，否则返回NRC22
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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
/**
 * @brief  读取数据标识符（服务0x22）
 * @note   UDS DID读取实现，支持单DID和多DID（最多5个DID）读取。
 *         DID数据格式详见 user_read_data_by_id。
 *         支持的DID列表：
 *         - 0xF187: 零件号（12字节）
 *         - 0xF18A: 供应商代码（10字节）
 *         - 0xF197: ECU名称（10字节，根据车门位置动态修改）
 *         - 0xF189: LIN序列号版本（24字节）
 *         - 0x0216: 应用软件版本（21字节）
 *         - 0xF184: 指纹信息（10字节）
 *         - 0xF089: 硬件版本（8字节）
 *         - 0xF180: Bootloader版本（8字节）
 *         - 0xF190: 保留（长度0）
 *         - 0xF0FA: 保留（长度0）
 *         - 0x0001: 触摸原始数据（27字节，产线测试用）
 *         NAD=0x7E/0x7F时静默返回。
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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

    /* 报文长度3字节：单DID模式 [SID][DID_high][DID_low] */
    if (UDS_READ_BY_DID_REQ_LEN == msglen)
    {
        locdid = ((uint16_t)diagnosticRxBuffer[1u] << 8u) + diagnosticRxBuffer[2u];
        result = uds_diag_DID_chk(locdid);
        /* DID在支持列表中 */
        if (result != 0u)
        {
            user_read_data_by_id(0, 0, locdid, &datalen);
            msglen = (datalen + UDS_READ_BY_DID_MIN_RESP_LEN);
            if (locdid == 0x0001u) /* 产线测试RAW数据不使用标准DID应答格式 */
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
            send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31); // NRC31: DID不在支持列表中
        }
    }
    /* 多DID模式：报文格式为 SID + DID1_H + DID1_L + DID2_H + DID2_L + ...
       报文长度范围：5~11字节（1~5个DID），[msglen-1]必须为偶数 */
    else if ((msglen > UDS_READ_BY_DID_REQ_LEN) && (msglen <= (((uint16_t)MULT_DID_MAX * 2u) + 1u)) && (((msglen - 1u) % 2u) == 0u)) // 多DID模式，最多5个DID（偶数个字节）
    {
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        memset(&mult_did, 0, sizeof(mult_did_data_t));
        mult_did.did_num = (uint8_t)((msglen - 1u) / 2u); /* 从报文长度计算DID数量 */

        /* 第一轮：校验每个DID的有效性，标记有效DID位 */
        for (uint8_t i = 0; i < mult_did.did_num; i++)
        {
            mult_did.did_array[i] = ((uint16_t)diagnosticRxBuffer[(2u * i) + 1u] << 8) + diagnosticRxBuffer[(2u * i) + 2u];
            result = uds_diag_DID_chk(mult_did.did_array[i]);
            /* DID在支持列表中，且不是0x0001（RAW数据不支持多DID模式） */
            if ((result != 0u) && (mult_did.did_array[i] != 0x0001u))
            {
                mult_did.did_valid_flag |= (uint16_t)1 << i;
            }
        }
        /* 至少有一个有效DID才返回正响应 */
        if (mult_did.did_valid_flag != 0u)
        {
            /* 第二轮：逐DID读取数据并组装到发送缓冲区，每个DID格式：DID_H(1B) + DID_L(1B) + data(NB) */
            for (uint8_t i = 0; i < mult_did.did_num; i++)
            {
                if ((mult_did.did_valid_flag & ((uint16_t)1u << i)) != 0u)
                {
                    diagnosticTxBuffer[mult_did.data_len + 1u] = (uint8_t)(mult_did.did_array[i] >> 8u);
                    diagnosticTxBuffer[mult_did.data_len + 2u] = (uint8_t)mult_did.did_array[i];
                    user_read_data_by_id(1, (uint8_t)(mult_did.data_len + 3u), mult_did.did_array[i], &datalen);
                    mult_did.data_len += (datalen + 2u); /* +2字节DID标识 */
                }
            }
            msglen = (mult_did.data_len + 1u); /* +1字节正响应SID */
            send_positive_response_message(msglen);
        }
        else
        {
            send_negative_response_message(UDS_REQUEST_OUT_OF_RANGE_31); // NRC31: 所有DID都不支持
        }
    }
    else
    {
        send_negative_response_message(UDS_INCOR_LEN_INVALID_FORMAT_13); // NRC13: 报文长度错误
    }
}

/**
 * @brief  封装pal_store_data_set写入接口
 * @note   供外部模块调用的Flash数据写入封装函数
 * @param addr    Flash目标地址
 * @param data    待写入数据指针
 * @param length  写入数据长度
 * @retval 无
 */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void uds_pal_store_data_set(uint32_t addr, uint8_t *data, uint16_t length)
{
    pal_store_data_set(addr, data, length);
}

/**
 * @brief  根据config_word重新映射NAD和LIN配置
 * @note   根据车门位置（左前/左后/右前/右后）设置对应的NAD地址、
 *        LIN配置表、响应错误信号等。在配字分配或系统初始化时调用。
 *         左前(0) -> NAD=0x68, EHIS_FL
 *         左后(1) -> NAD=0x6A, EHIS_RL
 *         右前(2) -> NAD=0x69, EHIS_FR
 *         右后(3) -> NAD=0x6B, EHIS_RR
 * @param  无（全局g_user_info.config_word）
 * @retval 无
 */
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

/**
 * @brief  通过SNPD分配NAD（服务0xB5）
 * @note   产线配置字写入流程：通过串行号分段诊断(SnPD)为ECU分配车门位置。
 *         NAD=0x7E/0x7F时静默处理。
 *         协议格式：[0xB5][0xF3][0x3F][func_id][len][config_word]
 *         func_id:
 *           0x01 -> 开始（Start）
 *           0x02 -> 分配（Assign），校验车门位置值有效性
 *           0x03 -> 保存（Save），写入Flash并使能SWD
 *           0x04 -> 结束（End）
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
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
            /* start: 初始化配字状态机为开始 */
            g_config_word_state = CONFIGURE_WORD_STATE_START;
            break;

        case 0x02:
            /* assign: 分配车门位置，校验config_word是否在有效范围内 */
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
            /* save: 将配字写入Flash持久化 */
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
            /* end: 结束配字流程 */
            g_config_word_state = CONFIGURE_WORD_STATE_END;
            break;

        default:
            (void)0;
            break;
        }
    }
}

/**
 * @brief  UDS诊断请求分发主函数
 * @note   根据首字节SID分发到各服务处理函数。
 *         SID映射表：
 *         0x10 -> uds_diagnostic_session_control      (会话控制)
 *         0x11 -> uds_diagnostic_rest                  (ECU复位)
 *         0x14 -> uds_diagnostic_clear_dtc_info        (清除DTC)
 *         0x22 -> uds_diagnostic_readdata_by_id        (读取DID)
 *         0x28 -> uds_communction_control              (通信控制)
 *         0x2E -> 拒绝（当前不支持写DID，返回NRC33）
 *         0x31 -> uds_diagnostic_route_control         (例程控制)
 *         0x3E -> uds_tester_present_control           (TesterPresent)
 *         0x85 -> uds_diagnostic_dtc_control           (DTC设置控制)
 *         0xB5 -> uds_diagnostic_assign_NAD            (通过SN分配NAD)
 *         0x27/0x34/0x36/0x37 -> 返回NRC7F（当前会话不支持）
 *         NAD=0x7E/0x7F时静默处理（LIN从节点诊断）
 * @param  无（全局diagnosticRxBuffer）
 * @retval 无
 */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1505 1 #3219 - Function used only in the defining translation unit, intentional design */
void lin_handle_uds(void)
{
    uint8_t idx = diagnosticRxBuffer[0u];

    negResponseCode = 0u;
        /* 预编程条件检查标志保持逻辑：一旦program_condition_check=1，允许以下服务不重置该标志
       允许保持的服务：10 02/82(切编程会话)、3E 00/80(TesterPresent)、85 82(DTC关+抑制)、28 83 03(静默+抑制)
       其他服务则清除该标志 */
    if (program_condition_check == 1u)
    {
        if (lin_current_rcvd_nad() == lin_configured_NAD)
        {
            /* 0x10 会话控制（02/82=编程会话）：保持标志以允许后续进入Boot */
            if ((diagnosticRxBuffer[0] == 0x10u) && ((diagnosticRxBuffer[1] == 0x02u) || (diagnosticRxBuffer[1] == 0x82u)))
            {
                program_condition_check = 1;
            }
            /* 0x3E TesterPresent（00/80）：保持会话激活同时保留标志 */
            else if ((diagnosticRxBuffer[0] == 0x3Eu) && ((diagnosticRxBuffer[1] == 0x00u) || (diagnosticRxBuffer[1] == 0x80u)))
            {
                program_condition_check = 1;
            }
            else
            {
                /* 0x85 DTC控制（82=关+抑制正响应）或0x28 通信控制（83 03=静默+抑制）：保持标志 */
                if (((diagnosticRxBuffer[0] == 0x85u) && (diagnosticRxBuffer[1] == 0x82u)) || ((diagnosticRxBuffer[0] == 0x28u) && (diagnosticRxBuffer[1] == 0x83u) && (diagnosticRxBuffer[2] == 0x03u)))
                {
                    program_condition_check = 1;
                }
                else
                {
                    /* 其他任何服务请求都将清除预编程条件标志 */
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
            send_negative_response_message(UDS_DID_SEC_ERR_33); // NRC33: 安全访问被拒绝（写DID需要安全解锁）
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
            send_negative_response_message(UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F); // NRC7F: 当前会话不支持（需编程会话）
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
            send_negative_response_message(UDS_SUBFUNC_NOT_SUPP_12); // NRC12: 子功能不支持
        }
        break;
    }
}

/**
 * @brief  自定义诊断服务入口（由LIN协议栈回调）
 * @note   将接收到的诊断请求复制到diagnosticRxBuffer中，
 *        记录会话时间戳，然后调用 lin_handle_uds 进行服务分发
 * @param sid    服务标识符
 * @param ptr    请求报文数据指针
 * @param length 请求报文长度
 * @retval 无
 */
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

/**
 * @brief  通过标识符读取诊断数据（服务0xB2）
 * @note   LIN从节点的供应商/功能ID匹配检查。
 *         - CUS_UDS_PRODUCT_IDENT(0xF3): 产品标识查询，对比config_word确认车门位置
 *         - LIN_READ_USR_DEF_MIN~MAX: 用户自定义ID，通过ld_read_by_id_callout分发
 * @param ptr    诊断请求报文指针（包含SID+数据）
 * @param length 报文长度
 * @retval 无（通过 lin_diag_positive_notify / lin_diag_negative_notify 返回）
 */
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
        /* 产品标识查询(0xF3)：请求格式 [0xF3][0x3F][0xFF][0x02][config_word]
           校验头部固定值 0x3F/0xFF/0x02，以及config_word是否在有效车门位置范围内 */
        if ((ptr[2] == 0x3Fu) && (ptr[3] == 0xFFu) && (ptr[4] == 0x02u))
        {
            if ((ptr[5] == (uint8_t)LEFT_FRONT_DOOR) || (ptr[5] == (uint8_t)LEFT_REAR_DOOR) || (ptr[5] == (uint8_t)RIGHT_FRONT_DOOR) || (ptr[5] == (uint8_t)RIGHT_REAR_DOOR))
            {
                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                pal_store_data_get(CUS_SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
                /* 从Flash读取当前配字与请求的config_word比对，一致则返回产品标识正响应 */
                if (g_user_info.config_word == ptr[5])
                {
                    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                    pal_store_data_get(CUS_SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
                    /* 构造正响应：[0x00][0x00][0x00][0x02][config_word] */
                    ptr[1] = 0x00;
                    ptr[2] = 0x00;
                    ptr[3] = 0x00;
                    ptr[4] = 0x02;
                    ptr[5] = g_user_info.config_word;
                    lin_diag_positive_notify(ptr[0], &ptr[1], 5);
                }
                else
                {
                    /* config_word不匹配：NRC 0x72 通用编程失败 */
                    lin_diag_negative_notify(ptr[0], 0x72);
                }
            }
            else
            {
                /* config_word值无效（非LF/LR/RF/RR）：NRC 0x31 请求超出范围 */
                lin_diag_negative_notify(ptr[0], 0x31);
            }
        }
        else
        {
            /* 协议格式不匹配：NRC 0x31 */
            lin_diag_negative_notify(ptr[0], 0x31);
        }

        break;
    default:
        /* 用户自定义ID范围（LIN_READ_USR_DEF_MIN~MAX）：通过ld_read_by_id_callout回调分发处理 */
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

/**
 * @brief  诊断服务钩子函数（主循环中调用）
 * @note   处理WriteDataById的挂起写入请求（pendWritebyID），
 *        当LIN发送完成后执行实际的Flash写入操作
 * @param  无
 * @retval 无
 */
void lin_diag_service_hook(void)
{
    /* PRQA S 3205 1 #3205 - Identifier '%1s' is intentionally unused (e.g., for future expansion, API compatibility, or debug purpose). */
    uint8_t result;

    if ((uint8_t)LD_COMPLETED == ld_tx_status())
    {
#ifdef LINUDS_WRITEBYID
        if (pendWritebyID) /*request to set session is received?*/
        {
            /* 清除挂起标志，准备执行WriteDataById的实际Flash写入 */
            if (pendWritebyID)
            {
                pendWritebyID = UDS_FALSE;
            }
            /* 所有前置校验通过后，调用user_WriteDataById执行实际写入操作 */
            result = user_WriteDataById(writedid, writelen, &writedatarecord[0]);
            /* successfully write data? */
            if (result)
            {
                /* 写入成功：构造正响应 [RespSID][DID_H][DID_L] */
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

/**
 * @brief  系统数据初始化（上电时调用）
 * @note   从Flash读取用户配字信息(g_user_info)和OTA系统信息(g_ota_info)，
 *        根据config_word重新映射NAD和LIN配置，
 *        并对 lock_failed_index 做上限保护（最大3）
 * @param  无
 * @retval 无
 */
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

/**
 * @brief  保存用户配字信息到Flash
 * @note   将 g_user_info（含config_word/NAD等）写入
 *        CUS_CFG_WORD_BASE_ADDR，附带CRC16校验，
 *        写入前先擦除整个扇区
 * @param  无
 * @retval 无
 */
STATIC void guserinfo_save(void)
{
    uint32_t wbuf[2] = {0xFFFFFFFFu, 0xFFFFFFFFu};
    uint8_t *pb = (uint8_t *)wbuf;

    /* 计算用户配字数据的CRC16校验值，追加到数据末尾一起写入Flash，用于上电校验完整性 */
    uint32_t crc = crc16_calculate_func(0xFFFF, (uint8_t *)&g_user_info, sizeof(g_user_info));
    /* PRQA S 3200 5 #3264 - Return value ignored, verified safe for system operation */
    memcpy(pb, (uint8_t *)&g_user_info, sizeof(g_user_info));
    /* PRQA S 1495 1 #1495 - Destination and source types are intentionally incompatible (e.g., using union for type punning or hardware register access). */
    memcpy(&pb[sizeof(g_user_info)], &crc, sizeof(crc));
    pal_store_erase(STORE_TYPE_SEL, CUS_CFG_WORD_BASE_ADDR, FLASH_SECTOR_SIZE);
    pal_store_write(STORE_TYPE_SEL, CUS_CFG_WORD_BASE_ADDR, (uint8_t *)wbuf, sizeof(wbuf));
}

/**
 * @brief  保存OTA系统信息到Flash
 * @note   将 g_ota_info（含安全访问失败计数、预编程请求标志等）写入
 *        CUS_SYSTEM_PARAM_BASE_ADDR，附带CRC16校验
 * @param  无
 * @retval 无
 */
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

/**
 * @brief  使能SWD调试接口
 * @note   将GPIO_PIN_0/SWCLK和GPIO_PIN_1/SWDIO复用为调试功能，
 *        同时关闭触摸采样以释放引脚
 * @param  无
 * @retval 无
 */
STATIC void enable_swd(void)
{
    /* 将GPIO_PIN_0/SWCLK和GPIO_PIN_1/SWDIO复用为调试功能 */
    ll_gpio_afio_config(GPIO_PIN_0, AFIO_MUX_0); // SWCLK
    ll_gpio_afio_config(GPIO_PIN_1, AFIO_MUX_0); // SWDIO

    /* SWD引脚与触摸通道复用，使能SWD前必须关闭触摸采样以释放引脚控制权 */
    TouchEnableSamp(0); // Turn off the touch function
}

/**
 * @brief  上电后处理Boot跳转App时的诊断应答
 * @note   检测 app_need_res_flag，若置位则发送模拟的"10 01"会话控制正响应
 *        （含P2/P2*Server定时器值），告知诊断仪ECU已就绪
 * @param  无
 * @retval 无
 */
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

/**
 * @brief  诊断会话超时检测与自动回退
 * @note   非默认会话下若5秒内无诊断请求（TesterPresent或任何服务），
 *        自动切回默认会话（SESSION_MODE_DEFAULT），恢复LIN通信配置，
 *        清除预编程条件标志。同时递减安全访问失败计数（每5秒减1次）
 * @param  无
 * @retval 无
 */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void LinDiagnosticSessionCheck(void)
{
    /* 非默认会话下，若5秒内无任何诊断活动（无TesterPresent或其他服务请求），
       自动切回默认会话，恢复LIN通信配置（取消静默），并清除预编程条件标志 */
    if (session_mode > (uint8_t)SESSION_MODE_DEFAULT)
    {
        if ((TcTimeGet() - diagnostic_session_cnt) >= 5000) /* 5s*/
        {
            diagnostic_session_cnt = TcTimeGet();
            session_mode = SESSION_MODE_DEFAULT;
            /* 恢复LIN通信配置：重新使能应用报文接收（取消静默模式） */
            lin_configuration_RAM[1] = 0x11;
            lin_configuration_RAM[2] = 0x13;
            lin_configuration_RAM[3] = 0x14;
            lin_configuration_RAM[4] = 0x12;

            lin_configuration_ROM[1] = 0x11;
            lin_configuration_ROM[2] = 0x13;
            lin_configuration_ROM[3] = 0x14;
            lin_configuration_ROM[4] = 0x12;
            program_condition_check = 0; /* 清除预编程条件标志 */
        }
    }
    else
    {
        /* 默认会话下持续更新时间戳，避免误超时 */
        diagnostic_session_cnt = TcTimeGet();
    }
    /* 安全访问失败锁定计数器递减逻辑：
       当 lock_failed_index > 2（即已锁定）时启动延时递减。
       每5秒（5000次主循环）递减1次，直到 lock_failed_index 降至2（解锁一次尝试机会）。
       递减结果通过 unlock_failed_store_flag 标记在主循环中写入Flash持久化 */
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

/**
 * @brief  更新安全访问锁定失败次数字段到Flash
 * @note   当 lock_failed_index 递减后，将最新的失败计数持久化到系统参数存储区
 * @param  无
 * @retval 无
 */
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
