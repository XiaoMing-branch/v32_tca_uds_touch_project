#include "fff_touch_haldispatch.h"

DEFINE_FAKE_VOID_FUNC(Touch_HalDispatch_SetFastScan, TOUCH_HalDispatch_Type *, uint8_t);
DEFINE_FAKE_VALUE_FUNC(TOUCH_HalDispatch_Type *, Touch_HalDispatchOnlyTouchCreate, TOUCH_HalDispatch_OnlyTouch_Type *, const TOUCH_HalScanerTable_Type *, const TOUCH_HalDispatch_OnlyTouchPara_Type *);
DEFINE_FAKE_VOID_FUNC(Touch_HalDispatch_DiscardSampData, int);
