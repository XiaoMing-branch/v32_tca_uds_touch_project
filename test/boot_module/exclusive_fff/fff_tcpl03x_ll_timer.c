
#include "fff_tcpl03x_ll_timer.h"

DEFINE_FAKE_VOID_FUNC(ll_timer_deinit);
DEFINE_FAKE_VOID_FUNC(ll_timer_init,timer_config_t * ,ISR_FUNC_CALLBACK);

DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_timer_isr_enable,bool);

DEFINE_FAKE_VALUE_FUNC(bool,ll_timer_isr_get);
DEFINE_FAKE_VOID_FUNC(ll_timer_trig_enable,bool);
DEFINE_FAKE_VOID_FUNC(ll_timer_enable,bool);
DEFINE_FAKE_VOID_FUNC(ll_timer_counter_set,uint16_t);

DEFINE_FAKE_VALUE_FUNC(uint16_t,ll_timer_counter_get);