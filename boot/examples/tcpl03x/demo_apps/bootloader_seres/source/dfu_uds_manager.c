/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   dfu_uds_manager source file.
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

#include "dfu_uds_manager.h"
#ifdef ENABLE_TEST_MODE
#include "fff_pal_store.h"
#include "fff_pal_systick.h"
#include "fff_pal_lin_comm.h"
#include "fff_pal_lin_tl.h"
#include "fff_pal_wdg.h"
#include "fff_aes_cmac.h"
#include "fff_utilities.h"
#include "fff_logging.h"
#include "fff_hardware.h"
#else
#include "pal_systick.h"
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "pal_lin_comm.h"
#include "pal_lin_tl.h"
#include "pal_store.h"
#include "pal_wdg.h"
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "aes_cmac.h"
#include "utilities.h"
#include "logging.h"
#endif

#define CFG_SUPPORT_DEBUG 0

#if 1 == CFG_SUPPORT_DEBUG
#define LOG_DFU(...)                     \
    do                                   \
    {                                    \
        log_debug("[DFU] " __VA_ARGS__); \
    } while (0)
#else
#define LOG_DFU(...)
#endif

/* PRQA S 1504 ++ #3220 -  Object used only in local translation unit, intentional design */
/* PRQA S 3408 ++ #3218 -External linkage function defined without prior declaration, intentional design */
/* PRQA S 1514 4 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
/* PRQA S 2071 4 #3269 - Language extension used for compiler and hardware optimization */
/* PRQA S 1502 2 #3216 - Unused parameter is part of standard callback prototype */
__attribute__((section(".ARM.__at_0x00003900"))) const uint8_t boot_version[8u] = {(uint8_t)'B', (uint8_t)'T', (uint8_t)':', (uint8_t)'B', (uint8_t)'.', (uint8_t)'0', (uint8_t)'4', 0x20u};
__attribute__((section(".ARM.__at_0x00003908"))) const uint8_t hardware_version[8u] = {(uint8_t)'H', (uint8_t)'W', (uint8_t)':', (uint8_t)'B', (uint8_t)'.', (uint8_t)'2', (uint8_t)'.', (uint8_t)'0'};
__attribute__((section(".ARM.__at_0x10000000"))) uint8_t flash_driver[66u] = {0u};

/* PRQA S 1514 30 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
uint8_t seed[16u] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
uint8_t key[16u] = {0x88, 0xB3, 0x4F, 0x45, 0xE1, 0x0D, 0xBB, 0xC3, 0x5D, 0xBF, 0x7E, 0xCF, 0x86, 0x8E, 0x73, 0x60};

uint8_t seed_use[16u] = {0u};
uint8_t flash_driver_cmac[16u] = {0u};
uint8_t app_cmac[16u] = {0u};
uint8_t g_config_word_state;
user_cfg_t g_user_info = {0u};
ota_cfg_t g_ota_info = {0u};
/* PRQA S 1502 1 #3216 - Unused parameter is part of standard callback prototype */
uint8_t g_negResponseCode;
uint8_t diagnosticTxBuffer[CUS_UDS_SEND_BUFFER_SIZE];
dfu_manager_context_t dfu_ctx = {0u};
uint8_t lin_configured_NAD = 0x68u;
volatile uint16_t timer_1s_cnt = 0u;
uint16_t lock_failed_cnt = 0u;

ServiceUDS_TypeDef uds_request_info =
    {
        .sessionMode = DEFALUT_SESSION,
};

uint8_t seed_cmac_succ = AS_FALSE;
uint8_t flashdrv_cmac_succ = AS_FALSE;
uint8_t app_cmac_succ = AS_FALSE;
uint8_t seed_00_ret = AS_FALSE;
uint8_t app_cmac_start = AS_FALSE;
uint8_t unlock_failed_store_flag = AS_FALSE;
uint8_t diagnostic_session_overtime_flag = AS_FALSE;
/* PRQA S 3408 -- */
/* PRQA S 1504 -- */

extern void lin_lld_isr_callback(uint32_t isr);

/********************************************************
** @brief   跳转到用户应用程序
**
** @note    跳转前禁用 TIMER、LINSCI 和 SysTick 中断，
**          并取消 LIN 总线初始化。从 FLASH_APP_ADDR + 0x100
**          处读取应用程序入口地址并跳转。
**
** @retval  无
*********************************************************/
STATIC void JumpToApp(void)
{
    NVIC_DisableIRQ(TIMER_IRQn);
    NVIC_DisableIRQ(LINSCI_IRQn);
    NVIC_DisableIRQ(SysTick_IRQn);
    pal_lin_deinit(LIN_BUS_0);
#ifdef CFG_SUPPORT_DEBUG
    logging_deinit();
#endif

#ifdef ENABLE_TEST_MODE

#else
    /* PRQA S 0306 3 #3271 - Cast between object pointer and integer for hardware address access */
    /* PRQA S 0305 1 #3270 - Cast between function pointer and integer for system/boot operations */
    FUNC_PTR pAppFunc = (FUNC_PTR) * (uint32_t *)(FLASH_APP_ADDR + 0x100u + 4u); /* The APP header 0x100 is sized for user data*/
    __set_MSP(*(uint32_t *)(FLASH_APP_ADDR + 0x100u));
    pAppFunc();
#endif
}

/********************************************************
** @brief   检查 LIN 队列是否为空
**
** @retval  uint8_t  队列为空返回 1，否则返回 0
*********************************************************/
/* PRQA S 3219 1 #3254 - Unused static function, reserved for future extension */
STATIC uint8_t queue_lin_empty(void)
{
    return (uint8_t)((dfu_ctx.queue_list.head == dfu_ctx.queue_list.tail) ? 1 : 0);
}

/********************************************************
** @brief   擦除 DFU 固件存储区域
**
** @note    擦除范围包括 DFU 信息区域和应用程序镜像区域，
**          总大小为 FLASH_DFU_INFO_SIZE + FLASH_APP_IMAGE_SIZE。
**
** @retval  uint8_t  擦除成功返回 DFU_MSG_SUCCESS，失败返回 DFU_MSG_ERASE_ERROR
*********************************************************/
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC uint8_t dfu_image_erase(void)
{
    if (ll_flash_erase_drv(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, FLASH_DFU_INFO_SIZE + FLASH_APP_IMAGE_SIZE) == 0)
    {
        return (uint8_t)DFU_MSG_SUCCESS;
    }

    return (uint8_t)DFU_MSG_ERASE_ERROR;
}

/********************************************************
** @brief   编程固件数据到应用程序存储区
**
** @param   addr    目标写入地址
** @param   data    待写入数据缓冲区指针
** @param   length  待写入数据长度
**
** @retval  uint8_t  编程成功返回 DFU_MSG_SUCCESS，地址超出范围返回 DFU_MSG_PROGRA_ERROR
*********************************************************/
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC uint8_t dfu_image_program(uint32_t addr, uint8_t *data, uint16_t length)
{
    if ((addr < FLASH_APP_ADDR) || (addr >= FLASH_APP_END_ADDR))
    {
        return (uint8_t)DFU_MSG_PROGRA_ERROR;
    }

    uint8_t res = (uint8_t)DFU_MSG_SUCCESS;
    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    ll_flash_write_drv(FLASH_TYPE_NVM, addr, data, length);

    return (res);
}
/********************************************************
** @brief   发送 UDS 正响应到 LIN 总线（带子功能参数）
**
** @param   sid       服务标识符
** @param   sub_func  子功能参数
** @param   data      附加响应数据缓冲区指针
** @param   length    附加响应数据长度
**
** @retval  无
*********************************************************/
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
STATIC void dfu_do_notify_cp(uint8_t sid, uint8_t sub_func, uint8_t *data, uint16_t length)
{
    uint8_t response[20u];
    uint8_t len = 2u + (uint8_t)length;

    response[0u] = sid + 0x40u;
    response[1u] = sub_func;

    for (uint16_t i = 0u; i < length; i++)
    {
        response[2u + i] = data[i];
    }

    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    lin_uds_send(lin_configured_NAD, response, len);
}

/********************************************************
** @brief   发送 UDS 正响应到 LIN 总线（不含子功能参数）
**
** @param   sid     服务标识符
** @param   data    附加响应数据缓冲区指针
** @param   length  附加响应数据长度
**
** @retval  无
*********************************************************/
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
STATIC void dfu_do_notify_cp_ex(uint8_t sid, uint8_t *data, uint16_t length)
{
    uint8_t response[40u];
    uint8_t len = 1u + (uint8_t)length;

    response[0u] = sid + 0x40u;

    for (uint16_t i = 0u; i < length; i++)
    {
        response[1u + i] = data[i];
    }

    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    lin_uds_send(lin_configured_NAD, response, len);
}

/********************************************************
** @brief   发送 UDS 响应（正响应或负响应）
**
** @param   resp_type   响应类型（POSITIVE / NEGATIVE）
** @param   sid         服务标识符
** @param   resp_value  响应值（正响应时为子功能，负响应时为 NRC 码）
**
** @retval  无
*********************************************************/
STATIC void dfu_do_notify_response(uint8_t resp_type, uint8_t sid, uint8_t resp_value)
{
    if ((uint8_t)POSITIVE == resp_type)
    {
        dfu_do_notify_cp(sid, resp_value, NULL, 0u);
    }
    else
    {
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        lin_uds_negative_response(lin_configured_NAD, sid, resp_value);
    }
}

