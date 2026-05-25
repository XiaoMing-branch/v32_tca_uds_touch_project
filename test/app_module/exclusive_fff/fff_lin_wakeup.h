/**
 *****************************************************************************
 * @brief   lin wakeup header file.
 *
 * @file    lin_wakeup.h
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

#ifndef __FFF_LIN_WAKEUP_H__
#define __FFF_LIN_WAKEUP_H__

#ifdef __cplusplus
extern "C"
{
#endif

DECLARE_FAKE_VOID_FUNC(system_low_power_init,uint32_t,uint32_t);
DECLARE_FAKE_VOID_FUNC(sleep_mode_enter,uint32_t,uint32_t);

#ifdef __cplusplus
}
#endif
#endif
