#ifndef FFF_HARDWORD_H
#define FFF_HARDWORD_H
#include <stdint.h>
#include <stdio.h>
#include <fff.h>




DECLARE_FAKE_VOID_FUNC(lin_lld_isr_callback,uint32_t);

DECLARE_FAKE_VOID_FUNC(NVIC_DisableIRQ,uint32_t);
DECLARE_FAKE_VOID_FUNC(__set_MSP,uint32_t);
DECLARE_FAKE_VOID_FUNC(NVIC_SystemReset);
DECLARE_FAKE_VALUE_FUNC(uint8_t,lin_get_uds_nad);
// DECLARE_FAKE_VALUE_FUNC(uint8_t,cpmpare_key,uint8_t *, uint8_t *, uint8_t );

#endif

