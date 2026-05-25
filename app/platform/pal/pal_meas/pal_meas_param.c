/**
 *****************************************************************************
 * @brief   pal meas param source file.
 *
 * @file    pal_meas_param.c
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

#include "pal_func_def.h"
#include "pal_meas_def.h"

/**
 * @brief 默认VBAT/VTEMP ADC参数配置表（TCPL03X）
 * @note  [0] = VTEMP通道, [1] = VBAT通道
 */
#if defined (__TCPL03X__)
adc_cfg_t default_vbvt_param_table[2] =
{
    {
        .ratio = 1,
        .adc_buf_en = false,
        .adc_buf_bypass = true,
        .adc_pag_en = true,
        .adc_pag_bypass = false,
        .vcr_enable = false,
        .vcr = ADC_VCR_SEL_236_7,
        .gain = ADC_GAIN_X16,
        .vcm = ADC_VCM_SEL_205,
        .vref = ADC_VREF_2500,
    },
    {
        .ratio = 20,
        .adc_buf_en = false,
        .adc_buf_bypass = true,
        .adc_pag_en = true,
        .adc_pag_bypass = false,
        .vcr_enable = false,
        .vcr = ADC_VCR_SEL_236_7,
        .gain = ADC_GAIN_X2,
        .vcm = ADC_VCM_SEL_205,
        .vref = ADC_VREF_2500,
    },
};

/**
 * @brief RGB各通道ADC参数配置表（TCPL03X）
 * @note  按[RGB_RED/GREEN/BLUE][SERIAL_1]索引，仅有单串配置
 */
const adc_cfg_t seft_check_param_table[RGB_TYPE_MAX][LED_MEAS_SERIAL_MAX] =
{
    {
        {
            .ratio = 5,
            .adc_buf_en = false,
            .adc_buf_bypass = true,
            .adc_pag_en = false,
            .adc_pag_bypass = true,
            .vcr_enable = false,
            .vcr = ADC_VCR_SEL_315_6,
            .gain = ADC_GAIN_X16,
            .vcm = ADC_VCM_SEL_205,
            .vref = ADC_VREF_2500,
        },
    },
    {
        {
            .ratio = 5,
            .adc_buf_en = false,
            .adc_buf_bypass = true,
            .adc_pag_en = false,
            .adc_pag_bypass = true,
            .vcr_enable = false,
            .vcr = ADC_VCR_SEL_437_45,
            .gain = ADC_GAIN_X16,
            .vcm = ADC_VCM_SEL_205,
            .vref = ADC_VREF_2500,
        },
    },
    {
        {
            .ratio = 5,
            .adc_buf_en = false,
            .adc_buf_bypass = true,
            .adc_pag_en = false,
            .adc_pag_bypass = true,
            .vcr_enable = false,
            .vcr = ADC_VCR_SEL_437_45,
            .gain = ADC_GAIN_X16,
            .vcm = ADC_VCM_SEL_205,
            .vref = ADC_VREF_2500,
        },
    },
};

/**
 * @brief 默认VBAT/VTEMP ADC参数配置表（TCPL01X）
 * @note  [0] = VTEMP通道, [1] = VBAT通道
 */
#else
adc_cfg_t default_vbvt_param_table[2] =
{
    {
        .gain1 = ADC_GAIN1_X20,
        .gain2 = ADC_GAIN2_X1, //VTEMP_ADC_GAIN
        .vcr = ADC_VCR_365,
        .vref = ADC_VREF_2035,
        .op_sel = ADC_OP_GAIN0,
    },
    {
        .gain1 = ADC_GAIN1_X5,
        .gain2 = ADC_GAIN2_X2, //VBAT_ADC_GAIN
        .vcr = ADC_VCR_365,
        .vref = ADC_VREF_2035,
        .op_sel = ADC_OP_GAIN0,
    },
};
/**
 * @brief RGB各通道ADC参数配置表（TCPL01X，支持3种串数配置）
 * @note  按[RGB_RED/GREEN/BLUE][SERIAL_1/2/3]索引
 *        SERIAL_1/2使用VB通道增益，SERIAL_3使用直接增益
 */
const adc_cfg_t seft_check_param_table[RGB_TYPE_MAX][LED_MEAS_SERIAL_MAX] =
{
    {
        {
            .gain1 = ADC_GAIN1_X17,
            .gain2 = ADC_GAIN2_X8, //VB_R_ADC_GAIN
            .vcr = ADC_VCR_668,
            .vref = ADC_VREF_1528,
            .op_sel = ADC_OP_GAIN0_1,

        },
        {
            .gain1 = ADC_GAIN1_X10,
            .gain2 = ADC_GAIN2_X2, //VB_R_ADC_GAIN
            .vcr = ADC_VCR_365,
            .vref = ADC_VREF_1528,
            .op_sel = ADC_OP_GAIN0_1,
        },
        {
            .gain1 = ADC_GAIN1_X2,
            .gain2 = ADC_GAIN2_X1, //R_ADC_GAIN
            .vcr = ADC_VCR_365,
            .vref = ADC_VREF_2035,
            .op_sel = ADC_OP_GAIN0,
        },
    },
    {
        {
            .gain1 = ADC_GAIN1_X15,
            .gain2 = ADC_GAIN2_X4, //VB_G_ADC_GAIN
            .vcr = ADC_VCR_820,
            .vref = ADC_VREF_1528,
            .op_sel = ADC_OP_GAIN0_1,
        },
        {
            .gain1 = ADC_GAIN1_X5,
            .gain2 = ADC_GAIN2_X4, //VB_G_ADC_GAIN
            .vcr = ADC_VCR_365,
            .vref = ADC_VREF_1528,
            .op_sel = ADC_OP_GAIN0_1,
        },
        {
            .gain1 = ADC_GAIN1_X2,
            .gain2 = ADC_GAIN2_X1, //G_ADC_GAIN
            .vcr = ADC_VCR_365,
            .vref = ADC_VREF_2035,
            .op_sel = ADC_OP_GAIN0,
        },
    },
    {
        {
            .gain1 = ADC_GAIN1_X12,
            .gain2 = ADC_GAIN2_X8, //VB_B_ADC_GAIN
            .vcr = ADC_VCR_719,
            .vref = ADC_VREF_1528,
            .op_sel = ADC_OP_GAIN0_1,
        },
        {
            .gain1 = ADC_GAIN1_X5,
            .gain2 = ADC_GAIN2_X3, //VB_B_ADC_GAIN
            .vcr = ADC_VCR_365,
            .vref = ADC_VREF_1528,
            .op_sel = ADC_OP_GAIN0_1,
        },
        {
            .gain1 = ADC_GAIN1_X2,
            .gain2 = ADC_GAIN2_X1, //G_ADC_GAIN
            .vcr = ADC_VCR_365,
            .vref = ADC_VREF_2035,
            .op_sel = ADC_OP_GAIN0,
        },
    },
};

#endif
