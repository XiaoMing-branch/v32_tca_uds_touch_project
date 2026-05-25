// #include "si_include.h"
#include "fff_touch_config.h"
// #include "app.h"
// #include "tc_halt.h"
// #include "si_touch_port.h"
// #include "touch_tool.h"
// #include "tc_log.h"

static const char *TAG = "TOUCH_CONFIG";

//**************************************************************************************************
//***IOTouch配置

DEFINE_FAKE_VOID_FUNC(TouchConfig, T_SiAlgoObject *);
DEFINE_FAKE_VOID_FUNC(TouchEnterHaltHook);
DEFINE_FAKE_VOID_FUNC(TouchWakeupHook);
DEFINE_FAKE_VOID_FUNC(TouchKeyForceSyncBaseline);
DEFINE_FAKE_VOID_FUNC(TouchKeyCallback, uint8_t, T_SiKeyStatus);
DEFINE_FAKE_VOID_FUNC(TouchGetRaw, T_SiData *, int);
DEFINE_FAKE_VOID_FUNC(TouchGetBaseline, T_SiData *, int);
DEFINE_FAKE_VOID_FUNC(TouchGetDiff, T_SiData *, int);
