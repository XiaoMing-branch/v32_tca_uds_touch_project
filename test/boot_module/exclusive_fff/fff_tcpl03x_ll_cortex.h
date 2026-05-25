#ifndef __FFF_TCPL03X_LL_CORTEX_H__
#define __FFF_TCPL03X_LL_CORTEX_H__

#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x.h"
#else
    #include "tcpl03x.h"
#endif

#include "fff.h"


#ifdef __cplusplus
extern "C"
{
#endif

DECLARE_FAKE_VOID_FUNC(EnableNvic,uint32_t,uint8_t,bool);

#ifdef __cplusplus
}
#endif
#endif /* __TCPL03X_LL_CORTEX_H__ */
