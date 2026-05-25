#include "fff_pal_lin_comm.h"

DEFINE_FAKE_VOID_FUNC(pal_lin_init,lin_bus_e,lin_mode_e,uint32_t,ISR_FUNC_CALLBACK);
DEFINE_FAKE_VOID_FUNC(pal_lin_deinit,lin_bus_e);
DEFINE_FAKE_VOID_FUNC(pal_lin_rx_response,lin_bus_e,uint8_t,uint8_t *,uint8_t);
DEFINE_FAKE_VALUE_FUNC(bool,pal_lin_tx_response,lin_bus_e,uint8_t,uint8_t *,uint8_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_tx_4byte,lin_bus_e,uint8_t *,uint8_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_tx_header,lin_bus_e,uint8_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_aa_enter,uint16_t *);
DEFINE_FAKE_VOID_FUNC(pal_lin_aa_exit);
DEFINE_FAKE_VOID_FUNC(pal_lin_aa_ready);
DEFINE_FAKE_VALUE_FUNC(uint8_t,pal_lin_parity_calib,lin_parity_type_e,uint8_t);
DEFINE_FAKE_VALUE_FUNC(uint8_t,pal_lin_checksum_calib,uint8_t,uint8_t *);
DEFINE_FAKE_VALUE_FUNC(uint16_t,pal_lin_aa_raw_code_get,uint16_t *,uint16_t);
DEFINE_FAKE_VOID_FUNC(pal_lin_abort_handle,lin_bus_e,lin_abort_type_e);
DEFINE_FAKE_VOID_FUNC(pal_lin_read_byte,lin_bus_e,lin_read_type_e,uint8_t *);
DEFINE_FAKE_VOID_FUNC(pal_lin_autobaudrate_check,lin_bus_e);