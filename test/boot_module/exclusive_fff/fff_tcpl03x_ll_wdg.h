#ifndef __FFF_TCPL03X_LL_WDG_H__
#define __FFF_TCPL03X_LL_WDG_H__


#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_def.h"
#else
    #include "tcpl03x_ll_def.h"
#endif

#include "fff.h"


#if 1
#define WDG_UNLCOK() (void *)0
#define WDG_LCOK()   (void *)0
#else
#define WDG_UNLCOK()
#define WDG_LCOK()
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
    ll_clk_config_t clk_cfg;
    ll_isr_config_t isr_cfg;
    bool   reset_on_overflow;  //reset on overflow, otherwise will generate an interrupt
    uint32_t    max_count;          //the max count the wdg will count from 0xFFF
} wdg_config_t;


DECLARE_FAKE_VOID_FUNC(ll_wdg_deinit);
DECLARE_FAKE_VOID_FUNC(ll_wdg_init,wdg_config_t *,ISR_FUNC_CALLBACK);
DECLARE_FAKE_VOID_FUNC(ll_wdg_isr_enable,bool);
DECLARE_FAKE_VOID_FUNC(ll_wdg_enable,bool);
DECLARE_FAKE_VOID_FUNC(ll_wdg_reload);


#if defined(__cplusplus)
}
#endif
#endif /* __TCPL03X_LL_WDG_H__ */
