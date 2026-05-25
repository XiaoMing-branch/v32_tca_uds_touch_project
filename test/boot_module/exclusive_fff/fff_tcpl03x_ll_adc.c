#include "fff_tcpl03x_ll_adc.h"


DEFINE_FAKE_VOID_FUNC(ll_adc_deinit);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_init,adc_config_t *,ISR_FUNC_CALLBACK);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_vref_config,adc_vref_e);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_gain_config,adc_channel_e,adc_cfg_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_isr_enable,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_lin_aa_enable,lin_aa_type_e,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_select_channel,adc_channel_e,adc_cfg_t *);
DEFINE_FAKE_VALUE_FUNC(uint8_t,ll_adc_fifo_length_get);
DEFINE_FAKE_VALUE_FUNC(uint16_t,ll_adc_fifo_get,uint16_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint16_t,ll_adc_fifo_clear);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_crtl_config,uint32_t *,bool);
DEFINE_FAKE_VOID_FUNC(ll_adc_softwart_start,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_it_start,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_vf_calculate_func,adc_channel_e,uint16_t *,uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_volt_calculate_func,int16_t,adc_cfg_t *,uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_vbat_calculate_func,int16_t,uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_temp_calculate_func,temp_channel_e,int16_t,adc_cfg_t *,uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_channnel_start,adc_channel_e,adc_cfg_t *,uint16_t *,uint8_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_adc_tsensor_enable,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_bias_control_enable,bool);
