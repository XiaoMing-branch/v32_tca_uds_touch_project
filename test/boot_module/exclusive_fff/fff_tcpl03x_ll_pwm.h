 
#ifndef __FFF_TCPL03X_LL_PWM_H__
#define __FFF_TCPL03X_LL_PWM_H__

#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_def.h"
#else
    #include "tcpl03x_ll_def.h"
#endif

#include "fff.h"


#if defined(__cplusplus)
extern "C" {
#endif

#define PWM_INT_CH0_CNT_MAX         0x00000002               //Mask PWM channel0 counter max overflow interrupt
#define PWM_INT_CH0_PWM_SET         0x00000020            //Mask PWM channel0 PWM output set interrupt
#define PWM_INT_CH0_PWM_CLR         0x00000040            //Mask PWM channel0 PWM output clear interrupt
#define PWM_INT_CH1_CNT_MAX         0x00000002               //Mask PWM channel1 counter max overflow interrupt
#define PWM_INT_CH1_PWM_SET         0x00000080            //Mask PWM channel1 PWM output set interrupt
#define PWM_INT_CH1_PWM_CLR         0x00000100            //Mask PWM channel1 PWM output clear interrupt
#define PWM_INT_CH2_CNT_MAX         0x00000002               //Mask PWM channel2 counter max overflow interrupt
#define PWM_INT_CH2_PWM_SET         0x00000200            //Mask PWM channel2 PWM output set interrupt
#define PWM_INT_CH2_PWM_CLR         0x00000400

#define PWM_INT_CH3_CNT_MAX         0x00000008                //Mask PWM channel2 counter max overflow interrupt
#define PWM_INT_CH3_PWM_SET         0x00000800            //Mask PWM channel2 PWM output set interrupt
#define PWM_INT_CH3_PWM_CLR         0x00001000

#define PWM_INIT_FLAG   (PWM_INT_CH0_PWM_CLR | PWM_INT_CH0_CNT_MAX | \
                              PWM_INT_CH1_PWM_CLR | PWM_INT_CH1_CNT_MAX | \
                              PWM_INT_CH2_PWM_CLR | PWM_INT_CH2_CNT_MAX)


#define PWM_INIT_ALL_FLAG     (0xFFFFUL)

/**
  * @brief  ll sci bus enumeration
  */
typedef enum
{
    LL_PWM_BUS_0 = 0,
    LL_PWM_BUS_MAX,
} ll_pwm_bus_e;

/**
  * @brief  ll uart bus enumeration
  */
typedef enum
{
    HVIO_MODE_LED = 0,
    HVIO_MODE_PWM
} pwm_hvio_mode_e;

/**
  * @brief  ll pwm channel enumeration
  */
typedef enum
{
    PWM_CHANNEL_0 = 0,
    PWM_CHANNEL_1,
    PWM_CHANNEL_2,
    PWM_CHANNEL_MAX
} pwm_channel_e;

/**
  * @brief  ll pwm channel enumeration
  */
typedef enum
{
    PWM_MODE_DOUBLE_PERIOD = 0,
    PWM_MODE_INDEPENDENT,       //independent mode
    PWM_MODE_SOFTWARE,          //Software mode
    PWM_MODE_FIXED,
    PWM_MODE_TYPE_MAX
} pwm_outmode_e;

/**
  * @brief  ll pwm channel enumeration
  */
typedef enum
{
    PWM_POLARITY_HIGH = 0,     //high level is valid
    PWM_POLARITY_LOW,
    PWM_POLARITY_MAX
} pwm_polarity_e;

/**
  * @brief  led fall sr time enumeration
  */
typedef enum
{
    LED_FALL_SR_TIME_81NS = 0,
    LED_FALL_SR_TIME_200NS,
    LED_FALL_SR_TIME_400NS,
    LED_FALL_SR_TIME_660NS,
    LED_FALL_SR_TIME_MAX,
} led_fall_sr_time_e;

