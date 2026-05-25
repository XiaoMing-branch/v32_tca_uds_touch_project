/**
 ******************************************************************************
 *       Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE  DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************
 */

#include "fff_tcae10.h"
#include "fff_system_tcae10.h"

#if defined (__ARMCC_VERSION) /* Keil ��Vision 5.29.0.0 */

    extern uint32_t Load$$RW_IRAM1$$Base;     /* Load Address of DDR_RW_DATA region*/
    extern uint32_t Image$$RW_IRAM1$$Base;    /* Exec Address of DDR_RW_DATA region*/
    extern uint32_t Image$$RW_IRAM1$$Length;  /* Length of DDR_RW_DATA region*/
    extern uint32_t Image$$RW_IRAM1$$ZI$$Base;
    extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;
#endif

#ifndef CFG_HCLK_CLOCK          //配置hclk时钟
    #define CFG_HCLK_CLOCK DEFAULT_SYSTEM_CLOCK
#endif

uint32_t SystemCoreClock = DEFAULT_SYSTEM_CLOCK;

DEFINE_FAKE_VALUE_FUNC(uint32_t,GetFclkVal);
DEFINE_FAKE_VOID_FUNC(SystemCoreClockUpdate);
DEFINE_FAKE_VALUE_FUNC(uint32_t, SystemGetHClkFreq);
DEFINE_FAKE_VALUE_FUNC(uint32_t, sys_hclk_freq_get);
DEFINE_FAKE_VALUE_FUNC(uint32_t, sys_pclk_freq_get);
DEFINE_FAKE_VOID_FUNC(SystemInit);

#if defined (__ARMCC_VERSION) /* Keil ��Vision 5.29.0.0 */

#endif
