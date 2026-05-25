/**
 *****************************************************************************
 * @brief   gpio driver header file.
 *
 * @file    tcae10_ll_gpio.h
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

#ifndef __TCAE10_LL_GPIO_H__
#define __TCAE10_LL_GPIO_H__

#include "tcae10_ll_def.h"

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
*          GPIO        AFIO_0          AFIO_1              AFIO_2          AFIO_3          AFIO_4          AFIO_5          AFIO_6         AFIO_7
*          GPIO0       SWCLK           GPIO                LED_S           LIN0_TX_T       DTB0            TAO             CAP1           SPI_CLK
*          GPIO1       SWDIO           GPIO                PWM_CH3         SPI_CLK         DTB1            CAP_SHIELD      CAP0           NULL
*          GPIO2       GPIO            UART_TX             PWM_CH0         SPI_RX          DTB2            SPI_DAT         CAP2           ADC1
*          GPIO3       GPIO            UART_TX             PWM_CH1         SPI_CS          DTB3            LIN1_RX         CAP_REF        NULL
*          GPIO4       GPIO            NULL                PWM_CH2         SPI_TX          DTB0            LIN1_TX         CAP3           ADC0
*          GPIO5       GPIO            LIN1_UART_1W        NULL            NULL            NULL            LIN1_TX         CAP4           NULL
*          GPIO6       LED0            PWM_CH0             UART_TX         GPIO            DTB1            TAI             NULL           NULL
*          GPIO7       LED1            PWM_CH1             LIN0_RX         GPIO            DTB2            NULL            NULL           NULL
*          GPIO8       LED2            PWM_CH2             LIN0_TX         GPIO            DTB3            NULL            CAP_SHIELD     NULL
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
    AFIO_MUX_7
} gpio_afio_mux_e;

/**
 * @defgroup GPIO_SOFTWARE_FUNCTIONS GPIO软件输入功能选择
 * @{
 */
/*GPIO0 SOFTWARE FUNCTIONS*/
#define GPIO0_SOFTWARE_INPUT_FUNCTION_SWCLK         0
#define GPIO0_SOFTWARE_INPUT_FUNCTION_GPIO          1
#define GPIO0_SOFTWARE_INPUT_FUNCTION_LED_SWITCH    2
#define GPIO0_SOFTWARE_INPUT_FUNCTION_LIN0_TXD_TEST 3
#define GPIO0_SOFTWARE_INPUT_FUNCTION_DTB0          4
#define GPIO0_SOFTWARE_INPUT_FUNCTION_TAO           5
#define GPIO0_SOFTWARE_INPUT_FUNCTION_CAP1          6
#define GPIO0_SOFTWARE_INPUT_FUNCTION_SPI_CLK       7

/*GPIO1 SOFTWARE FUNCTIONS*/
#define GPIO1_SOFTWARE_INPUT_FUNCTION_SWDIO         0
#define GPIO1_SOFTWARE_INPUT_FUNCTION_GPIO          1
#define GPIO1_SOFTWARE_INPUT_FUNCTION_PWM_CH3       2
#define GPIO1_SOFTWARE_INPUT_FUNCTION_SPI_CLK       3
#define GPIO1_SOFTWARE_INPUT_FUNCTION_DTB1          4
#define GPIO1_SOFTWARE_INPUT_FUNCTION_CAP_SHIELD    5
#define GPIO1_SOFTWARE_INPUT_FUNCTION_CAP0          6

/*GPIO2 SOFTWARE FUNCTIONS*/
#define GPIO2_SOFTWARE_INPUT_FUNCTION_GPIO          0
#define GPIO2_SOFTWARE_INPUT_FUNCTION_UART_TXD      1
#define GPIO2_SOFTWARE_INPUT_FUNCTION_PWM_CH0       2
#define GPIO2_SOFTWARE_INPUT_FUNCTION_SPI_RXD       3
#define GPIO2_SOFTWARE_INPUT_FUNCTION_DTB2          4
#define GPIO2_SOFTWARE_INPUT_FUNCTION_SPI_DATA      5
#define GPIO2_SOFTWARE_INPUT_FUNCTION_CAP2          6
#define GPIO2_SOFTWARE_INPUT_FUNCTION_ADC1          7

/*GPIO3 SOFTWARE FUNCTIONS*/
#define GPIO3_SOFTWARE_INPUT_FUNCTION_GPIO          0
#define GPIO3_SOFTWARE_INPUT_FUNCTION_UART_TXD      1
#define GPIO3_SOFTWARE_INPUT_FUNCTION_PWM_CH1       2
#define GPIO3_SOFTWARE_INPUT_FUNCTION_SPI_CSN       3
#define GPIO3_SOFTWARE_INPUT_FUNCTION_DTB3          4
#define GPIO3_SOFTWARE_INPUT_FUNCTION_LIN1_RXD      5
#define GPIO3_SOFTWARE_INPUT_FUNCTION_CAP_REF       6

