/**
 *****************************************************************************
 * @brief   adc driver source file.
 *
 * @file    tcae10_ll_adc.c
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

#include "fff_tcae10_ll_adc.h"
#include "fff_system_tcae10.h"
// #include "fff_tcae10_ll_cortex.h"
#include "fff_tcae10_ll_flash.h"

DEFINE_FAKE_VOID_FUNC(ll_adc_deinit);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_init, adc_config_t *, ISR_FUNC_CALLBACK);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_vref_config, adc_vref_e);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_gain_config, adc_channel_e, adc_cfg_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_isr_enable, bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_lin_aa_enable, lin_aa_type_e, bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_select_channel, adc_channel_e, adc_cfg_t *);
DEFINE_FAKE_VALUE_FUNC(uint8_t, ll_adc_fifo_length_get);
DEFINE_FAKE_VALUE_FUNC(uint16_t, ll_adc_fifo_get, uint16_t *, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint16_t, ll_adc_fifo_clear);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_crtl_config, uint32_t *, bool);
DEFINE_FAKE_VOID_FUNC(ll_adc_softwart_start, bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_vf_calculate_func, adc_channel_e, uint16_t *, uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_volt_calculate_func, int16_t, adc_cfg_t *, uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_vbat_calculate_func, int16_t, uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_temp_calculate_func, temp_channel_e, int16_t, adc_cfg_t *, uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_channnel_start, adc_channel_e, adc_cfg_t *, uint16_t *, uint8_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_adc_tsensor_enable, bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e, ll_bias_control_enable, bool);

#define ADC_ISR_FLAG (0x1FUL)

#define ADDR_TEMP_TRIM_PARAM 0x00800044UL
#define ADDR_VBAT_TRIM_PARAM 0x00800038UL

const uint16_t vcr_value[ADC_VCR_MAX] = {236, 314, 394, 437, 552, 631, 710, 789};
const uint16_t vref_value[ADC_VREF_MAX] = {2500, 2000, 1500};

static uint32_t adc_isr_flag = 0;
bool adc_isr_enable = false;
static ISR_FUNC_CALLBACK adc_isr_callback = NULL;

typedef struct
{
    uint32_t code1 : 16;
    uint32_t code2 : 16; // only for temp
} trim_value_t;

typedef struct
{
    float tos_value;
    float k_value;
} temp_trim_value_t;

typedef struct
{
    float a;
    float b;
    float c;
} vaon_coef_t;

typedef struct
{
    uint16_t gain;
    int16_t offset;
} adc_ch_trim_t;

trim_value_t vbat_trim_value = {0};          // 未初始化，补充为0
temp_trim_value_t vtemp_temp_value[2] = {0}; // 未初始化，补充为0
vaon_coef_t vaon_coef_value = {0};           // 未初始化，补充为0
adc_ch_trim_t vf_ch_trim_value[3] = {0};     // 未初始化，补充为0
adc_ch_trim_t aon_ch_trim_value = {0};       // 未初始化，补充为0
