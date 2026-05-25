#ifndef __FFF_TCPL03X_LL_GPIO_H__
#define __FFF_TCPL03X_LL_GPIO_H__


#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_def.h"
#else
    #include "tcpl03x_ll_def.h"
#endif

#include "fff.h"


#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief  gpio pin enumeration
 */
typedef enum
{
    GPIO_PIN_0 = 0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,   //LED0
    GPIO_PIN_7,   //LED1
    GPIO_PIN_8,   //LED2
} gpio_pin_e;

/**
 * @brief  gpio mode enumeration
 */
typedef enum
{
    GPIO_MODE_OUT_PP = 0,
    GPIO_MODE_OUT_OD,
    GPIO_MODE_IN_PP,
    GPIO_MODE_IN_OD,
} gpio_mode_e;

/**
 * @brief  gpio pull mode enumeration
 */
typedef enum
{
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
} gpio_pull_mode_e;

/**
 * @brief  gpio pull down type enumeration
 */
typedef enum
{
    GPIO_PULLDOWN_SWANDHW = 0,  /*!< GPIO Pull down is controlled by Outx_pd and hardware together*/
    GPIO_PULLDOWN_SW_ONLY,          /*!< GPIO Pull down is controlled by Outx_pd only*/
} gpio_pull_down_type;

/**
 * @brief  gpio trigger type enumeration
 */
typedef enum
{
    GPIO_TRIGGER_NULL = 0,
    GPIO_TRIGGER_LOW_LEVEL,           /*interrupt is active as long as GPIO is in low level*/
    GPIO_TRIGGER_FALLING_EDGE,        /*interrupt is active only on the falling edge*/
    GPIO_TRIGGER_HIGH_LEVEL,          /*interrupt is active as long as GPIO is in high level*/
    GPIO_TRIGGER_RISING_EDGE,         /*interrupt is active only on the rising edge*/
    GPIO_TRIGGER_RISING_FALLING_EDGE  /*both rising and falling edge will trigger interrupt*/
} gpio_trigger_flag_e;

/**
 * @brief  gpio trigger type enumeration GPIO0 and GPIO1为高速
*   GPIO    AFIO_0  AFIO_1      AFIO_2      AFIO_3      AFIO_4  AFIO_5      AFIO_6  AFIO_7
*   GPIO0   SWCLK   GPIO0       LED_S       LIN0_TX_T   NULL    NULL        NULL    NULL
*   GPIO1   SWDIO   GPIO1       PWM_CH3     NULL        NULL    NULL        NULL    NULL
*   GPIO2   GPIO2   DEBUG_TXD   PWM_CH0     NULL        NULL    NULL        NULL    NULL
*   GPIO3   GPIO3   DEBUG_TXD   PWM_CH1     NULL        NULL    LIN1_RXD    NULL    NULL
*   GPIO4   GPIO4   NULL        PWM_CH2     NULL        NULL    LIN1_TXD    NULL    NULL
*   GPIO5   GPIO5   LIN1_UART_1 NULL        NULL        NULL    LIN1_TXD    NULL    NULL
*   GPIO6   LED0    PWM_CH0     DEBUG_TXD   GPIO6       NULL    NULL        NULL    NULL
*   GPIO7   LED1    PWM_CH1     LIN0_RXD    GPIO7       NULL    NULL        NULL    NULL
*   GPIO8   LED2    PWM_CH2     LIN0_TXD    GPIO8       NULL    NULL        NULL    NULL
*/
typedef enum
{
    AFIO_MUX_0 = 0,
    AFIO_MUX_1,
    AFIO_MUX_2,
    AFIO_MUX_3,
    AFIO_MUX_4,
    AFIO_MUX_5,
    AFIO_MUX_6,
} gpio_afio_mux_e;

/**
  * @defgroup GPIO_Configuration struct
  */
typedef struct
{
    gpio_pin_e gpio_pin;                                    /*!< Specifies GPIO PIN it can be any value of @ref gpio_pin_e */
    gpio_mode_e mode;                           /*!< Specifies GPIO mode  it can be any value of @ref gpio_mode_e */
    gpio_pull_mode_e pull_mode;                     /*!< Specifies GPIO Pull mode it can be any value of @ref gpio_pull_mode_e */
    gpio_pull_down_type pull_down_type;   /*!< Specifies the pull-down type for the GPIO it can be any value of @ref gpio_pull_down_type */
    gpio_afio_mux_e afio;                 /*!< Specifies the alternate function for the gpio it can be any value of @ref gpio_afio_mux_e */
    gpio_trigger_flag_e trigger_flag;
} gpio_config_t;


DECLARE_FAKE_VOID_FUNC(ll_gpio_deinit);
DECLARE_FAKE_VOID_FUNC(ll_gpio_init,gpio_config_t *,ISR_FUNC_CALLBACK);
DECLARE_FAKE_VALUE_FUNC(bool,ll_gpio_read,gpio_pin_e);
DECLARE_FAKE_VOID_FUNC(ll_gpio_output,gpio_pin_e,bool);
DECLARE_FAKE_VOID_FUNC(ll_gpio_toggle,gpio_pin_e);
DECLARE_FAKE_VOID_FUNC(ll_gpio_isr_enable,gpio_pin_e,bool);
DECLARE_FAKE_VALUE_FUNC(bool,ll_gpio_interrupt_flag_get,gpio_pin_e);
DECLARE_FAKE_VOID_FUNC(ll_gpio_interrupt_clear,gpio_pin_e);
DECLARE_FAKE_VOID_FUNC(ll_gpio_afio_config,gpio_pin_e,gpio_afio_mux_e);
DECLARE_FAKE_VOID_FUNC(ll_gpio_ano_lpm_config,gpio_pin_e,bool);
DECLARE_FAKE_VOID_FUNC(ll_gpio_reset_enable,bool);


#if defined(__cplusplus)
}
#endif
#endif /* __TCPL03X_LL_GPIO_H__ */
