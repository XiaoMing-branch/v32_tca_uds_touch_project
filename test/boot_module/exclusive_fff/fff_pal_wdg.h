#ifndef __FFF_PAL_WDG_H__
#define __FFF_PAL_WDG_H__

#include "fff.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_FAKE_VOID_FUNC(wdg_init, uint16_t);
DECLARE_FAKE_VOID_FUNC(wdg_reload);
DECLARE_FAKE_VOID_FUNC(wdg_enable, bool);

// void wdg_init(uint16_t cnt_ms);
// void wdg_reload(void);
// void wdg_enable(bool enable);

#ifdef __cplusplus
}
#endif
#endif
