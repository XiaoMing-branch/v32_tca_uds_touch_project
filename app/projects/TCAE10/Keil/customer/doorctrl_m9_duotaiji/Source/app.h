#ifndef APP_H__
#define APP_H__

#include "test_config.h"
/**************************************************************/
/*Supports capacitive touch functionality*/
#define TOUCH_FUNC_EN           1

/*Supports LIN functionality*/
#define LIN_FUNC_EN             1

/*Supports low power functionality*/
#define LOW_POWER_EN            1

/*Print switch*/
#define DEBUG_PRINT_EN          0

/*Watchdog switch*/
#define WATCH_DOG_EN            1           //When using the LIN boot function, the watchdog needs to be enabled.

/*Configure character switch*/
#define CONFIG_BYTE_WRITE_EN    1
/* PRQA S 1534 4 #3261 - Unused macro defined for future extension and configuration compatibility */
#define LIN_CUSTOM_MASTER_WKUP  1

#define CFG_SUPPORT_LIN_SNPD    0
#define CFG_SUPPORT_DFU_MULT    0

#define UNLOCK_PIN              (GPIO_PIN_8)

#define FLASH_APP_ADDR          (0x00004100u)

#define OPEN_DOOR_MIN_TIMEMS        (320)
#define OPEN_DOOR_MAX_TIMEMS        (2000)

#endif
