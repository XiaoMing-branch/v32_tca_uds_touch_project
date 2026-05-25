#ifndef __FFF_TC_LOG_CONF_H__
#define __FFF_TC_LOG_CONF_H__

#include "fff.h"
#include <stdint.h>
#include "fff_app.h"

//*******************************************************************************
//日志开关

#if DEBUG_PRINT_EN

    #define LOG_LOCAL_LEVEL                 TC_LOG_VERBOSE         /*!< 日志等级 */

    #define LOG_SYMBOL_SWITCH               1                      /*!< 打印符号表开关 */
    #define LOG_SYSTEM_STATUS               0                      /*!< 打印系统状态开关 */
    #define LOG_RAWDATA                     0                      /*!< 打印rawdata开关 */

#else
    #define LOG_LOCAL_LEVEL                 TC_LOG_NONE

    #define LOG_SYMBOL_SWITCH               0                      /*!< 打印符号表开关 */
    #define LOG_SYSTEM_STATUS               0                      /*!< 打印系统状态开关 */
    #define LOG_RAWDATA                     0                      /*!< 打印rawdata开关 */
#endif

//*******************************************************************************
//其他配置

//日志接口类型
#define LOG_INTERFACE_UART              0                       /*!< UART日志接口*/
#define LOG_INTERFACE_LIN               1                       /*!< LIN日志接口 */
#define LOG_INTERFACE_LIN_UART          2                       /*!< LIN UART日志接口 */
#define LOG_INTERFACE_TYPE              LOG_INTERFACE_LIN  /*!< 选择当前打印的日志输出接口，当选择LOG_INTERFACE_LIN打印时，需要将lin_hw_cfg.h中的LIN_DEBUG_EN宏打开 */

#if !DEBUG_PRINT_EN
#undef LOG_INTERFACE_TYPE
#define LOG_INTERFACE_TYPE              LOG_INTERFACE_UART
#endif

#define LOG_FAST_MODE                   1                       /*!< 当用AT3000作为上位机时，设置为1，否则为0 */

#define LOG_RAWDATA_NORMAL_INTERVAL     0       /*!< 唤醒模式，打印RAWDATA时，每间隔几帧数据采集一帧，主要用于像lin等低速接口上，防止打印数据过多发不过来*/
#define LOG_RAWDATA_SLEEP_INTERVAL      0       /*!< 低功耗模式，打印RAWDATA时，每间隔几帧数据采集一帧，主要用于像lin等低速接口上，防止打印数据过多发不过来*/

#if !defined(LOG_INTERFACE_TYPE)
    #error "LOG_INTERFACE_TYPE not defined"
#else
    #if (LOG_INTERFACE_TYPE == LOG_INTERFACE_UART)

        #define LOG_TO_RAMBUFFER                0               /*!< 是否输出数据到缓存*/
        #define LOG_CUSTOM_OUTPUT               0               /*!< 是否自定义log输出，需要用户自己实现TcLogWrite功能*/
        #define LOG_USE_STD_WRITE               1               /*!< 使用fwrite发送数据*/

    #elif (LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN)

        #define LOG_TO_RAMBUFFER                1               /*!< 是否输出数据到缓存*/
        #define LOG_CUSTOM_OUTPUT               1               /*!< 是否自定义log输出，需要用户自己实现TcLogWrite功能*/
        #define LOG_USE_STD_WRITE               0               /*!< 使用fwrite发送数据*/

        #undef LOG_FAST_MODE                    /*!< 强制配置fast模式*/
        #define LOG_FAST_MODE                   1
    #elif (LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN_UART)

        #define LOG_TO_RAMBUFFER                0               /*!< 是否输出数据到缓存*/
        #define LOG_CUSTOM_OUTPUT               0               /*!< 是否自定义log输出，需要用户自己实现TcLogWrite功能*/
        #define LOG_USE_STD_WRITE               1               /*!< 使用fwrite发送数据*/

    #else
        #error "LOG_INTERFACE_TYPE not exist"
    #endif
#endif

#if LOG_TO_RAMBUFFER
    #define LOG_RAMBUFFER_SIZE      100         /*!< 缓冲区尺寸*/
#endif

#if LOG_CUSTOM_OUTPUT
    #define LOG_PRINT_BUFFER_SIZE   64          /*!< 打印缓冲区尺寸*/
#endif

#if LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN
    #define LIN_DEBUG_INFO_SID   0xA0
    extern uint8_t g_bUDSReadLogInfo;
#endif

#endif
