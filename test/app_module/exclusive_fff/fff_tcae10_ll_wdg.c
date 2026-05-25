/**
 *****************************************************************************
 * @brief   wdg driver source file.
 *
 * @file    tcae10_ll_wdg.c
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

#include "fff_tcae10_ll_wdg.h"
//#include "fff_tcae10_ll_cortex.h"

/** @defgroup IWDG_LOCK_Definitions
 * @{
 */
#if 1
#define WDG_UNLCOK() (IWDG->LOCK = 0XAAAA5555)
#define WDG_LCOK() (IWDG->LOCK = 0X12345678)
#else
#define WDG_UNLCOK()
#define WDG_LCOK()
#endif

static ISR_FUNC_CALLBACK wdg_callback = NULL;

/********************************************************
** \brief   ll_wdg_init
**
** \param   wdg_config_t*   config
** \param   ISR_FUNC_CALLBACK   callback
**
** \retval  None
*********************************************************/
DEFINE_FAKE_VOID_FUNC(ll_wdg_init, wdg_config_t *, ISR_FUNC_CALLBACK);

/********************************************************
** \brief   ll_wdg_init
**
** \param   None
**
** \retval  None
*********************************************************/
DEFINE_FAKE_VOID_FUNC(ll_wdg_deinit);

/********************************************************
** \brief   ll_wdg_isr_enable
**
** \param   ll_sci_bus_e    bus
** \param   bool            enable
**
** \retval  None
*********************************************************/
DEFINE_FAKE_VOID_FUNC(ll_wdg_isr_enable, bool);

/********************************************************
** \brief   ll_wdg_enable
**
** \param   bool    bool
**
** \retval  None
*********************************************************/
DEFINE_FAKE_VOID_FUNC(ll_wdg_enable, bool);

/********************************************************
** \brief   ll_wdg_reload
**
** \retval  None
*********************************************************/
DEFINE_FAKE_VOID_FUNC(ll_wdg_reload);

/********************************************************
** \brief   ll_wdg_interrupt_clear
**
** \retval  None
*********************************************************/
DEFINE_FAKE_VOID_FUNC(ll_wdg_interrupt_clear);

/********************************************************
** \brief   IWDG_IRQHandler
**
** \retval  None
*********************************************************/
// void IWDG_IRQHandler(void)
// {
//     ll_wdg_interrupt_clear();
//     ll_wdg_reload();

//     if (NULL != wdg_callback)
//     {
//         wdg_callback(0);
//     }
// }