/********************************************************
** @brief   发送会话控制响应（SID = 0x10）
**
** @param   sessiontype  会话类型
**
** @note    响应中包含 P2 和 P2E 超时参数。
**
** @retval  无
*********************************************************/
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void dfu_session_parameter_resp(uint8_t sessiontype)
{
    uint8_t session_parameter[4u];
    session_parameter[0u] = (P2_SERVER_MAX >> 8u) & 0xFFu;
    session_parameter[1u] = (P2_SERVER_MAX & 0xFFu);
    session_parameter[2u] = ((uint16_t)P2E_SERVER_MAX >> 8u) & 0xFFu;
    session_parameter[3u] = (P2E_SERVER_MAX & 0xFFu);
    dfu_do_notify_cp(0x10u, sessiontype, session_parameter, sizeof(session_parameter));
}

/********************************************************
** @brief   获取上次 DFU 升级信息
**
** @note    从 Flash 中读取 DFU 信息并检查魔数有效性。
**          如果 APP 请求扩展编程标志有效，则切换到编程会话。
**          如果魔数无效，返回错误。
**
** @retval  uint8_t  成功返回 DFU_MSG_SUCCESS，失败返回 DFU_MSG_ERROR
*********************************************************/
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC uint8_t last_dfu_info_get(void)
{
    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    pal_store_read(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)&dfu_ctx.dfu_info, sizeof(last_dfu_info_t));

    if (DFU_INFO_MAGIC != dfu_ctx.dfu_info.magic)
    {
        return (uint8_t)DFU_MSG_ERROR;
    }
    if ((g_ota_info.app_req_ext_program_flag == 0x01u) || (g_ota_info.app_req_ext_program_flag == 0x03u))
    {
        dfu_ctx.op_code = (uint8_t)DFU_CMD_PROGRAM_SESSION;
        uds_request_info.sessionMode = PROGRAM_SESSION;
        if (g_ota_info.app_req_ext_program_flag == 0x01u)
        {
            dfu_session_parameter_resp(0x02u);
            delay_ms(1u);
        }
        g_ota_info.app_req_ext_program_flag = 0x00u;
        pal_store_data_set(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
        return (uint8_t)DFU_MSG_ERROR;
    }

    return (uint8_t)DFU_MSG_SUCCESS;
}

/********************************************************
** @brief   检查 UDS 数据标识符（DID）是否有效
**
** @param   ucSess  DID 值
**
** @note    0xF189（序列号）和 0x0216（软件版本）需要 APP
**          标志有效才能访问。
**
** @retval  1   DID 有效
** @retval  0   DID 无效
*********************************************************/
STATIC uint8_t uds_diag_DID_chk(uint16_t ucSess)
{
    uint8_t ucRet;
    switch (ucSess)
    {
    /* Seres part num:6106150-RQ01 */
    case 0xF187u:
    /* Seres Supplier code:3233 */
    case 0xF18Au:
    /* Seres ECU name:EHIS_FL */
    case 0xF197u:
    case 0xF089u:
    case 0xF180u:
    case 0xF184u:
    case 0xF190u:
        ucRet = 1u;
        break;
    case 0xF189u:
    case 0x0216u:
        // Determine whether the APP flag is valid. If it is invalid, it indicates that the APP does not exist, and respond with NRC31.
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        pal_store_read(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)&dfu_ctx.dfu_info, sizeof(last_dfu_info_t));
        if (DFU_INFO_MAGIC == dfu_ctx.dfu_info.magic)
        {
            ucRet = 1u;
        }
        else
        {
            ucRet = 0u;
        }
        break;
    default:
        ucRet = 0u;
        break;
    }
    return ucRet;
}

/********************************************************
** @brief   根据配置字重新映射 LIN NAD 地址
**
** @note    从用户配置信息中读取 NAD 值，验证其合法性后更新
**          LIN 通信使用的 NAD 地址。同时校验锁定失败计数器的合理性。
**
** @retval  无
*********************************************************/
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void uds_diagnostic_configword_remap_nad(void)
{
    uint8_t nad_temp = g_user_info.nad_info;

    if ((nad_temp == 0x68u) || (nad_temp == 0x6Au) ||
        (nad_temp == 0x69u) || (nad_temp == 0x6Bu))
    {
        lin_configured_NAD = nad_temp;
    }
    else
    {
        lin_configured_NAD = 0x68;
    }

    if (g_ota_info.lock_failed_index > 3u) /* Parameter Rationality Check */
    {
        g_ota_info.lock_failed_index = 0u;
    }
    LOG_DFU("lock index =%d\r\n", g_ota_info.lock_failed_index);
}

/********************************************************
** @brief   更新上次 DFU 升级信息到 Flash
**
** @param   info  指向 DFU 信息结构体的指针
**
** @retval  uint8_t  始终返回 DFU_MSG_SUCCESS
*********************************************************/
STATIC uint8_t last_dfu_info_update(last_dfu_info_t *info)
{
    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    ll_flash_write_drv(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)info, sizeof(last_dfu_info_t));
    return (uint8_t)(DFU_MSG_SUCCESS);
}

/********************************************************
** @brief   DFU 处理流程退出
**
** @param   reason  退出原因（成功时更新 DFU 信息标志）
**
** @note    如果退出原因为成功，则在 Flash 中更新 DFU 信息
**          并写入魔数以标记 APP 有效。
**
** @retval  无
*********************************************************/
STATIC void dfu_process_exit(uint8_t reason)
{
    if ((uint8_t)DFU_MSG_SUCCESS == reason)
    {
        dfu_ctx.dfu_info.magic = DFU_INFO_MAGIC;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        last_dfu_info_update(&dfu_ctx.dfu_info);
    }
}
/********************************************************
** @brief   处理 UDS 会话控制服务（SID = 0x10）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    支持默认会话、扩展会话和编程会话的切换。
**          编程会话需要先通过例程预编程检查。
**          根据不同的 NAD 地址和子功能决定是否发送响应。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
STATIC void session_control_handle(uint8_t *param, uint16_t length)
{
    if (length == BOOT_Frame_length_2)
    {
        switch (param[1u])
        {
        case 0x01u:
        case 0x81u:
            if (uds_request_info.sessionMode == PROGRAM_SESSION)
            {
                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                pal_store_read(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)&dfu_ctx.dfu_info, sizeof(last_dfu_info_t));
                if (DFU_INFO_MAGIC == dfu_ctx.dfu_info.magic) // Reset is only allowed when the APP flag bit is valid; otherwise, it stays in the BootLoader.
                {
                    if (param[1] == 0x81u) // Inhibit positive response or function-addressed direct reset
                    {
                        NVIC_SystemReset();
                    }
                    else if (lin_get_uds_nad() == 0x7Eu)
                    {
                        NVIC_SystemReset();
                    }
                    else if (lin_get_uds_nad() == 0x7Fu)
                    {
                        NVIC_SystemReset();
                    }
                    else // 01 is a record flag, used for APP response 50 01
                    {
                        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                        pal_store_data_get(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
                        if (g_ota_info.app_need_res_flag != 1U)
                        {
                            g_ota_info.app_need_res_flag = 1U;
                            pal_store_data_set(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
                        }
                        delay_ms(5u);
                        NVIC_SystemReset();
                    }
                }
                else
                {
                    dfu_ctx.op_code = (uint8_t)DFU_CMD_DEFAULT_SESSION;
                    uds_request_info.sessionMode = DEFALUT_SESSION;
                    seed_cmac_succ = AS_FALSE;
                    seed_00_ret = AS_FALSE;
                }
            }
            else
            {
                dfu_ctx.op_code = (uint8_t)DFU_CMD_DEFAULT_SESSION;
                uds_request_info.sessionMode = DEFALUT_SESSION;
                seed_cmac_succ = AS_FALSE;
                seed_00_ret = AS_FALSE;
            }
            if ((uds_request_info.sessionMode == DEFALUT_SESSION) && (param[1u] == 0x01u))
            {
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_session_parameter_resp(param[1u]);
                }
            }
            break;
        case 0x03u:
            if (uds_request_info.sessionMode != PROGRAM_SESSION)
            {
                uds_request_info.sessionMode = EXTEND_SESSION;
                dfu_ctx.op_code = (uint8_t)DFU_CMD_EXTEND_SESSION;
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_session_parameter_resp(param[1u]);
                }
            }
            else
            {
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOTSUPPORTED_INACTIVESESSION); // NRC7E
                }
            }
            seed_cmac_succ = AS_FALSE;
            seed_00_ret = AS_FALSE;
            break;
        case 0x83u:
            uds_request_info.sessionMode = EXTEND_SESSION;
            dfu_ctx.op_code = (uint8_t)DFU_CMD_EXTEND_SESSION;
            seed_cmac_succ = AS_FALSE;
            seed_00_ret = AS_FALSE;
            break;
        case 0x02u:
        case 0x82u:
            if (dfu_ctx.op_code >= (uint8_t)DFU_CMD_ROUTINE_PROGRAM_CHECK)
            {
                dfu_ctx.op_code = (uint8_t)DFU_CMD_PROGRAM_SESSION;
                uds_request_info.sessionMode = PROGRAM_SESSION;
                dfu_ctx.boot_state = (uint8_t)BOOT_STATE_UPGRADE;
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_session_parameter_resp(0x02u);
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], CONDITION_NOT_CORRECT); // NRC22
            }
            seed_cmac_succ = AS_FALSE;
            seed_00_ret = AS_FALSE;
            break;
        default:
            if (lin_get_uds_nad() == 0x7Eu)
            {
            }
            else if (lin_get_uds_nad() == 0x7Fu)
            {
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
            }
            break;
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/********************************************************
** @brief   比较两个密钥/种子值是否相等
**
** @param   _seed   种子值缓冲区指针
** @param   _key    密钥值缓冲区指针
** @param   length  比较长度（字节数）
**
** @retval  1   比较相等
** @retval  0   比较不相等
*********************************************************/
/* PRQA S 3673 3 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC uint8_t cpmpare_key(uint8_t *_seed, uint8_t *_key, uint8_t length)
{
    /*compare key seed*/
    for (uint8_t i = 0u; i < length; i++)
    {
        if (_seed[i] != _key[i])
        {
            return 0u;
        }
    }
    return 1u;
}

