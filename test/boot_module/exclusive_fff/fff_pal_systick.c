#include "fff_pal_systick.h"

DEFINE_FAKE_VALUE_FUNC(uint32_t,systick_count_get);
DEFINE_FAKE_VALUE_FUNC(uint32_t,systick_diff,uint32_t);
DEFINE_FAKE_VOID_FUNC(delay_ms, uint32_t);
DEFINE_FAKE_VOID_FUNC(delay_us, uint32_t);