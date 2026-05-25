#include "fff_tcae10_ll_captouch.h"
DEFINE_FAKE_VOID_FUNC(CapTouch_InterruptEnable, uint8_t);
DEFINE_FAKE_VOID_FUNC(CapTouch_InterruptDisable, uint8_t);
DEFINE_FAKE_VOID_FUNC(CapTouch_InterruptClear, uint8_t);
DEFINE_FAKE_VOID_FUNC(CapTouch_Enable, FunctionalState);
DEFINE_FAKE_VOID_FUNC(CapTouch_Init, CapTouch_InitConfig_t *);
DEFINE_FAKE_VOID_FUNC(CapTouch_SoftwareTrig);
DEFINE_FAKE_VOID_FUNC(CapTouch_Hopping, uint8_t);
DEFINE_FAKE_VALUE_FUNC(bool, CapTouch_InterruptStatusGet, uint8_t);
