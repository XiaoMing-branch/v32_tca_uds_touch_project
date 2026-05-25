#ifndef __APP_H__
#define __APP_H__

#include "test_config.h"
#include "fff.h"
/**************************************************************/
/*支持电容触摸功能*/
#define TOUCH_FUNC_EN           1

/*支持LIN功能*/
#define LIN_FUNC_EN             1

/*支持低功耗功能*/
#define LOW_POWER_EN            1

/*打印开关*/
#define DEBUG_PRINT_EN          0

/*看门狗开关*/
#define WATCH_DOG_EN            1           //用lin boot功能时，需要将看门狗打开

/*配置字开关*/
#define CONFIG_BYTE_WRITE_EN    1

#define LIN_CUSTOM_MASTER_WKUP  1

#define CFG_SUPPORT_LIN_SNPD    0
#define CFG_SUPPORT_DFU_MULT    0

#define UNLOCK_PIN              (GPIO_PIN_8)

#define FLASH_APP_ADDR          (0x00004100u)

#define OPEN_DOOR_MIN_TIMEMS        (320)
#define OPEN_DOOR_MAX_TIMEMS        (2000)

#endif
