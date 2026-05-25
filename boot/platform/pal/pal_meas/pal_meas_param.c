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
 * @brief  ADC默认电压/温度参数表
 * @note   索引0:温度传感器, 索引1:电池电压
 *         TCPL03X使用ratio/buf/pag/vcr/gain/vcm/vref参数
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

const adc_cfg_t seft_check_param_table[LED_TYPE_MAX][LED_MEAS_SERIAL_MAX] =
{
    {
        {
            .ratio = 5,
            .adc_buf_en = true,
            .adc_buf_bypass = false,
            .adc_pag_en = true,
            .adc_pag_bypass = true,
            .vcr_enable = false,
            .vcr = ADC_VCR_SEL_437_45,
            .gain = ADC_GAIN_X1,
            .vcm = ADC_VCM_SEL_205,
            .vref = ADC_VREF_2500,
        },
    },
    {
        {
            .ratio = 5,
            .adc_buf_en = true,
            .adc_buf_bypass = false,
            .adc_pag_en = true,
            .adc_pag_bypass = true,
            .vcr_enable = false,
            .vcr = ADC_VCR_SEL_437_45,
            .gain = ADC_GAIN_X1,
            .vcm = ADC_VCM_SEL_205,
            .vref = ADC_VREF_2500,
        },
    },
    {
        {
            .ratio = 5,
            .adc_buf_en = true,
            .adc_buf_bypass = false,
            .adc_pag_en = true,
            .adc_pag_bypass = true,
            .vcr_enable = false,
            .vcr = ADC_VCR_SEL_437_45,
            .gain = ADC_GAIN_X1,
            .vcm = ADC_VCM_SEL_205,
            .vref = ADC_VREF_2500,
        },
    },
};

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
const adc_cfg_t seft_check_param_table[LED_TYPE_MAX][LED_MEAS_SERIAL_MAX] =
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