/********************************************************
** @brief   根据配置的 NAD 地址重新设置密钥
**
** @note    根据 g_user_info.nad_info 的值选择对应的密钥：
**          - 0x68：左前（key_lf）
**          - 0x69：右前（key_rf）
**          - 0x6A：左后（key_lr）
**          - 0x6B：右后（key_rr）
**
** @retval  无
*********************************************************/
STATIC void key_reset_by_nad(void)
{
    const uint8_t key_lf[16] = {
        0x88, 0xB3, 0x4F, 0x45,
        0xE1, 0x0D, 0xBB, 0xC3,
        0x5D, 0xBF, 0x7E, 0xCF,
        0x86, 0x8E, 0x73, 0x60};
    const uint8_t key_rf[16] = {
        0x32, 0xA9, 0x83, 0x03,
        0xAD, 0x07, 0xAE, 0x9C,
        0x2B, 0x46, 0x1F, 0xDE,
        0x1D, 0xA5, 0x46, 0x76};
    const uint8_t key_lr[16] = {
        0x48, 0x54, 0x24, 0xF9,
        0xF5, 0x76, 0x3E, 0x2B,
        0x99, 0x87, 0xD0, 0x11,
        0x1A, 0x8B, 0xD2, 0x82};
    const uint8_t key_rr[16] = {
        0x2A, 0x7E, 0xF5, 0x25,
        0x7E, 0x01, 0xE6, 0x06,
        0xCD, 0x0C, 0x68, 0xFE,
        0xA0, 0x3E, 0x5E, 0x5C};

    if (g_user_info.nad_info == 0x68U)
    {
        /* PRQA S 3200 ++ #3264 - Return value ignored, verified safe for system operation */
        #ifdef ENABLE_TEST_MODE
        #else
            memcpy(key, key_lf, sizeof(key_lf));
        #endif
    }
    else if (g_user_info.nad_info == 0x69U)
    {
        #ifdef ENABLE_TEST_MODE
        #else
            memcpy(key, key_rf, sizeof(key_rf));
        #endif
    }
    else if (g_user_info.nad_info == 0x6AU)
    {
        #ifdef ENABLE_TEST_MODE
        #else
            memcpy(key, key_lr, sizeof(key_lr));
        #endif
    }
    else if (g_user_info.nad_info == 0x6BU)
    {
        #ifdef ENABLE_TEST_MODE
        #else
            memcpy(key, key_rr, sizeof(key_rr));
        #endif
/* PRQA S 3200 -- */
    }
    else
    {
        (void)0;
    }
}

