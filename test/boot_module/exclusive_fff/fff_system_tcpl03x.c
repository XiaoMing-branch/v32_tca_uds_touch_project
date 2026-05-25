#include "fff_system_tcpl03x.h"

DEFINE_FAKE_VOID_FUNC(sys_core_clock_update);
DEFINE_FAKE_VALUE_FUNC(uint32_t,sys_hclk_freq_get); 
DEFINE_FAKE_VALUE_FUNC(uint32_t,sys_pclk_freq_get);
DEFINE_FAKE_VOID_FUNC(SystemInit);
DEFINE_FAKE_VOID_FUNC(system_remap_config, uint32_t *, bool *);