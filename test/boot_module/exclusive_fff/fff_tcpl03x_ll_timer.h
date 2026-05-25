#ifndef __FFF_TCPL03X_LL_TIMER_H__
#define __FFF_TCPL03X_LL_TIMER_H__



#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_def.h"
#else
    #include "tcpl03x_ll_def.h"
#endif

#include "fff.h"


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
    ll_clk_config_t clk_cfg;
    ll_isr_config_t isr_cfg;
    uint16_t initial_value; /*!< Specifies the initial decrement alue, the timer will decremnt from this value.
                                This parameter can be any value between 0x00 and 0xFFFF */

    bool repeat_disable;    /*!< if loop is disabled, the timer will decrement N times
                                N is specified by loop_repeat_counts*/
    bool trigger_mode;
} timer_config_t;


DECLARE_FAKE_VOID_FUNC(ll_timer_deinit);
DECLARE_FAKE_VOID_FUNC(ll_timer_init,timer_config_t * ,ISR_FUNC_CALLBACK);

DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_timer_isr_enable,bool);

DECLARE_FAKE_VALUE_FUNC(bool,ll_timer_isr_get);
DECLARE_FAKE_VOID_FUNC(ll_timer_trig_enable,bool);
DECLARE_FAKE_VOID_FUNC(ll_timer_enable,bool);
DECLARE_FAKE_VOID_FUNC(ll_timer_counter_set,uint16_t);

DECLARE_FAKE_VALUE_FUNC(uint16_t,ll_timer_counter_get);


#if defined(__cplusplus)
}
#endif
#endif /* __TCPL03X_LL_TIMER_H__ */
