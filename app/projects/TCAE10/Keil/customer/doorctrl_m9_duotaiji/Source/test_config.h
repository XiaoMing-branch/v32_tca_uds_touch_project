#ifndef __TEST_CONFIG_H__
#define __TEST_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_TEST_MODE
    #define STATIC 
#else
    #define STATIC static
#endif

#ifdef __cplusplus
}
#endif
#endif
