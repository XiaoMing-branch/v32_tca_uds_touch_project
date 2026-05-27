/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   bootloader 示例应用源文件。
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
/**
 * @brief   门 GPIO 初始化。
 *          配置 GPIO_PIN_1 为推挽输出模式，初始输出低电平。
 *
 * @retval  无
 */
static void DoorGpioInit(void)
{
    ll_gpio_output(GPIO_PIN_1, false);          /* 初始输出低电平 */

    gpio_config_t cfg;                          /* 声明 GPIO 配置结构体 */
    cfg.gpio_pin = GPIO_PIN_1;                  /* 配置引脚号为 GPIO_PIN_1 */
    cfg.mode = GPIO_MODE_OUT_PP;                /* 配置为推挽输出模式 */
    cfg.afio = AFIO_MUX_1;                      /* 配置复用功能选择 */
    ll_gpio_init(&cfg, NULL);                   /* 执行 GPIO 初始化 */
    ll_gpio_output(GPIO_PIN_1, false);          /* 再次确保输出为低电平 */
}

/* PRQA S 3469 8 #3258 - Function-like macro used for performance and compiler optimization requirements */
/* PRQA S 3408 2 #3218 - External linkage function defined without prior declaration, intentional design */
/* PRQA S 1503 1 #3214 - Unused function defined for future extension and module completeness */
/**
 * @brief   bootloader 主入口函数。
 *          关闭全局中断，初始化 DFU 管理器，重新使能中断，然后进入主循环。
 *
 * @retval  0  正常返回（实际不会返回，在 while(1) 中循环运行）
 */
int32_t TcMain(void)
{
    interrupt_disable();                        /* 关闭全局中断，防止初始化过程被中断打扰 */

    // DoorGpioInit()_;                         /* 门 GPIO 初始化（预留功能，当前未启用） */
    dfu_manager_init();                         /* 初始化 DFU 管理器：看门狗启动、密钥生成、系统配置加载、LIN 总线初始化 */
    interrupt_enable();                         /* 重新使能全局中断，进入正常运行模式 */

    while (1)                                   /* 超级循环：bootloader 主状态机持续运行，永不退出 */
    {
        main_loops();                           /* 执行主循环处理：喂狗 → LIN 诊断服务分发 → 异常处理 → 随机种子更新 */
                                                /* 状态机：IDLE 状态下定期检测 DFU 信息 → 决策跳转 USER_APP 或进入 UPGRADE 模式 */
    }
}
