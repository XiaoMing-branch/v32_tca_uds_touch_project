 #include "fff_tcpl03x_ll_lpm.h"
 
DEFINE_FAKE_VOID_FUNC(ll_lpm_mcu_enter,sleep_mode_e,bool);
DEFINE_FAKE_VOID_FUNC(ll_lpm_afe_enter,sleep_mode_e);
DEFINE_FAKE_VOID_FUNC(ll_pmu_gpio_lowpower);
DEFINE_FAKE_VOID_FUNC(ll_pmu_ldo_dummy_enable,bool);