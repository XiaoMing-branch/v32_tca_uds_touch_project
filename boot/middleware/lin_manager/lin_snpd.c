/**
 *****************************************************************************
 * @brief   lin snpd example source file.
 *
 * @file    lin_snpd.c
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

#include "lin.h"
#include "diagnosticIII.h"
#include "lin_snpd.h"
#include "store_manager.h"
#include "measure.h"
#include "pal_lin_comm.h"
#include "pal_systick.h"
#include "logging.h"

#define LOG_SPND(...)  do{log_debug("[SNPD] " __VA_ARGS__);}while(0)
#define LIN_SNPD_TIMEOUT   40000

lin_snpd_context_t *lin_snpd_ctx;

#if CFG_SUPPORT_LIN_SNPD

#if CFG_SNPD_SFT_MODE
#define SNPD_SLAVE_NUM            15
typedef struct
{
    uint8_t org_nad;
    uint8_t new_nad;
    uint16_t code[5];
} snpd_raw_date_t;
snpd_raw_date_t snpd_raw_date[SNPD_SLAVE_NUM];
#endif

/**
 * @brief  输出SNPD原始ADC数据（软件模式）
 * @param  org_nad - 原始NAD地址
 * @param  new_nad - 新NAD地址
 * @note   仅在CFG_SNPD_SFT_MODE使能时有效，将原始NAD和新NAD存储到raw_date数组
 * @retval 无
 */
static void lin_snpd_raw_adc_out(uint8_t org_nad, uint8_t new_nad)
{
#if CFG_SNPD_SFT_MODE
    uint8_t count = 0;
    count = new_nad % 0x10;

    if (count > SNPD_SLAVE_NUM - 1)
    {
        count = 15;
    }

    snpd_raw_date[count].org_nad = org_nad;
    snpd_raw_date[count].new_nad = new_nad;

    for (uint8_t i = 0; i < 5; i++)
    {
        snpd_raw_date[count].code[i] = 0;
    }

    uint8_t raw_len = pal_lin_aa_raw_code_get(snpd_raw_date[count].code, 5);

    for (uint8_t i = raw_len; i < 5; i++)
    {
        snpd_raw_date[count].code[i] = 11;
    }

#else
    (void) org_nad;
    (void) new_nad;
#endif
}

/**
 * @brief  进入SNPD模式
 * @param  无
 * @note   暂停PN测量采样，清除SNPD状态，设置电流阈值，进入自动寻址模式
 * @retval 无
 */
static void lin_snpd_enter(void)
{
#if CFG_SUPPORT_LIN_SNPD && CFG_SUPPORT_LIN_SNPD_LED

    if (NULL != lin_snpd_ctx->enter_func)
    {
        lin_snpd_ctx->enter_func();
    }

#endif

    uint16_t aa_cur_th[4];
    aa_cur_th[0] = ((g_sys_cfgs.cur_th_st12 & 0xFF00) >> 8) | ((g_sys_cfgs.cur_th_st12 & 0xFF) << 8);
    aa_cur_th[1] = ((g_sys_cfgs.cur_th_st12 & 0xFF000000) >> 24) | ((g_sys_cfgs.cur_th_st12 & 0xFF0000) >> 8);
    aa_cur_th[2] = ((g_sys_cfgs.cur_th_st34 & 0xFF00) >> 8) | ((g_sys_cfgs.cur_th_st34 & 0xFF) << 8);
    aa_cur_th[3] = ((g_sys_cfgs.cur_th_st34 & 0xFF000000) >> 24) | ((g_sys_cfgs.cur_th_st34 & 0xFF0000) >> 8);

    /* aa and colormix mutex */
    for (led_channel_e channel = LED_CHANNEL_0; channel < LED_CHANNEL_MAX; channel++)
    {
        meas_pn_sample_suspend(channel);
    }

    memset(lin_snpd_ctx->status, 0, LIN_AA_STATUS_MAX);
#if CFG_SNPD_SFT_MODE
    memset(snpd_raw_date, 0, SNPD_SLAVE_NUM * sizeof(snpd_raw_date_t));
#endif

    pal_lin_aa_enter(aa_cur_th);
}

/**
 * @brief  退出SNPD模式
 * @param  无
 * @note   退出自动寻址模式，恢复PN测量采样，调用退出回调函数
 * @retval 无
 */
static void lin_snpd_exit(void)
{
    pal_lin_aa_exit();

    /* aa and colormix mutex */
    for (led_channel_e channel = LED_CHANNEL_0; channel < LED_CHANNEL_MAX; channel++)
    {
        meas_pn_sample_resume(channel);
    }

#if CFG_SUPPORT_LIN_SNPD && CFG_SUPPORT_LIN_SNPD_LED

    if (NULL != lin_snpd_ctx->exit_func)
    {
        lin_snpd_ctx->exit_func();
    }

#endif
}

