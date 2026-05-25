#include "fff_pal_lin_tl.h"

DEFINE_FAKE_VOID_FUNC(lin_tl_init);
DEFINE_FAKE_VALUE_FUNC(bool,lin_uds_send,uint8_t,uint8_t *,uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool,lin_uds_negative_response,uint8_t,uint8_t,uint8_t);
DEFINE_FAKE_VALUE_FUNC(bool,lin_uds_receive,uint8_t,uint8_t *,uint16_t *);
DEFINE_FAKE_VALUE_FUNC(bool,lin_tl_uncd_frame_get,lin_bus_e,uint8_t *,uint8_t *);