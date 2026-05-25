#include "fff_tcpl03x_ll_sci.h"
DEFINE_FAKE_VOID_FUNC(ll_sci_deinit,ll_sci_bus_e);
DEFINE_FAKE_VOID_FUNC(ll_sci_init,ll_sci_bus_e,sci_config_t*,ISR_FUNC_CALLBACK);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_baudrate_config,ll_sci_bus_e,uint32_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_isr_enabl,ll_sci_bus_e,bool);
DEFINE_FAKE_VOID_FUNC(ll_sci_state_clear,ll_sci_bus_e,ll_sci_clear_type_e);

DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_rx_delay_set,ll_sci_bus_e,uint8_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_wakeup_enable,ll_sci_bus_e,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_aa_enable,ll_sci_bus_e,lin_aa_type_e,bool,uint16_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_aa_disable,ll_sci_bus_e);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_aa_ready_set,ll_sci_bus_e,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_transmit,ll_sci_bus_e ,uint8_t * ,uint16_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_receive,ll_sci_bus_e ,uint8_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_transmit,ll_sci_bus_e,uint8_t,uint8_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_receive,ll_sci_bus_e,uint8_t,uint8_t *,uint16_t);

DEFINE_FAKE_VALUE_FUNC(uint8_t,ll_lin_checksum_calib_func,uint8_t,uint8_t *,uint16_t);

DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_pid_read,ll_sci_bus_e,uint8_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_read_byte,ll_sci_bus_e,uint8_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_auto_baudrate_read,ll_sci_bus_e,uint32_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_baudrate_read,ll_sci_bus_e,uint32_t *);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_ctrl_glben,ll_sci_bus_e,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_ctrl_rx_abort,ll_sci_bus_e,bool);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_ctrl_brk_tx,ll_sci_bus_e,uint8_t);
DEFINE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_tx_header,ll_sci_bus_e,uint8_t);