/**
  * @brief  led rise sr time enumeration
  */
typedef enum
{
    LED_RISE_SR_TIME_116NS = 0,
    LED_RISE_SR_TIME_200NS,
    LED_RISE_SR_TIME_400NS,
    LED_RISE_SR_TIME_630NS,
    LED_RISE_SR_TIME_MAX,
} led_rise_sr_time_e;

/**
  * @brief  led pwm counter mode enumeration
  */
typedef enum
{
    PWM_COUNTER_MODE_UP = 0,
    PWM_COUNTER_MODE_UP_DOWN,
    PWM_COUNTER_MODE_MAX,
} pwm_counter_mode_e;

/**
  * @brief  led pwm sync mode enumeration
  */
typedef enum
{
    PWM_SYNC_MODE_NULL = 0,         /*!< no Synchronization */
    PWM_SYNC_MODE_CH0_CH1,          /*!< Sync channel 0~1 */
    PWM_SYNC_MODE_CH0_CH1_CH2,      /*!< Sync channel 0~2 */
    PWM_SYNC_MODE_CH0_CH1_CH2_CH3,  /*!< Sync channel 0~3 */
    PWM_SYNC_MODE_MAX,
} pwm_sync_mode_e;

/**
  * @brief  led driver current enumeration
  */
typedef enum
{
    LED_DRIVER_5MA = 0,
    LED_DRIVER_10MA,
    LED_DRIVER_15MA,
    LED_DRIVER_20MA,
    LED_DRIVER_25MA,
    LED_DRIVER_30MA,
    LED_DRIVER_35MA,
    LED_DRIVER_40MA,
    LED_DRIVER_45MA,
    LED_DRIVER_MAX,
} led_driver_current_e;

/**
  * @brief  led diag current enumeration
  */
typedef enum
{
    LED_DIAG_MAX,
} led_diag_current_e;

typedef struct
{
    ll_clk_config_t     clk_cfg;
    ll_isr_config_t     isr_cfg;

    pwm_counter_mode_e  count_mode;
    pwm_sync_mode_e     sync_mode;
    uint16_t            period;

    pwm_outmode_e       out_mode;

    pwm_polarity_e      polarity; /*!1'b0 : PWM channel0 output 1'b1 valid; 1'b1 : PWM channel0 output 1'b0 valid; */

    pwm_hvio_mode_e     hvio_mode;

} pwm_config_t;


DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_isr_flag_enable,ll_pwm_bus_e,uint32_t,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_isr_enable,ll_pwm_bus_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_isr_clear,ll_pwm_bus_e,uint32_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_isr_flag_get,ll_pwm_bus_e,uint32_t *);
DECLARE_FAKE_VOID_FUNC(ll_pwm_enable,pwm_hvio_mode_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_channel_threshold_config,pwm_channel_e,uint16_t,uint16_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_channel_period_config,pwm_channel_e,uint16_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_high_threshold_get,pwm_channel_e,uint16_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_channel_counter_get,pwm_channel_e,uint16_t *);
DECLARE_FAKE_VOID_FUNC(ll_pwm_break_set,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_status_get,pwm_channel_e,uint8_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_driver_current_config,pwm_channel_e,led_driver_current_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_driver_current_get,pwm_channel_e,led_driver_current_e *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_diag_current_config,pwm_channel_e,led_diag_current_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_diagnose_enable,pwm_channel_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_exchannel_enable,ll_pwm_bus_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_exchannel_init,ll_pwm_bus_e,uint16_t,pwm_polarity_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_switch_init,ll_pwm_bus_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_led_fall_rise_time_set,pwm_channel_e,led_rise_sr_time_e,led_fall_sr_time_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_deinit,ll_pwm_bus_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_pwm_init,ll_pwm_bus_e,pwm_config_t *,ISR_FUNC_CALLBACK);


#if defined(__cplusplus)
}
#endif
#endif /* __TCPL03X_LL_PWM_H__ */