/********************************************************
** @brief   处理 UDS 安全访问服务（SID = 0x27）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    支持种子请求（子功能 0x01/0x09/0x89）和
**          密钥验证（子功能 0x0A）。使用 AES-CMAC 算法进行
**          密钥验证。连续 3 次解锁失败后将锁定 10 秒。
**
** @retval  无
*********************************************************/
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void security_access_handle(uint8_t *param, uint16_t length)
{
    uint8_t out[16u];
    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if ((length == BOOT_Frame_length_2) || (length == BOOT_Frame_length_18))
    {
        if (uds_request_info.sessionMode == PROGRAM_SESSION)
        {
            switch (param[1u])
            {
            case 0x89u:
            case 0x09u:
                if (length == BOOT_Frame_length_2)
                {
                    if (g_ota_info.lock_failed_index < 3u)
                    {
                        if (seed_cmac_succ == AS_TRUE)
                        {
                            if (seed_00_ret == AS_TRUE)
                            {
                                seed_cmac_succ = AS_FALSE;
                                seed_00_ret = AS_FALSE;
                                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_SEQUEENCE_ERROR); // NRC24
                            }
                            else
                            {
                                for (uint8_t i = 0u; i < 16u; i++)
                                {
                                    seed_use[i] = 0u;
                                }
                                dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)seed_use, sizeof(seed_use));
                                seed_00_ret = AS_TRUE;
                            }
                        }
                        else
                        {
                            if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_PROGRAM_SESSION) || (dfu_ctx.op_code == (uint8_t)DFU_CMD_SECURITY_SEED_REQUEST))
                            {
                                for (uint8_t i = 0u; i < 16u; i++)
                                {
                                    seed_use[i] = seed[i];
                                }
                                dfu_ctx.op_code = (uint8_t)DFU_CMD_SECURITY_SEED_REQUEST;

                                dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)seed_use, sizeof(seed_use));
                            }
                            else
                            {
                                seed_cmac_succ = AS_FALSE;
                                seed_00_ret = AS_FALSE;
                                dfu_do_notify_response(NEGATIVE, param[0u], CONDITION_NOT_CORRECT); // NRC22
                            }
                        }
                    }
                    else
                    {
                        seed_cmac_succ = AS_FALSE;
                        seed_00_ret = AS_FALSE;
                        dfu_ctx.op_code = (uint8_t)DFU_CMD_PROGRAM_SESSION;
                        dfu_do_notify_response(NEGATIVE, param[0u], REQUIREDTIMEDELAY_NOTEXPIRED); // NRC37
                    }
                }
                else
                {
                    seed_cmac_succ = AS_FALSE;
                    seed_00_ret = AS_FALSE;
                    dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
                }
                break;
            case 0x0au:
                if (length == BOOT_Frame_length_18)
                {
                    if (dfu_ctx.op_code == (uint8_t)DFU_CMD_SECURITY_SEED_REQUEST)
                    {
                        key_reset_by_nad();
                        aes_cmac(key, seed_use, sizeof(seed_use), (uint8_t *)out);
                        if (cpmpare_key(out, &param[2u], 16u) == 1u)
                        {
                            dfu_ctx.op_code = (uint8_t)DFU_CMD_SECURITY_KEY_CHECK;
                            seed_cmac_succ = AS_TRUE;
                            // Unlock successful, clear the counter of 27 unlock failures
                            /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                            pal_store_data_get(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
                            g_ota_info.lock_failed_index = 0u;
                            pal_store_data_set(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
                            dfu_do_notify_response(POSITIVE, param[0u], param[1u]);
                        }
                        else
                        {
                            dfu_ctx.op_code = (uint8_t)DFU_CMD_PROGRAM_SESSION;
                            if (g_ota_info.lock_failed_index < 3u)
                            {
                                //  Unlock failed, the counter for 27 unlock failures is stored in flash
                                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                                pal_store_data_get(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
                                g_ota_info.lock_failed_index++;
                            }
                            if (g_ota_info.lock_failed_index <= 2u)
                            {
                                seed_cmac_succ = AS_FALSE;
                                seed_00_ret = AS_FALSE;
                                dfu_do_notify_response(NEGATIVE, param[0], INVALID_KEY); // NRC35
                            }
                            else if (g_ota_info.lock_failed_index == 3u)
                            {
                                seed_cmac_succ = AS_FALSE;
                                seed_00_ret = AS_FALSE;
                                dfu_do_notify_response(NEGATIVE, param[0u], ENOA); // NRC36
                            }
                            else
                            {
                                seed_cmac_succ = AS_FALSE;
                                seed_00_ret = AS_FALSE;
                                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_SEQUEENCE_ERROR); // NRC24
                                break;
                            }
                            pal_store_data_set(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
                        }
                    }
                    else
                    {
                        seed_cmac_succ = AS_FALSE;
                        seed_00_ret = AS_FALSE;
                        dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_SEQUEENCE_ERROR); // NRC24
                    }
                }
                else
                {
                    seed_cmac_succ = AS_FALSE;
                    seed_00_ret = AS_FALSE;
                    dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
                }
                break;
            default:
                seed_cmac_succ = AS_FALSE;
                seed_00_ret = AS_FALSE;
                dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
                break;
            }
        }
        else
        {
            seed_cmac_succ = AS_FALSE;
            seed_00_ret = AS_FALSE;
            dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
        }
    }
    else
    {
        seed_cmac_succ = AS_FALSE;
        seed_00_ret = AS_FALSE;
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/********************************************************
** @brief   处理固件信息同步服务（SID = 0x2E）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    用于写入指纹信息（DID 0xF184），需要在安全访问
**          通过后才能操作。
**
** @retval  无
*********************************************************/
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void firmware_info_sync_handle(uint8_t *param, uint16_t length)
{
    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if (length > BOOT_Frame_length_3)
    {
        if (uds_request_info.sessionMode == PROGRAM_SESSION)
        {
            if ((param[1u] == 0xF1u) && (param[2u] == 0x84u))
            {
                if (length == BOOT_Frame_length_13)
                {
                    if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_SECURITY_KEY_CHECK) && (seed_cmac_succ == AS_TRUE))
                    {
                        dfu_ctx.op_code = (uint8_t)DFU_CMD_WRITE_FINGER;
                        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                        memcpy(dfu_ctx.dfu_info.fingerprint, &param[3u], 10u);
                        dfu_do_notify_cp(param[0u], param[1u], &param[2u], 1u);
                    }
                    else
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], SECURITY_ACCESS_DENIED); // NRC33
                    }
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
            }
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/********************************************************
** @brief   处理请求下载服务（SID = 0x34）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    处理 Flash 驱动下载和应用程序下载的请求。
**          根据当前操作码（op_code）判断是下载 Flash 驱动
**          还是应用程序镜像，并初始化相应传输上下文。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void request_download_handle(uint8_t *param, uint16_t length)
{
    uint8_t req_down_resp[2u];
    uint32_t req_addr = 0u;
    uint32_t req_size = 0u;
    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if (uds_request_info.sessionMode == PROGRAM_SESSION)
    {
        if (length == BOOT_Frame_length_11)
        {
            if (seed_cmac_succ == AS_TRUE)
            {
                if (param[1] == 0x00u)
                {
                    if (param[2] == 0x44u)
                    {
                        req_addr = ((uint32_t)param[3u] << 24u) | ((uint32_t)param[4u] << 16u) | ((uint32_t)param[5u] << 8u) | param[6u];
                        req_size = ((uint32_t)param[7u] << 24u) | ((uint32_t)param[8u] << 16u) | ((uint32_t)param[9u] << 8u) | param[10u];
                        if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_WRITE_FINGER) || (dfu_ctx.op_code == (uint8_t)DFU_CMD_SECURITY_KEY_CHECK))
                        {
                            if ((req_addr == FLASH_DRIVER_ADDR) && (req_size == FLASH_DRIVER_LENGTH))
                            {
                                dfu_ctx.flashdrv_write_addr = req_addr;
                                dfu_ctx.flashdrv_write_size = (uint8_t)req_size;
                                dfu_ctx.op_code = (uint8_t)DFU_CMD_FLASH_DRIVER_REQUEST;

                                dfu_ctx.write_length = 0u;
                                dfu_ctx.receive_length = 0u;

                                dfu_ctx.queue_list.tail = 0u;
                                dfu_ctx.queue_list.head = 0u;
                                dfu_ctx.recevice_index = 0x01u;
                                dfu_ctx.write_index = 0u;

                                req_down_resp[0] = 0x02u;
                                req_down_resp[1] = 0x02u;
                                dfu_do_notify_cp(param[0u], 0x20u, req_down_resp, sizeof(req_down_resp));
                                LOG_DFU("add=0x%x, size=0x%x\n", req_addr, req_size);
                            }
                            else
                            {
                                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
                            }
                        }
                        else if (dfu_ctx.op_code == (uint8_t)DFU_CMD_APP_ERASE)
                        {
                            dfu_ctx.write_addr = req_addr;
                            dfu_ctx.dfu_info.image_size = req_size;
                            if (FLASH_APP_ADDR == dfu_ctx.write_addr)
                            {
                                dfu_ctx.op_code = (uint8_t)DFU_CMD_APP_REQUEST;

                                dfu_ctx.write_length = 0u;
                                dfu_ctx.receive_length = 0u;

                                dfu_ctx.queue_list.tail = 0u;
                                dfu_ctx.queue_list.head = 0u;
                                dfu_ctx.recevice_index = 0x01u;
                                dfu_ctx.write_index = 0u;

                                req_down_resp[0] = 0x02u;
                                req_down_resp[1] = 0x02u;
                                dfu_do_notify_cp(param[0u], 0x20u, req_down_resp, sizeof(req_down_resp));
                                LOG_DFU("add=0x%x, size=0x%x\n", req_addr, req_size);
                            }
                            else
                            {
                                LOG_DFU("nrc31=0x%x, size=0x%x\n", req_addr, req_size);
                                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
                            }
                        }
                        else
                        {
                            dfu_do_notify_response(NEGATIVE, param[0u], DOWNLOAD_REJECTED); // NRC70
                        }
                    }
                    else
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
                    }
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SECURITY_ACCESS_DENIED); // NRC33
            }
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
    }
}

/********************************************************
** @brief   处理数据传输服务（SID = 0x36）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    支持 Flash 驱动数据和应用程序数据的传输。
**          使用队列机制管理接收数据块索引，当接收长度达到
**          DFU_PROGRAM_LENGTH 或镜像大小时进行流控。
**
** @retval  无
*********************************************************/
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void transfer_data_handle(uint8_t *param, uint16_t length)
{
    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if (uds_request_info.sessionMode == PROGRAM_SESSION)
    {
        if (length < 0x3u)
        {
            dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
        }
        else if (length > 0x202u)
        {
            dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
        }
        else
        {
            if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_FLASH_DRIVER_REQUEST) || (dfu_ctx.op_code == (uint8_t)DFU_CMD_FLASH_DRIVER_TRANSFER))
            {
                dfu_ctx.op_code = (uint8_t)DFU_CMD_FLASH_DRIVER_TRANSFER;
                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                memcpy(flash_driver, &param[2u], dfu_ctx.flashdrv_write_size);
                LOG_DFU("drv len=%d\n", dfu_ctx.flashdrv_write_size);
                dfu_do_notify_response(POSITIVE, param[0u], param[1u]);
            }
            else if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_APP_REQUEST) || (dfu_ctx.op_code == (uint8_t)DFU_CMD_APP_TRANSFER))
            {
                dfu_ctx.op_code = (uint8_t)DFU_CMD_APP_TRANSFER;
                if ((param[1u] == 0u) || (param[1u] != (dfu_ctx.write_index + 1u)))
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], BLOCK_SEQUENCE_COUNT_ERR); // NRC73
                    return;
                }
                dfu_ctx.recevice_index = param[1u];
                dfu_ctx.receive_length += ((uint32_t)length - 2u);
                dfu_ctx.program_flag = 1u;
                if ((uint8_t)DFU_MSG_SUCCESS == dfu_image_program(dfu_ctx.write_addr, &param[2u], length - 2u))
                {
                    /* PRQA S 3440 1 #3221 - Use of increment or decrement result is safe and required by logic */
                    if ((++(dfu_ctx.queue_list.head)) >= QUEUE_LIN_LEN)
                    {
                        dfu_ctx.queue_list.head = 0u;
                    }

                    dfu_ctx.write_index = dfu_ctx.recevice_index;

                    dfu_ctx.write_addr += ((uint32_t)length - 2u);
                    dfu_ctx.write_length += ((uint32_t)length - 2u);
                    dfu_do_notify_response(POSITIVE, param[0u], param[1u]);
                }
                else
                {
                    dfu_ctx.queue_list.tail = 0u;
                    dfu_ctx.queue_list.head = 0u;
                    dfu_do_notify_response(NEGATIVE, param[0u], GENERAL_PROGRAM_FAILURE); // NRC72
                    LOG_DFU("write error\r\n");
                }

                if ((dfu_ctx.receive_length >= DFU_PROGRAM_LENGTH) ||
                    ((dfu_ctx.receive_length + dfu_ctx.write_length) == dfu_ctx.dfu_info.image_size))
                {
                    /* PRQA S 3440 1 #3221 - Use of increment or decrement result is safe and required by logic */
                    if ((++(dfu_ctx.queue_list.tail)) >= QUEUE_LIN_LEN)
                    {
                        dfu_ctx.queue_list.tail = 0u;
                    }
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], TRANSFER_DATA_PAUSE); // NRC71
                }

                if (dfu_ctx.write_length == dfu_ctx.dfu_info.image_size) // Avoid excessive sending
                {
                    dfu_ctx.queue_list.tail = 0u; // Queue cleared
                    dfu_ctx.queue_list.head = 0u;
                }
                LOG_DFU("app index=%d len=%d\n", dfu_ctx.recevice_index, length);
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_SEQUEENCE_ERROR); // NRC24
            }
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
    }
}