/**
 * @brief  SNPD选择确认（准备好NAD选择）
 * @param  无
 * @note   通知底层硬件NAD选择已完成
 * @retval 无
 */
static void lin_snpd_select(void)
{
    pal_lin_aa_ready();
}

/**
 * @brief  写入NAD地址到Flash存储
 * @param  nad - 要写入的NAD地址值
 * @note   同时更新系统配置结构体并保存到Flash
 * @retval 无
 */
static void lin_snpd_nad_write(uint8_t nad)
{
    g_sys_cfgs.nad = nad;
    store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
}

/**
 * @brief  DFU模式下自动配置NAD地址
 * @param  无
 * @note   使用系统滴答计数值的低4位作为NAD地址
 * @retval 无
 */
static void autoaddress_config_for_dfu(void)
{
    uint32_t value = systick_count_get();

    lin_configured_NAD = value & 0x0F;
    lin_snpd_nad_write(lin_configured_NAD);
}

/**
 * @brief  获取SNPD状态
 * @param  type - 状态类型（@ref lin_aa_status_e）
 * @retval 状态值，无效类型返回0
 */
uint8_t lin_snpd_status_get(lin_aa_status_e type)
{
    if (type >= LIN_AA_STATUS_MAX)
    {
        return 0;
    }

    return (lin_snpd_ctx->status[type]);
}

/**
 * @brief  设置SNPD状态
 * @param  type - 状态类型（@ref lin_aa_status_e）
 * @param  value - 要设置的状态值
 * @note   若上下文指针为空则直接返回
 * @retval 无
 */
void lin_snpd_status_set(lin_aa_status_e type, uint8_t value)
{
    if (NULL == lin_snpd_ctx)
    {
        return;
    }

    if (type < LIN_AA_STATUS_MAX)
    {
        lin_snpd_ctx->status[type] = value;
    }
}

#if (CFG_SUPPORT_LIN_SNPD_TIMEOUT)
/**
 * @brief  SNPD超时检测
 * @param  无
 * @note   超时计数达到LIN_SNPD_TIMEOUT时退出SNPD模式并复位状态
 * @retval 无
 */
static void lin_snpd_timeout_check(void)
{
    if (LIN_AA_STATE_IDLE != lin_snpd_status_get(LIN_AA_STATUS_STATE))
    {
        if (lin_snpd_ctx->timeout >= LIN_SNPD_TIMEOUT)
        {
            lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_IDLE);
            lin_snpd_exit();
        }
        else
        {
            lin_snpd_ctx->timeout++;
        }
    }
    else
    {
        lin_snpd_ctx->timeout = 0;
    }
}
#endif

/**
 * @brief  SNPD流程处理函数（主循环调用）
 * @param  无
 * @note   处理SNPD状态机：空闲→进入→NAD选择→保存→退出
 * @retval 无
 */
void lin_snpd_process_handle(void)
{
    if (NULL == lin_snpd_ctx)
    {
        return;
    }

#if (CFG_SUPPORT_LIN_SNPD_TIMEOUT && CFG_SUPPORT_LIN_SNPD)
    /* timeout check */
    lin_snpd_timeout_check();
#endif

    uint8_t aa_step = lin_snpd_status_get(LIN_AA_STATUS_STEP);

    if (!aa_step)
    {
        return;
    }

    if (LIN_SNPD_CMD_ENTER == aa_step || LIN_SNPD_CMD_EXIT == aa_step)
    {
        if (LIN_SNPD_CMD_EXIT == aa_step)
        {
            lin_snpd_exit();
        }

        lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_IDLE);
    }

    switch (lin_snpd_status_get(LIN_AA_STATUS_STATE))
    {
        case LIN_AA_STATE_IDLE:
            if (LIN_SNPD_CMD_ENTER == aa_step)
            {
                lin_snpd_enter();
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_ENTER);
            }

            break;

        case LIN_AA_STATE_ENTER:
            if (LIN_SNPD_CMD_NAD == aa_step && lin_snpd_status_get(LIN_AA_STATUS_SELECT))
            {
                lin_configured_NAD = lin_snpd_status_get(LIN_AA_STATUS_NAD);
                lin_snpd_select();
                lin_snpd_status_set(LIN_AA_STATUS_SELECT, 0);
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_SAVE);
            }

            if (lin_snpd_status_get(LIN_AA_STATUS_RAW_CODE))
            {
                lin_snpd_status_set(LIN_AA_STATUS_RAW_CODE, false);
                lin_snpd_raw_adc_out((uint8_t)g_sys_cfgs.org_nad, lin_snpd_status_get(LIN_AA_STATUS_NAD));
            }

            break;

        case LIN_AA_STATE_SAVE:
            if (LIN_SNPD_CMD_SAVE == aa_step)
            {
                /* 将new nad 保存到flash中to do */
                lin_snpd_nad_write(lin_configured_NAD);
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_EXIT);
            }

            break;

        case LIN_AA_STATE_EXIT:
            if (LIN_SNPD_CMD_EXIT == aa_step)
            {
                lin_snpd_exit();
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_IDLE);
            }

            break;

        default:
            break;
    }

    lin_snpd_status_set(LIN_AA_STATUS_STEP, 0);
}

