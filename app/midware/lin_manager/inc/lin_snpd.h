/**
 *****************************************************************************
 * @brief   LIN从节点位置检测(SNPD)模块头文件。
 *          定义SNPD测试模式、状态枚举、ADC数据结构及API声明。
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

#define SNPD_TEST_MODE_NONE     0
#define SNPD_TEST_MODE_PRINT    1
#define SNPD_TEST_MODE_LIN      2

#define SNPD_TEST_MODE SNPD_TEST_MODE_NONE

#if SNPD_TEST_MODE == SNPD_TEST_MODE_LIN
#define AA_SLAVE_NUM            15
#endif

#define SET_ORGINAL_NAD         1

#define LIN_SNPD_CMD_ENTER      0x01
#define LIN_SNPD_CMD_NAD        0x02
#define LIN_SNPD_CMD_SAVE       0x03
#define LIN_SNPD_CMD_EXIT       0x04

#define LIN_AA_STATE_IDLE       0
#define LIN_AA_STATE_ENTER      1
#define LIN_AA_STATE_SAVE       2
#define LIN_AA_STATE_EXIT       3

/**
 * @brief  SNPD自动寻址状态枚举，用于lin_snpd_context_t中的状态数组索引
 */
typedef enum
{
    LIN_AA_STATUS_STATE = 0,    /* AA状态机当前状态 */
    LIN_AA_STATUS_NAD,           /* 节点地址(NAD) */
    LIN_AA_STATUS_STEP,          /* AA执行步骤 */
    LIN_AA_STATUS_SELECT,        /* NAD选择标志 */
    LIN_AA_STATUS_RAW_CODE,      /* ADC裸数据就绪标志 */
    LIN_AA_STATUS_MAX,           /* 状态总数 */
} lin_aa_status_e;

/** @brief SNPD回调函数类型（进入/退出AA模式时的LED控制回调） */
typedef void (*LIN_FUNC_CALLBACK)(void);

#if (SNPD_TEST_MODE == SNPD_TEST_MODE_LIN)
/** @brief AA过程ADC原始数据结构体 */
struct aa_adc_data
{
    uint8_t org_nad;       /* 原始节点地址 */
    uint8_t new_nad;        /* 新分配的节点地址 */
    uint16_t adc[5];        /* 5通道ADC原始值 */
};
extern struct aa_adc_data adc_raw_data[AA_SLAVE_NUM];
#endif

/** @brief SNPD上下文结构体，保存AA过程的运行时状态和回调 */
typedef struct
{
    uint32_t timeout;                         /* 超时计数器 */
    uint8_t status[LIN_AA_STATUS_MAX];        /* 状态数组（状态/NAD/步骤/标志） */
    LIN_FUNC_CALLBACK enter_func;             /* 进入AA模式回调（如LED指示） */
    LIN_FUNC_CALLBACK exit_func;              /* 退出AA模式回调（如LED恢复） */
} lin_snpd_context_t;

/** @brief 输出ADC裸数据，用于自动寻址过程中记录各通道ADC原始值 */
void lin_snpd_raw_adc_out(uint8_t org_nad, uint8_t new_nad);
/** @brief 获取SNPD模块指定状态值 */
uint8_t lin_snpd_status_get(lin_aa_status_e type);
/** @brief 设置SNPD模块指定状态值 */
void lin_snpd_status_set(lin_aa_status_e type, uint8_t value);
/** @brief 读取当前节点地址(NAD) */
void lin_snpd_nad_read(uint8_t *nad);
#ifdef CFG_LIN_CONFORM_TEST
/** @brief 读取LIN帧ID配置（仅一致性测试） */
void lin_snpd_id_read(void);
#endif
/** @brief 写入节点地址(NAD)并保存到Flash */
void lin_snpd_nad_write(uint8_t nad);
/** @brief 读取电流阈值配置 */
void lin_snpd_cur_th_get(uint32_t *st12, uint32_t *st34);
/** @brief 设置电流阈值配置并保存到Flash */
void lin_snpd_cur_th_set(uint32_t *st12, uint32_t *st34);
/** @brief 初始化SNPD模块，注册上下文 */
void lin_snpd_init(lin_snpd_context_t *ctx);
/** @brief SNPD主状态机处理函数，需在主循环中周期性调用 */
void lin_snpd_process_handle(void);
/** @brief DFU模式下自动配置节点地址 */
void autoaddress_config_for_dfu(void);

#ifdef __cplusplus
}
#endif
#endif /* __LIN_SNPD_H__ */