/********************************************************
** @brief   处理请求传输退出服务（SID = 0x37）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    根据当前操作码确认退出 Flash 驱动传输或
**          应用程序传输阶段。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void request_transfer_exit_handle(uint8_t *param, uint16_t length)
{
    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if (uds_request_info.sessionMode == PROGRAM_SESSION)
    {
        if (length == BOOT_Frame_length_1)
        {
            if (dfu_ctx.op_code == (uint8_t)DFU_CMD_FLASH_DRIVER_TRANSFER)
            {
                dfu_ctx.op_code = (uint8_t)DFU_CMD_FLASH_DRIVER_EXIT;
                dfu_do_notify_cp_ex(param[0u], NULL, 0u);
            }
            else if (dfu_ctx.op_code == (uint8_t)DFU_CMD_APP_TRANSFER)
            {
                dfu_ctx.op_code = (uint8_t)DFU_CMD_APP_EXIT;
                dfu_do_notify_cp_ex(param[0u], NULL, 0u);
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_SEQUEENCE_ERROR); // NRC24
            }
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
    }
}

/********************************************************
** @brief   处理例程控制服务（SID = 0x31）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    支持以下例程：
**          - 0x0203：预编程检查（扩展会话）
**          - 0xDD02：签名验证（Flash 驱动/应用程序 CMAC 校验）
**          - 0xFF00：擦除 Flash
**          - 0xFF01：应用程序兼容性检查
**
** @retval  无
*********************************************************/
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void routine_control_handle(uint8_t *param, uint16_t length)
{
    uint8_t pre_program_resp[3u] = {0u};
    uint8_t data_tmp[32u] = {0u};

    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if ((0x01u == param[1u]) || (0x81u == param[1u]))
    {
        uint16_t routine_id = ((uint16_t)param[2u] << 8u) | param[3u];
        param[1u] = 0x01u;
        switch (routine_id)
        {
        case 0x0203u:
            if (uds_request_info.sessionMode == EXTEND_SESSION)
            {
                if (length == BOOT_Frame_length_4)
                {
                    if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_EXTEND_SESSION) || (dfu_ctx.op_code == (uint8_t)DFU_CMD_ROUTINE_PROGRAM_CHECK))
                    {
                        dfu_ctx.op_code = (uint8_t)DFU_CMD_ROUTINE_PROGRAM_CHECK;
                        pre_program_resp[0u] = param[2u];
                        pre_program_resp[1u] = param[3u];
                        pre_program_resp[2u] = 0u;
                        dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, sizeof(pre_program_resp));
                    }
                    else
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], CONDITION_NOT_CORRECT); // NRC22
                    }
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
            }
            break;
        case 0xDD02u: // Signature verification
            if (uds_request_info.sessionMode == PROGRAM_SESSION)
            {
                if (length == BOOT_Frame_length_20)
                {
                    if (dfu_ctx.op_code == (uint8_t)DFU_CMD_FLASH_DRIVER_EXIT)
                    {
                        sha256(flash_driver, dfu_ctx.flashdrv_write_size, data_tmp);
                        aes_cmac(key, data_tmp, 32, (uint8_t *)flash_driver_cmac);

                        if (cpmpare_key(flash_driver_cmac, &param[4u], 16u) == 1u)
                        {
                            dfu_ctx.op_code = (uint8_t)DFU_CMD_FLASH_DRIVER_CMAC_CHECK;
                            flashdrv_cmac_succ = AS_TRUE;
                            pre_program_resp[0u] = param[2u];
                            pre_program_resp[1u] = param[3u];
                            pre_program_resp[2u] = 0u;
                            dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, sizeof(pre_program_resp));
                        }
                        else
                        {
                            flashdrv_cmac_succ = AS_FALSE;
                            pre_program_resp[0u] = param[2u];
                            pre_program_resp[1u] = param[3u];
                            pre_program_resp[2u] = 0x2u;
                            dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, sizeof(pre_program_resp));
                        }
                    }
                    else if (dfu_ctx.op_code == (uint8_t)DFU_CMD_APP_EXIT)
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], RCRRP); // NRC78
                        timer_1s_cnt = 0u;
                        app_cmac_start = AS_TRUE;
                        /* PRQA S 0306 1 #3271 - Cast between object pointer and integer for hardware address access */
                        sha256((uint8_t *)FLASH_APP_ADDR, dfu_ctx.dfu_info.image_size, data_tmp);
                        aes_cmac(key, data_tmp, 32, (uint8_t *)app_cmac);

                        app_cmac_start = AS_FALSE;
                        if (cpmpare_key(app_cmac, &param[4u], 16u) == 1u)
                        {
                            dfu_ctx.op_code = (uint8_t)DFU_CMD_APP_CMAC_CHECK;
                            app_cmac_succ = AS_TRUE;
                            pre_program_resp[0u] = param[2u];
                            pre_program_resp[1u] = param[3u];
                            pre_program_resp[2u] = 0u;
                            dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, sizeof(pre_program_resp));
                        }
                        else
                        {
                            app_cmac_succ = AS_FALSE;
                            pre_program_resp[0u] = param[2u];
                            pre_program_resp[1u] = param[3u];
                            pre_program_resp[2u] = 0x2u;
                            dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, sizeof(pre_program_resp));
                        }
                    }
                    else
                    {
                        pre_program_resp[0u] = param[2u];
                        pre_program_resp[1u] = param[3u];
                        pre_program_resp[2u] = 0x2u;
                        dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, sizeof(pre_program_resp));
                    }
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
            }
            break;
        case 0xFF00u: // Erase FLASH
            if (uds_request_info.sessionMode == PROGRAM_SESSION)
            {
                if (length == BOOT_Frame_length_12)
                {
                    if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_FLASH_DRIVER_CMAC_CHECK) && (flashdrv_cmac_succ == AS_TRUE))
                    {
                        /* PRQA S 3200 3 #3264 - Return value ignored, verified safe for system operation */
                        lin_uds_negative_response(lin_configured_NAD, param[0u], RCRRP);
                        delay_ms(90u);
                        dfu_image_erase();
                        dfu_ctx.op_code = (uint8_t)DFU_CMD_APP_ERASE;
                        pre_program_resp[0u] = param[2u];
                        pre_program_resp[1u] = param[3u];
                        dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, 2u);
                    }
                    else
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], CONDITION_NOT_CORRECT); // NRC22
                    }
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
            }
            break;
        case 0xFF01u: // Application Compatibility Check
            if (uds_request_info.sessionMode == PROGRAM_SESSION)
            {
                if (length == BOOT_Frame_length_4)
                {
                    if ((dfu_ctx.op_code == (uint8_t)DFU_CMD_APP_CMAC_CHECK) && (app_cmac_succ == AS_TRUE))
                    {
                        dfu_ctx.op_code = (uint8_t)DFU_CMD_APP_COMPATIBLE_CHECK;
                        pre_program_resp[0u] = param[2u];
                        pre_program_resp[1u] = param[3u];
                        pre_program_resp[2u] = 0u;
                        /* Update the app flag */
                        dfu_process_exit((uint8_t)DFU_MSG_SUCCESS);
                        dfu_do_notify_cp(param[0u], param[1u], (uint8_t *)pre_program_resp, sizeof(pre_program_resp));
                    }
                    else
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], CONDITION_NOT_CORRECT); // NRC22
                    }
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
            }
            break;
        default:
            dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
            break;
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
    }
}

/********************************************************
** @brief   处理 ECU 复位服务（SID = 0x11）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    支持硬件复位和软件复位，根据子功能决定
**          是否发送正响应。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
STATIC void reset_handle(uint8_t *param, uint16_t length)
{
    if (length == BOOT_Frame_length_2)
    {
        if (param[1] == 0x01u)
        {
            if (lin_get_uds_nad() == 0x7Eu)
            {
            }
            else if (lin_get_uds_nad() == 0x7Fu)
            {
            }
            else
            {
                dfu_do_notify_response(POSITIVE, param[0], param[1]);
            }

            delay_ms(50u);
            NVIC_SystemReset();
        }
        else if (param[1] == 0x81u)
        {
            NVIC_SystemReset();
        }
        else if ((param[1] == 0x03u) || (param[1] == 0x02u))
        {
            if (lin_get_uds_nad() == 0x7Eu)
            {
            }
            else if (lin_get_uds_nad() == 0x7Fu)
            {
            }
            else
            {
                dfu_do_notify_response(POSITIVE, param[0], param[1]);
            }
        }
        else if ((param[1] == 0x83u) || (param[1] == 0x82u)) // Affirmative response prohibited
        {
        }
        else
        {
            if (lin_get_uds_nad() == 0x7Eu)
            {
            }
            else if (lin_get_uds_nad() == 0x7Fu)
            {
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
            }
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/********************************************************
** @brief   处理清除 DTC 信息服务（SID = 0x14）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度（本函数未使用）
**
** @note    当前实现中仅返回服务不支持响应（NRC 0x7F）。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 1 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
STATIC void clear_dtc_info_handle(uint8_t *param, uint16_t length)
{
    (void)length;
    if (lin_get_uds_nad() == 0x7Eu)
    {
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
    }
}
#ifdef ENABLE_TEST_MODE
#else
/********************************************************
** @brief   启用 SWD 调试接口
**
** @note    配置 GPIO_PIN_0 和 GPIO_PIN_1 为 SWCLK 和 SWDIO
**          复用功能，供调试使用。
**
** @retval  无
*********************************************************/
    STATIC void enable_swd(void) // Enable SWD interface
    {
        ll_gpio_afio_config(GPIO_PIN_0, AFIO_MUX_0); // SWCLK
        ll_gpio_afio_config(GPIO_PIN_1, AFIO_MUX_0); // SWDIO
    }
