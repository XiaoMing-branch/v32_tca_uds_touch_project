#include "fff_tcpl03x_ll_gpio.h"

DEFINE_FAKE_VOID_FUNC(ll_gpio_deinit);
DEFINE_FAKE_VOID_FUNC(ll_gpio_init,gpio_config_t *,ISR_FUNC_CALLBACK);
DEFINE_FAKE_VALUE_FUNC(bool,ll_gpio_read,gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_output,gpio_pin_e,bool);
DEFINE_FAKE_VOID_FUNC(ll_gpio_toggle,gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_isr_enable,gpio_pin_e,bool);
DEFINE_FAKE_VALUE_FUNC(bool,ll_gpio_interrupt_flag_get,gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_interrupt_clear,gpio_pin_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_afio_config,gpio_pin_e,gpio_afio_mux_e);
DEFINE_FAKE_VOID_FUNC(ll_gpio_ano_lpm_config,gpio_pin_e,bool);
DEFINE_FAKE_VOID_FUNC(ll_gpio_reset_enable,bool);