/**
 * @brief  SNPD诊断命令处理
 * @param  ptr - 诊断数据缓冲区指针
 * @param  length - 数据长度
 * @note   处理0xFF（步进命令）、0xAA/0xBB（电流阈值）、0xCC（NAD配置）、0xD0~0xD4（软件模式数据读取）
 * @retval 无
 */
void lin_snpd_diag_handle(uint8_t *ptr, uint16_t length)
{
    uint8_t fuc_id;
    uint32_t cur_th_st12 = 0;
    uint32_t cur_th_st34 = 0;
    (void)(&cur_th_st12);
    (void)(&cur_th_st34);

    if (ptr[1] == 0xFF)
    {
        fuc_id = ptr[3];
        lin_snpd_status_set(LIN_AA_STATUS_STEP, ptr[3]);

        /* update new nad */
        if (LIN_SNPD_CMD_NAD == fuc_id)
        {
            lin_snpd_status_set(LIN_AA_STATUS_NAD, ptr[5]);
            lin_snpd_status_set(LIN_AA_STATUS_RAW_CODE, true);
        }
    }

    if (ptr[1] == 0xAA)
    {
        g_sys_cfgs.cur_th_st12 = ptr[5] << 24 | ptr[4] << 16 | ptr[3] << 8 | ptr[2];
        LOG_SPND("Current threshold step1&2 = %08x\r\n", g_sys_cfgs.cur_th_st12);
        store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
    }

    if (ptr[1] == 0xBB)
    {
        g_sys_cfgs.cur_th_st34 = ptr[5] << 24 | ptr[4] << 16 | ptr[3] << 8 | ptr[2];
        LOG_SPND("Current threshold step3&4 = %08x\r\n", g_sys_cfgs.cur_th_st34);
        store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
    }

#if CFG_SUPPORT_ORG_NAD

    if (ptr[1] == 0xCC)
    {
        switch (ptr[2])
        {
            case CUSTOMER_NAD_ORIGIN_SET:
            {
                lin_configured_NAD = ptr[3];
                g_sys_cfgs.org_nad = lin_configured_NAD;
                store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
                break;
            }

            case CUSTOMER_NAD_ORIGIN_GET:
            {
                store_system_data_get(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
                lin_configured_NAD = g_sys_cfgs.org_nad;
                break;
            }

            case CUSTOMER_NAD_RECOVERY_ORIGIN:
            {
                store_system_data_get(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
                lin_configured_NAD = g_sys_cfgs.org_nad;
                g_sys_cfgs.nad = g_sys_cfgs.org_nad;
                store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
                break;
            }

            case CUSTOMER_NAD_SET_FOR_DFU:
            {
                autoaddress_config_for_dfu();
                break;
            }

            case CUSTOMER_NAD_GET_FOR_DFU:
            {
                lin_diag_positive_notify(ptr[0], NULL, 0);
                break;
            }

            default:
                break;
        }
    }

#if CFG_SNPD_SFT_MODE

    if ((ptr[1] == 0xD0) || (ptr[1] == 0xD1) || (ptr[1] == 0xD2)
        || (ptr[1] == 0xD3) || (ptr[1] == 0xD4))
    {
        uint8_t count = ptr[1] % 0xD0;;
        uint8_t data_len = (ptr[1] == 0xD4) ? 32 : 40;

        for (uint8_t i = 0; i < data_len; i++)
        {
            ptr[i + 1] = *((uint8_t *)(&snpd_raw_date) + i + 40 * count);
        }

        lin_diag_positive_notify(ptr[0], &ptr[1], data_len);
    }

#endif
#endif
}
#endif

/**
 * @brief  初始化SNPD模块
 * @param  ctx - SNPD上下文结构体指针
 * @note   保存上下文指针以供其他函数使用
 * @retval 无
 */
void lin_snpd_init(lin_snpd_context_t *ctx)
{
    lin_snpd_ctx = ctx;
}