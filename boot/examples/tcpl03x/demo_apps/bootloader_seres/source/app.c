/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   bootloader example source file.
 *
 * @file    app.c
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

#include "system_tcpl03x.h"
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "tcpl03x.h"
#include "pal_systick.h"
#include "pal_func_def.h"
#include "dfu_uds_manager.h"

/* PRQA S 3219 1 #3254 - Unused static function, reserved for future extension */
static void DoorGpioInit(void)
{
    ll_gpio_output(GPIO_PIN_1, false);

    gpio_config_t cfg;
    cfg.gpio_pin = GPIO_PIN_1;
    cfg.mode = GPIO_MODE_OUT_PP;
    cfg.afio = AFIO_MUX_1;
    ll_gpio_init(&cfg, NULL);
    ll_gpio_output(GPIO_PIN_1, false);
}

/* PRQA S 3469 8 #3258 - Function-like macro used for performance and compiler optimization requirements */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
int32_t TcMain(void)
{
    interrupt_disable();
    // DoorGpioInit()_;
    dfu_manager_init();
    interrupt_enable();

    while (1)
    {
        main_loops();
    }
}
