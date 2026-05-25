#include "fff_touch_halnode.h"
DEFINE_FAKE_VOID_FUNC(Touch_HalInterface_SetReady, TOUCH_HalInterface_Type *, int);
DEFINE_FAKE_VALUE_FUNC(TOUCH_HalInterface_Type *, Touch_HalChargeCreate, TOUCH_HalCharge_Type *, const TOUCH_HalConfig_Type *, const TOUCH_HalAdcConfig_Type *);
