#include "fff_tcpl03x_ll_wdg.h"

DEFINE_FAKE_VOID_FUNC(ll_wdg_deinit);
DEFINE_FAKE_VOID_FUNC(ll_wdg_init,wdg_config_t *,ISR_FUNC_CALLBACK);
DEFINE_FAKE_VOID_FUNC(ll_wdg_isr_enable,bool);
DEFINE_FAKE_VOID_FUNC(ll_wdg_enable,bool);
DEFINE_FAKE_VOID_FUNC(ll_wdg_reload);