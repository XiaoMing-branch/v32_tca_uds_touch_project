/**
 *****************************************************************************
 * @brief   LIN SNPD header file.
 *
 * @file    lin_snpd.h
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

#ifndef __LIN_SNPD_H__
#define __LIN_SNPD_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CFG_SNPD_SFT_MODE       0
#if CFG_SNPD_SFT_MODE
#define CFG_SUPPORT_ORG_NAD     1
#endif // CFG_SNPD_SFT_MODE

//CUSTOMER_NAD_CONFIG
#define CUSTOMER_NAD_ORIGIN_SET                 0x01
#define CUSTOMER_NAD_ORIGIN_GET                 0x02
#define CUSTOMER_NAD_RECOVERY_ORIGIN            0x03
#define CUSTOMER_NAD_SET_FOR_DFU                0x04
#define CUSTOMER_NAD_GET_FOR_DFU                0x05

#define LIN_SNPD_CMD_ENTER      0x01
#define LIN_SNPD_CMD_NAD        0x02
#define LIN_SNPD_CMD_SAVE       0x03
#define LIN_SNPD_CMD_EXIT       0x04

#define LIN_AA_STATE_IDLE       0
#define LIN_AA_STATE_ENTER      1
#define LIN_AA_STATE_SAVE       2
#define LIN_AA_STATE_EXIT       3

typedef enum
{
    LIN_AA_STATUS_STATE = 0,
    LIN_AA_STATUS_NAD,
    LIN_AA_STATUS_STEP,
    LIN_AA_STATUS_SELECT,
    LIN_AA_STATUS_RAW_CODE,
    LIN_AA_STATUS_MAX,
} lin_aa_status_e;

typedef void (*LIN_FUNC_CALLBACK)(void);

typedef struct
{
    uint32_t timeout;
    uint8_t status[LIN_AA_STATUS_MAX];
    LIN_FUNC_CALLBACK enter_func;
    LIN_FUNC_CALLBACK exit_func;
} lin_snpd_context_t;

/**
 * @brief  获取SNPD状态值
 * @param  type - 状态类型（@ref lin_aa_status_e）
 * @retval 返回对应状态值，类型无效时返回0
 */
uint8_t lin_snpd_status_get(lin_aa_status_e type);

/**
 * @brief  设置SNPD状态值
 * @param  type - 状态类型（@ref lin_aa_status_e）
 * @param  value - 要设置的状态值
 * @retval 无
 */
void lin_snpd_status_set(lin_aa_status_e type, uint8_t value);

/**
 * @brief  初始化SNPD模块
 * @param  ctx - SNPD上下文结构体指针
 * @retval 无
 */
void lin_snpd_init(lin_snpd_context_t *ctx);

/**
 * @brief  SNPD流程处理（主循环周期调用）
 * @param  无
 * @retval 无
 */
void lin_snpd_process_handle(void);

/**
 * @brief  SNPD诊断命令处理
 * @param  ptr - 诊断数据缓冲区指针
 * @param  length - 数据长度
 * @retval 无
 */
void lin_snpd_diag_handle(uint8_t *ptr, uint16_t length);

#ifdef __cplusplus
}
#endif
#endif /* __LIN_SNPD_H__ */
