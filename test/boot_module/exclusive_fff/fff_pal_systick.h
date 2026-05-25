#ifndef __FFF_PAL_SYSTICK_H__
#define __FFF_PAL_SYSTICK_H__

#include "fff.h"
#include <stdint.h>

// DECLARE_FAKE_VALUE_FUNC(bool, ll_flash_erase_drv, uint8_t, uint32_t, uint32_t);

DECLARE_FAKE_VALUE_FUNC(uint32_t,systick_count_get);
DECLARE_FAKE_VALUE_FUNC(uint32_t,systick_diff,uint32_t);
DECLARE_FAKE_VOID_FUNC(delay_ms, uint32_t);
DECLARE_FAKE_VOID_FUNC(delay_us, uint32_t);

// uint32_t systick_count_get(void);
// uint32_t systick_diff(uint32_t start_tick);
// void delay_ms(uint32_t ms);
// void delay_us(uint32_t us);


#endif