/*GPIO4 SOFTWARE FUNCTIONS*/
#define GPIO4_SOFTWARE_INPUT_FUNCTION_GPIO          0
#define GPIO4_SOFTWARE_INPUT_FUNCTION_PWM_CH2       2
#define GPIO4_SOFTWARE_INPUT_FUNCTION_SPI_TXD       3
#define GPIO4_SOFTWARE_INPUT_FUNCTION_DTB0          4
#define GPIO4_SOFTWARE_INPUT_FUNCTION_LIN1_TXD      5
#define GPIO4_SOFTWARE_INPUT_FUNCTION_CAP3          6
#define GPIO4_SOFTWARE_INPUT_FUNCTION_ADC0          7

/*GPIO5 SOFTWARE FUNCTIONS*/
#define GPIO5_SOFTWARE_INPUT_FUNCTION_GPIO          0
#define GPIO5_SOFTWARE_INPUT_FUNCTION_LIN1_UART_1W  1
#define GPIO5_SOFTWARE_INPUT_FUNCTION_LIN1_TXD      5
#define GPIO5_SOFTWARE_INPUT_FUNCTION_CAP4          6

/*GPIO_LED0 SOFTWARE FUNCTIONS*/
#define LED_IO0_SOFTWARE_INPUT_FUNCTION_LED0        0
#define LED_IO0_SOFTWARE_INPUT_FUNCTION_PWM_CH0     1
#define LED_IO0_SOFTWARE_INPUT_FUNCTION_UART_TXD    2
#define LED_IO0_SOFTWARE_INPUT_FUNCTION_GPIO6       3
#define LED_IO0_SOFTWARE_INPUT_FUNCTION_DTB1        4
#define LED_IO0_SOFTWARE_INPUT_FUNCTION_TAI         5

/*GPIO_LED1 SOFTWARE FUNCTIONS*/
#define LED_IO1_SOFTWARE_INPUT_FUNCTION_LED1        0
#define LED_IO1_SOFTWARE_INPUT_FUNCTION_PWM_CH1     1
#define LED_IO1_SOFTWARE_INPUT_FUNCTION_LIN0_RXD    2
#define LED_IO1_SOFTWARE_INPUT_FUNCTION_GPIO7       3
#define LED_IO1_SOFTWARE_INPUT_FUNCTION_DTB2        4

/*GPIO_LED2 SOFTWARE FUNCTIONS*/
#define LED_IO2_SOFTWARE_INPUT_FUNCTION_LED2        0
#define LED_IO2_SOFTWARE_INPUT_FUNCTION_PWM_CH2     1
#define LED_IO2_SOFTWARE_INPUT_FUNCTION_LIN0_TXD    2
#define LED_IO2_SOFTWARE_INPUT_FUNCTION_GPIO8       3
#define LED_IO2_SOFTWARE_INPUT_FUNCTION_DTB3        4
#define LED_IO2_SOFTWARE_INPUT_FUNCTION_CAP_SHIELD  6
/** @} */

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

/**
 * @brief  去初始化GPIO模块
 */
void ll_gpio_deinit(void);
/**
 * @brief  初始化GPIO引脚
 * @param config - GPIO配置结构体指针（含引脚、模式、上下拉等）
 * @param callback - 中断回调函数指针
 */
void ll_gpio_init(gpio_config_t *config, ISR_FUNC_CALLBACK callback);
/**
 * @brief  读取GPIO引脚电平
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 * @retval true: 高电平，false: 低电平
 */
bool ll_gpio_read(gpio_pin_e gpio_pin);
/**
 * @brief  设置GPIO引脚输出电平
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 * @param state - true: 高电平，false: 低电平
 */
void ll_gpio_output(gpio_pin_e gpio_pin, bool state);
/**
 * @brief  翻转GPIO引脚输出电平
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 */
void ll_gpio_toggle(gpio_pin_e gpio_pin);
/**
 * @brief  使能/禁能GPIO中断
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 * @param enable - true: 使能，false: 禁能
 */
void ll_gpio_isr_enable(gpio_pin_e gpio_pin, bool enable);
/**
 * @brief  获取GPIO中断标志
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 * @retval true: 中断已触发，false: 中断未触发
 */
bool ll_gpio_interrupt_flag_get(gpio_pin_e gpio_pin);
/**
 * @brief  清除GPIO中断标志
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 */
void ll_gpio_interrupt_clear(gpio_pin_e gpio_pin);
/**
 * @brief  配置GPIO复用功能
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 * @param afio_mux - 复用功能选择 @ref gpio_afio_mux_e
 */
void ll_gpio_afio_config(gpio_pin_e gpio_pin, gpio_afio_mux_e afio_mux);
/**
 * @brief  获取GPIO当前复用功能
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 * @retval 当前复用功能选择 @ref gpio_afio_mux_e
 */
gpio_afio_mux_e ll_gpio_afio_get(gpio_pin_e gpio_pin);
/**
 * @brief  配置GPIO低功耗模式下的模拟引脚状态
 * @param gpio_pin - GPIO引脚号 @ref gpio_pin_e
 * @param keep_high - true: 保持高电平，false: 保持低电平
 */
void ll_gpio_ano_lpm_config(gpio_pin_e gpio_pin, bool keep_high);
/**
 * @brief  使能/禁能GPIO复位功能
 * @param enable - true: 使能复位，false: 禁能复位
 */
void ll_gpio_reset_enable(bool enable);

#if defined(__cplusplus)
}
#endif
#endif /* __TCAE10_LL_GPIO_H__ */