#endif
/********************************************************
** @brief   处理配置字分配服务（SID = 0xB5）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    支持配置字的状态机管理：
**          - 开始（0x01）
**          - 分配（0x02）：设置车门位置
**          - 保存（0x03）：写入 Flash 并根据 NAD 使能 SWD
**          - 结束（0x04）
**
** @retval  无
*********************************************************/
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void assign_config_word_handle(uint8_t *param, uint16_t length)
{
    uint8_t fuc_id;
    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if (length == BOOT_Frame_length_6)
    {
        if ((0xF3u == param[1u]) && (0x3Fu == param[2u]) && (0x02u == param[4u]))
        {
            fuc_id = param[3u];
            switch (fuc_id)
            {
            case 0x01u:
                /* start */
                g_config_word_state = CONFIGURE_WORD_STATE_START;
                break;

            case 0x02u:
                /* assign */
                if (g_config_word_state == CONFIGURE_WORD_STATE_START)
                {
                    if ((param[5u] == (uint8_t)LEFT_FRONT_DOOR) || (param[5u] == (uint8_t)LEFT_REAR_DOOR) ||
                        (param[5u] == (uint8_t)RIGHT_FRONT_DOOR) || (param[5u] == (uint8_t)RIGHT_REAR_DOOR))
                    {
                        g_user_info.config_word = param[5u];

                        g_config_word_state = CONFIGURE_WORD_STATE_ASIGN;
                        LOG_DFU("cfg %x\n", g_user_info.config_word);
                    }
                }
                break;

            case 0x03u:
                /* save */
                if (g_config_word_state == CONFIGURE_WORD_STATE_ASIGN)
                {
                    pal_store_data_set(CFG_WORD_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
                    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                    pal_store_data_get(CFG_WORD_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
                    uds_diagnostic_configword_remap_nad();
                    if (lin_get_uds_nad() == 0x68u)
                    {
                        #ifdef ENABLE_TEST_MODE
                        #else
                            enable_swd(); // Enable SWD interface
                        #endif
                    }
                    else if (lin_get_uds_nad() == 0x6Au)
                    {
                        #ifdef ENABLE_TEST_MODE
                        #else
                            enable_swd(); // Enable SWD interface
                        #endif
                    }
                    else if (lin_get_uds_nad() == 0x69u)
                    {
                        #ifdef ENABLE_TEST_MODE
                        #else
                            enable_swd(); // Enable SWD interface
                        #endif
                    }
                    else if (lin_get_uds_nad() == 0x6Bu)
                    {
                        #ifdef ENABLE_TEST_MODE
                        #else
                            enable_swd(); // Enable SWD interface
                        #endif
                    }
                    else
                    {
                        (void)0;
                    }
                    g_config_word_state = CONFIGURE_WORD_STATE_SAVE;
                    LOG_DFU("SAVE\n");
                }
                break;

            case 0x04u:
                /* end */
                g_config_word_state = CONFIGURE_WORD_STATE_END;
                break;

            default:
                (void)0;
                break;
            }
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/********************************************************
** @brief   处理通过标识符读取服务（SID = 0xB2）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    用于读取指定车门位置的配置字信息，验证
**          当前配置是否与请求一致。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 2 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
STATIC void read_by_identify_handle(uint8_t *param, uint16_t length)
{
    uint8_t rsp_data[5u] = {0u};
    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    if (length == BOOT_Frame_length_6)
    {
        if ((param[1u] == 0xF3u) && (param[2u] == 0x3Fu) && (param[3u] == 0xFFu) && (param[4u] == 0x02u))
        {
            if ((param[5u] == (uint8_t)LEFT_FRONT_DOOR) || (param[5u] == (uint8_t)LEFT_REAR_DOOR) ||
                (param[5u] == (uint8_t)RIGHT_FRONT_DOOR) || (param[5u] == (uint8_t)RIGHT_REAR_DOOR))
            {
                /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
                pal_store_data_get(CFG_WORD_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
                if (g_user_info.config_word == param[5u])
                {
                    rsp_data[0u] = 0x00u;
                    rsp_data[1u] = 0x00u;
                    rsp_data[2u] = 0x00u;
                    rsp_data[3u] = 0x02u;
                    rsp_data[4u] = g_user_info.config_word;
                    dfu_do_notify_cp_ex(param[0u], rsp_data, 5u);
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], GENERAL_PROGRAM_FAILURE); // NRC72
                }
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
            }
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/********************************************************
** @brief   用户自定义读取数据标识符（DID）处理函数
**
** @param   mul_flag  多 DID 读取标志（0：单 DID，非 0：多 DID）
** @param   mul_len   多 DID 模式下数据偏移长度
** @param   did       DID 标识符
** @param   len       输出参数，返回数据长度
**
** @note    支持以下 DID：
**          - 0xF187：Seres 零件号
**          - 0xF18A：Seres 供应商代码
**          - 0xF197：ECU 名称
**          - 0xF189：序列号
**          - 0x0216：软件版本
**          - 0xF184：指纹信息
**          - 0xF089：硬件版本
**          - 0xF180：Bootloader 版本
**
** @retval  无
*********************************************************/
STATIC void user_read_data_by_id(uint8_t mul_flag, uint8_t mul_len, uint16_t did, uint16_t *len)
{
    uint8_t loc;
    static const char *const seres_part_numbers[] = {"4280310-RW02"};
    uint8_t const seres_supplier_code[10u] = {(uint8_t)'3', (uint8_t)'1', (uint8_t)'9', (uint8_t)'7', 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u};
    uint8_t seres_ecu_name[10u] = {(uint8_t)'E', (uint8_t)'H', (uint8_t)'I', (uint8_t)'S', (uint8_t)'_', (uint8_t)'F', (uint8_t)'L', 0x20u, 0x20u, 0x20u};
    uint8_t seres_software_version[21u] = {0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u};
    uint8_t _hardware_version[8u] = {0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u};
    uint8_t _boot_version[8u] = {0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u, 0x20u};

    switch (did)
    {
    case 0xF187u:
        *len = 12u;
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
    case 0xF18Au:
        *len = 10u;
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
    case 0xF197u:
        *len = 10u;

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
    case 0xF189u:
        *len = 24u;
        if (mul_flag != 0u)
        {
            /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
            ll_flash_read(FLASH_TYPE_NVM, FLASH_SEQ_NUM_ADDR, (uint8_t *)&diagnosticTxBuffer[mul_len], 24u);
        }
        else
        {
            /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
            ll_flash_read(FLASH_TYPE_NVM, FLASH_SEQ_NUM_ADDR, (uint8_t *)&diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN], 24u);
        }
        break;
    /* Seres software version*/
    case 0x0216u:
        *len = 21u;
        /* copy seres_software_version[] */
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        ll_flash_read(FLASH_TYPE_NVM, FLASH_SERES_APP_SOFTVER_ADDR, (uint8_t *)&seres_software_version[0u], 21u);
        if (g_user_info.config_word == (uint8_t)LEFT_FRONT_DOOR) // Left front door handle
        {
            seres_software_version[7] = (uint8_t)'F';
            seres_software_version[8] = (uint8_t)'L';
        }
        else if (g_user_info.config_word == (uint8_t)LEFT_REAR_DOOR) // Left rear door handle
        {
            seres_software_version[7] = (uint8_t)'R';
            seres_software_version[8] = (uint8_t)'L';
        }
        else if (g_user_info.config_word == (uint8_t)RIGHT_FRONT_DOOR) // Right front door handle
        {
            seres_software_version[7] = (uint8_t)'F';
            seres_software_version[8] = (uint8_t)'R';
        }
        else if (g_user_info.config_word == (uint8_t)RIGHT_REAR_DOOR) // Right rear door handle
        {
            seres_software_version[7] = (uint8_t)'R';
            seres_software_version[8] = (uint8_t)'R';
        }
        else
        {
            (void)0;
        }

        for (loc = 0u; loc < 21u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(seres_software_version[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(seres_software_version[loc]);
            }
        }
        break;
    /* Seres fingerprint info*/
    case 0xF184u:
        *len = 10u;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        pal_store_read(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)&dfu_ctx.dfu_info, sizeof(last_dfu_info_t));
        for (loc = 0u; loc < 10u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(dfu_ctx.dfu_info.fingerprint[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(dfu_ctx.dfu_info.fingerprint[loc]);
            }
        }
        break;
    /* hardware verison info */
    case 0xF089u:
        *len = 8u;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        ll_flash_read(FLASH_TYPE_NVM, FLASH_HW_VERSION_ADDR, (uint8_t *)_hardware_version, sizeof(_hardware_version));
        for (loc = 0u; loc < 8u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(hardware_version[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(hardware_version[loc]);
            }
        }
        break;
    /* bootloader verison info */
    case 0xF180u:
        *len = 8u;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        ll_flash_read(FLASH_TYPE_NVM, FLASH_BOOT_VERSION_ADDR, (uint8_t *)_boot_version, sizeof(_boot_version));
        for (loc = 0u; loc < 8u; loc++)
        {
            if (mul_flag != 0u)
            {
                diagnosticTxBuffer[mul_len + loc] = (uint8_t)(_boot_version[loc]);
            }
            else
            {
                diagnosticTxBuffer[UDS_READ_BY_DID_MIN_RESP_LEN + loc] = (uint8_t)(_boot_version[loc]);
            }
        }
        break;
    default:
        (void)0;
        break;
    }
}

/********************************************************
** @brief   处理读取数据标识符服务（SID = 0x22）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    支持单 DID 和多 DID（最多 5 个）读取模式。
**          通过 uds_diag_DID_chk 验证 DID 有效性后，
**          调用 user_read_data_by_id 获取数据。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 4 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 2889 3 #3257 - Multiple return statements for logical clarity and efficiency */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void read_data_by_identify_handle(uint8_t *param, uint16_t length)
{
    uint8_t result = NEGATIVE;
    uint16_t msglen = 0u, datalen;
    uint16_t locdid = 0xFFFFu;
    mult_did_data_t mult_did;

    msglen = length;

    if (lin_get_uds_nad() == 0x7Eu)
    {
        return;
    }
    else if (lin_get_uds_nad() == 0x7Fu)
    {
        return;
    }
    else
    {
        (void)0;
    }
    /* message length correct check */
    if (UDS_READ_BY_DID_REQ_LEN == msglen)
    {
        locdid = ((uint16_t)param[1u] << 8u) + param[2u];
        result = uds_diag_DID_chk(locdid);
        /* DID supported */
        if (result != 0u)
        {
            /*call the user function to process the service after all checks are correct*/
            user_read_data_by_id(0, 0, locdid, &datalen);
            msglen = (datalen + UDS_READ_BY_DID_MIN_RESP_LEN);

            diagnosticTxBuffer[1u] = (uint8_t)(locdid >> 8u);
            diagnosticTxBuffer[2u] = (uint8_t)locdid;
            dfu_do_notify_cp_ex(param[0u], &diagnosticTxBuffer[1u], msglen - 1u);
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
        }
    }
    else if ((msglen > UDS_READ_BY_DID_REQ_LEN) && (msglen <= (((uint8_t)MULT_DID_MAX * 2u) + 1u)) && (((msglen - 1u) % 2u) == 0u)) // Up to 5 dids
    {
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        memset(&mult_did, 0, sizeof(mult_did_data_t));
        mult_did.did_num = (uint8_t)(msglen - 1u) / 2u;

        for (uint8_t i = 0; i < mult_did.did_num; i++)
        {
            mult_did.did_array[i] = ((uint16_t)param[(2u * i) + 1u] << 8u) + param[(2u * i) + 2u];
            result = uds_diag_DID_chk(mult_did.did_array[i]);
            /* DID supported */
            if (result != 0u)
            {
                mult_did.did_valid_flag |= (uint8_t)(1U << i);
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
            msglen = (mult_did.data_len + 1U);
            dfu_do_notify_cp_ex(param[0u], &diagnosticTxBuffer[1u], msglen - 1u);
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/********************************************************
** @brief   处理通信控制服务（SID = 0x28）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    在扩展会话模式下处理通信类型和使能/禁用控制。
**          支持子功能 0x00-0x03（物理寻址）和 0x80-0x83（功能寻址）。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 3 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void communcation_control_handle(uint8_t *param, uint16_t length)
{
    if (uds_request_info.sessionMode == EXTEND_SESSION)
    {
        if (length == BOOT_Frame_length_3)
        {
            if ((param[1u] == 0x00u) || (param[1u] == 0x01u) ||
                (param[1u] == 0x02u) || (param[1u] == 0x03u))
            {
                if ((param[2u] == 0x01u) || (param[2u] == 0x03u))
                {
                    if (lin_get_uds_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_get_uds_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        dfu_do_notify_response(POSITIVE, param[0u], param[1u]);
                    }
                }
                else
                {
                    if (lin_get_uds_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_get_uds_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
                    }
                }
            }
            else if ((param[1u] == 0x80u) || (param[1u] == 0x81u) ||
                     (param[1u] == 0x82u) || (param[1u] == 0x83u))
            {
                if ((param[2u] == 0x01u) || (param[2u] == 0x03u))
                {
                }
                else
                {
                    if (lin_get_uds_nad() == 0x7Eu)
                    {
                    }
                    else if (lin_get_uds_nad() == 0x7Fu)
                    {
                    }
                    else
                    {
                        dfu_do_notify_response(NEGATIVE, param[0u], REQUEST_OUT_RANGE); // NRC31
                    }
                }
            }
            else
            {
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
                }
            }
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
        }
    }
    else
    {
        if (lin_get_uds_nad() == 0x7Eu)
        {
        }
        else if (lin_get_uds_nad() == 0x7Fu)
        {
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
        }
    }
}

/********************************************************
** @brief   处理 DTC 控制服务（SID = 0x85）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    在扩展会话模式下处理 DTC 控制。
**          子功能 0x01：清除 DTC，0x02：设置 DTC。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 3 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void dtc_control_handle(uint8_t *param, uint16_t length)
{
    if (uds_request_info.sessionMode == EXTEND_SESSION)
    {
        if (length == BOOT_Frame_length_2)
        {
            if (param[1u] == 0x01u)
            {
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_do_notify_response(POSITIVE, param[0u], param[1u]);
                }
            }
            else if (param[1u] == 0x02u)
            {
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_do_notify_response(POSITIVE, param[0u], param[1u]);
                }
            }
            else if ((param[1] == 0x81u) || (param[1] == 0x82u))
            {
            }
            else
            {
                if (lin_get_uds_nad() == 0x7Eu)
                {
                }
                else if (lin_get_uds_nad() == 0x7Fu)
                {
                }
                else
                {
                    dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
                }
            }
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
        }
    }
    else
    {
        if (lin_get_uds_nad() == 0x7Eu)
        {
        }
        else if (lin_get_uds_nad() == 0x7Fu)
        {
        }
        else
        {
            dfu_do_notify_response(NEGATIVE, param[0u], SERVICENOTSUPPORTED_INACTIVESESSION); // NRC7F
        }
    }
}

/********************************************************
** @brief   处理诊断会话保持服务（SID = 0x3E，Tester Present）
**
** @param   param   UDS 请求报文指针
** @param   length  UDS 请求报文长度
**
** @note    子功能 0x00 发送正响应，0x80 抑制正响应。
**
** @retval  无
*********************************************************/
/* PRQA S 3673 3 #3259 - Pointer parameter design maintains API consistency, no impact on safety */
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void diagnostic_session_handle(uint8_t *param, uint16_t length)
{
    if (length == BOOT_Frame_length_2)
    {
        if (param[1] == 0x00u)
        {
            if (lin_get_uds_nad() == 0x7Eu)
            {
            }
            else if (lin_get_uds_nad() == 0x7Fu)
            {
            }
            else
            {
                dfu_do_notify_response(POSITIVE, param[0], param[1]);
            }
        }
        else if (param[1] == 0x80u)
        {
        }
        else
        {
            if (lin_get_uds_nad() == 0x7Eu)
            {
            }
            else if (lin_get_uds_nad() == 0x7Fu)
            {
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, param[0u], SUBFUNCTION_NOT_SUPPORTED); // NRC12
            }
        }
    }
    else
    {
        dfu_do_notify_response(NEGATIVE, param[0u], IMLOIF); // NRC13
    }
}

/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1514 1 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
const dfu_process_context_t dfu_process_ctx[] = {
    {SERVICE_SESSION_CONTROL, &session_control_handle},             // 0x10 Physical Addressing+Functional Addressing
    {SERVICE_ECU_RESET, &reset_handle},                             // 0x11 Physical Addressing+Functional Addressing
    {SERVICE_SECURITY_ACCESS, &security_access_handle},             // 0x27 Physical Addressing
    {SERVICE_FIRMWARE_INFO_SYNC, &firmware_info_sync_handle},       // 0x2E Physical Addressing
    {SERVICE_ROUTINE_CONTROL, &routine_control_handle},             // 0x31 Physical Addressing
    {SERVICE_REQUEST_DOWNLOAD, &request_download_handle},           // 0x34 Physical Addressing
    {SERVICE_TRANSFER_DATA, &transfer_data_handle},                 // 0x36 Physical Addressing
    {SERVICE_REQUEST_TRANSFER_EXIT, &request_transfer_exit_handle}, // 0x37 Physical Addressing
    {SERVICE_CLEAR_DTC_INFO, &clear_dtc_info_handle},               // 0x14 Physical Addressing+Functional Addressing
    {SERVICE_ASSIGN_NAD_VIA_SNPD, &assign_config_word_handle},      // 0xB5
    {SERVICE_READ_BY_IDENTIFY, &read_by_identify_handle},           // 0xB2
    {SERVICE_READ_DATA_BY_IDENTIFY, &read_data_by_identify_handle}, // 0x22 Physical Addressing
    {SERVICE_COMMUNCATION_CONTROL, &communcation_control_handle},   // 0x28 Functional Addressing
    {SERVICE_DTC_CONTROL, &dtc_control_handle},                     // 0x85 Functional Addressing
    {SERVICE_TESTER_PRESENT, &diagnostic_session_handle},           // 0x3E Functional Addressing
#ifdef ENABLE_TEST_MODE
    {0x00, NULL},  /* test only: NULL func to cover the NULL check branch */
#endif
};

#define DFU_PROCESS_STEP_MAX (sizeof(dfu_process_ctx) / sizeof(dfu_process_context_t))

/********************************************************
** @brief   更新 LIN 随机种子值
**
** @note    循环更新 seed 数组的每个字节，使用当前会话超时
**          计数器和自增计数器增加随机性。用于安全访问的种子生成。
**
** @retval  无
*********************************************************/
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void lin_update_random_value(void)
{
    static uint8_t random_i = 0u;
    static uint8_t auto_add_cnt = 0u;
    if (random_i < 16u)
    {
        seed[random_i] = seed[random_i] + random_i + (uint8_t)dfu_ctx.uds_timeout + auto_add_cnt;
        random_i++;
    }
    else
    {
        random_i = 0u;
    }
    auto_add_cnt++;
}

/********************************************************
** @brief   LIN 异常处理函数
**
** @note    在主循环中调用，处理以下异常：
**          - 保存解锁失败计数到 Flash
**          - 诊断会话超时时重置系统（如果 APP 标志有效）
**
** @retval  无
*********************************************************/
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void lin_exception_handle(void)
{
    if (unlock_failed_store_flag == AS_TRUE)
    {
        unlock_failed_store_flag = AS_FALSE;
        pal_store_data_set(SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
    }
    if (diagnostic_session_overtime_flag == AS_TRUE)
    {
        diagnostic_session_overtime_flag = AS_FALSE;
        uds_request_info.sessionMode = DEFALUT_SESSION;
        dfu_ctx.op_code = (uint8_t)DFU_CMD_DEFAULT_SESSION;
        /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
        pal_store_read(FLASH_TYPE_NVM, FLASH_DFU_INFO_ADDR, (uint8_t *)&dfu_ctx.dfu_info, sizeof(last_dfu_info_t));
        if (DFU_INFO_MAGIC == dfu_ctx.dfu_info.magic)
        {
            NVIC_SystemReset();
        }
    }
}

/********************************************************
** @brief   LIN 诊断服务分发处理
**
** @note    接收 LIN 报文并根据 SID 查找对应的服务处理函数
**          进行分发。未识别的 SID 返回服务不支持响应（NRC 0x11）。
**
** @retval  无
*********************************************************/
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void lin_diag_service_handle(void)
{
    uint16_t length = 0u;
    uint8_t i;
    uint8_t *base = (uint8_t *)&dfu_ctx.queue_list.packet[dfu_ctx.queue_list.tail];
    uint8_t *ptr = &base[2u];
    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    lin_uds_receive(lin_configured_NAD, ptr, &length);
    uint8_t sid = ptr[0u];
    if (length != 0u)
    {
        for (i = 0u; i < DFU_PROCESS_STEP_MAX; i++)
        {
            if (sid == dfu_process_ctx[i].sid)
            {

                if (NULL != dfu_process_ctx[i].func)
                {
                    dfu_process_ctx[i].func(ptr, length);
                }
                break;
            }
        }
        if (i == DFU_PROCESS_STEP_MAX)
        {
            if (lin_get_uds_nad() == 0x7Eu)
            {
            }
            else if (lin_get_uds_nad() == 0x7Fu)
            {
            }
            else
            {
                dfu_do_notify_response(NEGATIVE, ptr[0u], SERVICE_NOT_SUPPORTED);
            }
        }
        dfu_ctx.uds_timeout = 0u;
    }
}

/********************************************************
** @brief   DFU 超时处理
**
** @note    递增超时计数器，当连续未收到诊断报文时间超过
**          LIN_UDS_TIMEOUT 时，设置诊断会话超时标志。
**
** @retval  无
*********************************************************/
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
void dfu_timeout_handle(void)
{
    dfu_ctx.uds_timeout++;
    if (dfu_ctx.uds_timeout > LIN_UDS_TIMEOUT)
    {
        dfu_ctx.uds_timeout = 0u;
        // 5s did not receive any messages, diagnostic session timed out, set the timeout flag. In the main loop, check whether the APP flag is valid; if valid, reset it and then enter the APP's default session, if invalid, stay in the bootloader's default session.
        diagnostic_session_overtime_flag = AS_TRUE;
    }
}

/********************************************************
** @brief   DFU 系统数据初始化
**
** @note    从 Flash 中读取配置字信息和 OTA 系统参数，
**          并根据 NAD 值重新映射 LIN 通信地址。
**
** @retval  无
*********************************************************/
/* PRQA S 1505 2 #3219 - Function used only in the defining translation unit, intentional design */
/* PRQA S 3408 1 #3218 - External linkage function defined without prior declaration, intentional design */
void dfu_store_system_data_init(void)
{
    /* PRQA S 3200 2 #3264 - Return value ignored, verified safe for system operation */
    ll_flash_read(STORE_TYPE_SEL, CFG_WORD_BASE_ADDR, (uint8_t *)&g_user_info, sizeof(g_user_info));
    ll_flash_read(STORE_TYPE_SEL, SYSTEM_PARAM_BASE_ADDR, (uint8_t *)&g_ota_info, sizeof(g_ota_info));
    uds_diagnostic_configword_remap_nad();
}

/********************************************************
** @brief   DFU 管理器初始化
**
** @note    初始化看门狗、生成 CMAC 密钥、初始化日志、
**          加载系统配置数据、清空 DFU 上下文并初始化 LIN 总线。
**
** @retval  无
*********************************************************/
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void dfu_manager_init(void)
{
    wdg_init(10000u);
    Gen_CMACkey(key);
#ifdef CFG_SUPPORT_DEBUG
    logging_init();
#endif
    dfu_store_system_data_init();
    /* PRQA S 3200 1 #3264 - Return value ignored, verified safe for system operation */
    memset(&dfu_ctx, 0, sizeof(dfu_ctx));
    pal_lin_init(LIN_BUS_0, LIN_MODE_SLV, LIN_BAUD_RATE, &lin_lld_isr_callback);

    LOG_DFU("lin_configured_NAD = %02X\r\n", lin_configured_NAD);
}

/********************************************************
** @brief   主循环处理
**
** @note    喂狗、处理 LIN 诊断服务、异常处理、更新随机种子。
**          在空闲状态下周期性检查 DFU 信息以决定跳转到 APP
**          或进入升级模式。
**
** @retval  无
*********************************************************/

/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void main_loops(void)
{
    static uint32_t LoopCnt = 0u;
    wdg_reload();
    lin_diag_service_handle();
    lin_exception_handle();
    lin_update_random_value();

    if (dfu_ctx.boot_state == (uint8_t)BOOT_STATE_IDLE)
    {
        delay_ms(1u);

        /* about 42ms+5ms */
        /* PRQA S 3440 1 #3221 - Use of increment or decrement result is safe and required by logic */
        if ((++LoopCnt) > 50u)
        {
            LoopCnt = 0u;

            if ((uint8_t)DFU_MSG_SUCCESS == last_dfu_info_get())
            {
                LOG_DFU("BOOT_STATE_USER_APP\r\n");
                dfu_ctx.boot_state = (uint8_t)BOOT_STATE_USER_APP;
            }
            else
            {
                LOG_DFU("BOOT_STATE_UPGRADE\r\n");
                dfu_ctx.boot_state = (uint8_t)BOOT_STATE_UPGRADE;
            }
        }
    }
    else if (dfu_ctx.boot_state == (uint8_t)BOOT_STATE_USER_APP)
    {
        JumpToApp(); /* jump user app*/
    }
    else
    {
        (void)0;
    }
}

/********************************************************
** @brief   操作系统任务更新
**
** @note    处理 DFU 超时检测。当计算 APP CMAC 时，每 1.8 秒
**          发送 NRC 0x78 响应。解锁失败 3 次后，每 10 秒
**          释放一次锁定计数。
**
** @retval  无
*********************************************************/
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1514 1 #3212 - The object is only referenced by a single function within the translation unit, reserved by intentional design */
uint8_t clear_wdg_cnt = 0u;
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
void os_task_update(void)
{
    dfu_timeout_handle();
    if (app_cmac_start == AS_TRUE)
    {
        /* PRQA S 3387 2 #3265 - Increment or decrement operation is safe with no unintended side effects */
        /* PRQA S 3440 1 #3221 - Use of increment or decrement result is safe and required by logic */
        if ((++timer_1s_cnt) > 1800u)
        {
            timer_1s_cnt = 0u;
            dfu_do_notify_response(NEGATIVE, 0x31u, RCRRP); // NRC78, when calculating APP CMAC, respond to NRC78 every 1.8 seconds
        }
        /* PRQA S 3440 1 #3221 - Use of increment or decrement result is safe and required by logic */
        if ((++clear_wdg_cnt) > 100u)
        {
            clear_wdg_cnt = 0u;
            wdg_reload();
        }
    }
    if (g_ota_info.lock_failed_index > 2u) // In the case of 3 times, release once every 10 seconds
    {
        /* PRQA S 3440 1 #3221 - Use of increment or decrement result is safe and required by logic */
        if ((++lock_failed_cnt) > 10000u)
        {
            lock_failed_cnt = 0u;
            g_ota_info.lock_failed_index = 2u;
            // Set a storage flag for updating the 27 unlock failure count, and operate the flash in the main loop
            unlock_failed_store_flag = AS_TRUE;
        }
    }
}
