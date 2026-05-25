/**
 *****************************************************************************
 * @brief   logging header file.
 * @file    logging.h
 * @author  AE/FAE team
 * @date    2024.01.01
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.</b>
 *
 *****************************************************************************
 */

#ifndef __FFF_LOGGING_H__
#define __FFF_LOGGING_H__

#include "fff.h"

// #include "tc_printf.h"
// #include "pal_func_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if CFG_SUPPORT_LOG
#define log_debug(format, args ...)  do{tc_printf("[D]"format, ## args); }while(0)
#define log_info(format, args ...)   do{tc_printf("[I]"format, ## args); }while(0)
#define log_warn(format, args ...)   do{tc_printf("[W]"format, ## args); }while(0)
#define log_err(format, args ...)    do{tc_printf("[E]"format, ## args); }while(0)
#else
#define log_debug(format, args ...)
#define log_info(format, args ...)
#define log_warn(format, args ...)
#define log_err(format, args ...)
#endif

DECLARE_FAKE_VOID_FUNC(logging_init);
DECLARE_FAKE_VOID_FUNC(logging_deinit);

// void logging_init(void);
// void logging_deinit(void);

#ifdef __cplusplus
}
#endif
#endif
