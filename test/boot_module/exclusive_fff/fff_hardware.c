#include "fff_hardware.h"

DEFINE_FAKE_VOID_FUNC(lin_lld_isr_callback,uint32_t);

DEFINE_FAKE_VOID_FUNC(NVIC_DisableIRQ,uint32_t);
DEFINE_FAKE_VOID_FUNC(__set_MSP,uint32_t);
DEFINE_FAKE_VOID_FUNC(NVIC_SystemReset);
DEFINE_FAKE_VALUE_FUNC(uint8_t,lin_get_uds_nad);
// DEFINE_FAKE_VALUE_FUNC(uint8_t,cpmpare_key,uint8_t *, uint8_t *, uint8_t );
