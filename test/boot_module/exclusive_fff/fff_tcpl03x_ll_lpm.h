#ifndef __FFF_TCPL03X_LL_LPM_H__
#define __FFF_TCPL03X_LL_LPM_H__



#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_def.h"
#else
    #include "tcpl03x_ll_def.h"
#endif

#include "fff.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define SLPMODE_IDLE            0
#define SLPMODE_SLEEP           1
#define SLPMODE_SLEEPWALK       2
#define SLPMODE_DEEPSLEEP       3

typedef enum
{
    IDLE_MODE       = 0,
    SLEEP_MODE      = 1,
    SLEEPWALK_MODE  = 2,
    DEEPSLEEP_MODE  = 3,
    SLEEP_MODE_MAX  = 4
} sleep_mode_e;


DECLARE_FAKE_VOID_FUNC(ll_lpm_mcu_enter,sleep_mode_e,bool);
DECLARE_FAKE_VOID_FUNC(ll_lpm_afe_enter,sleep_mode_e);
DECLARE_FAKE_VOID_FUNC(ll_pmu_gpio_lowpower);
DECLARE_FAKE_VOID_FUNC(ll_pmu_ldo_dummy_enable,bool);

#ifdef __cplusplus
}
#endif
#endif /* __TCPL03X_LL_LPM_H__ */
