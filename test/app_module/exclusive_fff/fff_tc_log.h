#ifndef __FFF_TC_LOG_H__
#define __FFF_TC_LOG_H__

/**
 * @file
 * @brief        日志工具
 * @details      打印日志信息
 * @author      AE/FAE team
 * @date       2024-3-11
 * @version  v1.0
 * @par Copyright(c):    tinychip corporation
 * @par History:
 *   version: author, date, desc\n
 */

#include "fff.h"
#include <stdint.h>
#include "fff_tc_log_conf.h"
//#include "tcae10_ll_uart.h"

#define LOG_COLOR_E     /*!< 打印error日志颜色 */
#define LOG_COLOR_W     /*!< 打印warn日志颜色 */
#define LOG_COLOR_I     /*!< 打印info日志颜色 */
#define LOG_COLOR_D     /*!< 打印debug日志颜色 */
#define LOG_COLOR_V     /*!< 打印verbose日志颜色 */
#define LOG_RESET_COLOR /*!< 打印reset日志颜色 */

/**
 * @brief       格式化日志信息
 * @param[in]   letter 日志类型标识
 * @param[in]   desp 日志描述信息
 * @param[in]   format 日志格式符
 * @retval      格式化后字符串
 */
#define LOG_FORMAT(letter, desp, format) LOG_COLOR_##letter #desp " %s: " format LOG_RESET_COLOR "\n"

/**
 * @brief 日志等级
 *
 */
#define TC_LOG_NONE 0    /*!< No log output */
#define TC_LOG_ERROR 1   /*!< Critical errors, software module can not recover on its own */
#define TC_LOG_WARN 2    /*!< Error conditions from which recovery measures have been taken */
#define TC_LOG_INFO 3    /*!< Information messages which describe normal flow of events */
#define TC_LOG_DEBUG 4   /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
#define TC_LOG_VERBOSE 5 /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */

#if LOG_CUSTOM_OUTPUT // 用户自定义输出
/**
 * @brief       打印日志
 * @param[in]   level 日志等级
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOG_LEVEL(level, tag, format, ...)                             \
    do                                                                    \
    {                                                                     \
        if (level == TC_LOG_ERROR)                                        \
        {                                                                 \
            TcLogWrite(LOG_FORMAT(E, _LOGE, format), tag, ##__VA_ARGS__); \
        }                                                                 \
        else if (level == TC_LOG_WARN)                                    \
        {                                                                 \
            TcLogWrite(LOG_FORMAT(W, _LOGW, format), tag, ##__VA_ARGS__); \
        }                                                                 \
        else if (level == TC_LOG_DEBUG)                                   \
        {                                                                 \
            TcLogWrite(LOG_FORMAT(D, _LOGD, format), tag, ##__VA_ARGS__); \
        }                                                                 \
        else if (level == TC_LOG_VERBOSE)                                 \
        {                                                                 \
            TcLogWrite(LOG_FORMAT(V, _LOGV, format), tag, ##__VA_ARGS__); \
        }                                                                 \
        else                                                              \
        {                                                                 \
            TcLogWrite(LOG_FORMAT(I, _LOGI, format), tag, ##__VA_ARGS__); \
        }                                                                 \
    } while (0)
#else
/**
 * @brief       打印日志
 * @param[in]   level 日志等级
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOG_LEVEL(level, tag, format, ...)                         \
    do                                                                \
    {                                                                 \
        if (level == TC_LOG_ERROR)                                    \
        {                                                             \
            printf(LOG_FORMAT(E, _LOGE, format), tag, ##__VA_ARGS__); \
        }                                                             \
        else if (level == TC_LOG_WARN)                                \
        {                                                             \
            printf(LOG_FORMAT(W, _LOGW, format), tag, ##__VA_ARGS__); \
        }                                                             \
        else if (level == TC_LOG_DEBUG)                               \
        {                                                             \
            printf(LOG_FORMAT(D, _LOGD, format), tag, ##__VA_ARGS__); \
        }                                                             \
        else if (level == TC_LOG_VERBOSE)                             \
        {                                                             \
            printf(LOG_FORMAT(V, _LOGV, format), tag, ##__VA_ARGS__); \
        }                                                             \
        else                                                          \
        {                                                             \
            printf(LOG_FORMAT(I, _LOGI, format), tag, ##__VA_ARGS__); \
        }                                                             \
    } while (0)
#endif

/**
 * @brief       输出特定等级的日志信息
 * @param[in]   level 日志等级
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOG_LEVEL_LOCAL(level, tag, format, ...)          \
    do                                                       \
    {                                                        \
        if (LOG_LOCAL_LEVEL >= level)                        \
            TC_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while (0)

#if defined(LOG_LOCAL_LEVEL)
/**
 * @brief       输出error日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGE(tag, format, ...) TC_LOG_LEVEL_LOCAL(TC_LOG_ERROR, tag, format, ##__VA_ARGS__)
/**
 * @brief       输出warn日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGW(tag, format, ...) TC_LOG_LEVEL_LOCAL(TC_LOG_WARN, tag, format, ##__VA_ARGS__)
/**
 * @brief       输出info日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGI(tag, format, ...) TC_LOG_LEVEL_LOCAL(TC_LOG_INFO, tag, format, ##__VA_ARGS__)
/**
 * @brief       输出debug日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGD(tag, format, ...) TC_LOG_LEVEL_LOCAL(TC_LOG_DEBUG, tag, format, ##__VA_ARGS__)
/**
 * @brief       输出verbose日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGV(tag, format, ...) TC_LOG_LEVEL_LOCAL(TC_LOG_VERBOSE, tag, format, ##__VA_ARGS__)
#else
/**
 * @brief       输出error日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGE(tag, format, ...) \
    {                             \
        (void)(tag);              \
        (void)(format);           \
    }
/**
 * @brief       输出warn日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGW(tag, format, ...) \
    {                             \
        (void)(tag);              \
        (void)(format);           \
    }
/**
 * @brief       输出info日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGI(tag, format, ...) \
    {                             \
        (void)(tag);              \
        (void)(format);           \
    }
/**
 * @brief       输出debug日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGD(tag, format, ...) \
    {                             \
        (void)(tag);              \
        (void)(format);           \
    }
/**
 * @brief       输出verbose日志
 * @param[in]   tag 日志标签
 * @param[in]   format 日志格式符
 * @retval
 */
#define TC_LOGV(tag, format, ...) \
    {                             \
        (void)(tag);              \
        (void)(format);           \
    }
#endif

#if LOG_LOCAL_LEVEL == TC_LOG_NONE
#define TC_LOG(...)
#define printf(...)
#else
#define TC_LOG printf
#endif

#if LOG_CUSTOM_OUTPUT
/**
 * @brief       底层自定义发送日志数据
 * @param[in]   format 日志格式符
 * @retval
 */
DECLARE_FAKE_VOID_FUNC_VARARG(TcLogWrite, const char *, ...);
// void TcLogWrite(const char *format, ...);
#endif

#define LOG_SEND_REPEAT 1 /*!< 打印寄存器时，重复发送次数，防止漏发 */

/**
 * @brief 日志命令
 *
 */
enum
{
    TC_LOG_CMD_REG,           /*!< 寄存器命令 */
    TC_LOG_CMD_SYMBOL,        /*!< 符号表命令 */
    TC_LOG_CMD_SYMBOL_NOTE,   /*!< 符号表备注信息 */
    TC_LOG_CMD_RAM,           /*!< 发送ram数据命令 */
    TC_LOG_CMD_SYMBOL_U8,     /*!< 带名称的符号表，U8 */
    TC_LOG_CMD_SYMBOL_I8,     /*!< 带名称的符号表，I8 */
    TC_LOG_CMD_SYMBOL_U16,    /*!< 带名称的符号表，U16 */
    TC_LOG_CMD_SYMBOL_I16,    /*!< 带名称的符号表，I16 */
    TC_LOG_CMD_SYMBOL_U32,    /*!< 带名称的符号表，U32 */
    TC_LOG_CMD_SYMBOL_I32,    /*!< 带名称的符号表，I32 */
    TC_LOG_CMD_SYSTEM_STATUS, /*!< 系统状态 */
    TC_LOG_CMD_RAWDATA_U8,    /*!< 原始值，U8 */
    TC_LOG_CMD_RAWDATA_I8,    /*!< 原始值，I8 */
    TC_LOG_CMD_RAWDATA_U16,   /*!< 原始值，U16 */
    TC_LOG_CMD_RAWDATA_I16,   /*!< 原始值，I16 */
    TC_LOG_CMD_RAWDATA_U32,   /*!< 原始值，U32 */
    TC_LOG_CMD_RAWDATA_I32,   /*!< 原始值，I32 */
};

#if !LOG_FAST_MODE
/**
 * @brief       打印符号表
 * @param[in]   p map文件中定义的data符号的变量的地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
int TC_LOG_SYMBOL(const void *p, int len);
#endif

/**
 * @brief       打印命名U8符号
 * @param[in]   name 符号名称，const char *类型
 * @param[in]   p 符号地址
 * @param[in]   len 符号长度，单位byte
 * @retval
 */
#define TC_LOG_SYMBOL_U8(name, p, len) TC_LOG_SYMBOL_NAMED(TC_LOG_CMD_SYMBOL_U8, (name), (p), (len))
/**
 * @brief       打印命名I8符号
 * @param[in]   name 符号名称，const char *类型
 * @param[in]   p 符号地址
 * @param[in]   len 符号长度，单位byte
 * @retval
 */
#define TC_LOG_SYMBOL_I8(name, p, len) TC_LOG_SYMBOL_NAMED(TC_LOG_CMD_SYMBOL_I8, (name), (p), (len))
/**
 * @brief       打印命名U16符号
 * @param[in]   name 符号名称，const char *类型
 * @param[in]   p 符号地址
 * @param[in]   len 符号长度，单位byte
 * @retval
 */
#define TC_LOG_SYMBOL_U16(name, p, len) TC_LOG_SYMBOL_NAMED(TC_LOG_CMD_SYMBOL_U16, (name), (p), (len))
/**
 * @brief       打印命名I16符号
 * @param[in]   name 符号名称，const char *类型
 * @param[in]   p 符号地址
 * @param[in]   len 符号长度，单位byte
 * @retval
 */
#define TC_LOG_SYMBOL_I16(name, p, len) TC_LOG_SYMBOL_NAMED(TC_LOG_CMD_SYMBOL_I16, (name), (p), (len))
/**
 * @brief       打印命名U32符号
 * @param[in]   name 符号名称，const char *类型
 * @param[in]   p 符号地址
 * @param[in]   len 符号长度，单位byte
 * @retval
 */
#define TC_LOG_SYMBOL_U32(name, p, len) TC_LOG_SYMBOL_NAMED(TC_LOG_CMD_SYMBOL_U32, (name), (p), (len))
/**
 * @brief       打印命名I32符号
 * @param[in]   name 符号名称，const char *类型
 * @param[in]   p 符号地址
 * @param[in]   len 符号长度，单位byte
 * @retval
 */
#define TC_LOG_SYMBOL_I32(name, p, len) TC_LOG_SYMBOL_NAMED(TC_LOG_CMD_SYMBOL_I32, (name), (p), (len))

/**
 * @brief       打印U8原始数据
 * @param[in]   type 数据类型
 * @param[in]   status 数据状态
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
#define TC_LOG_RAWDATA_U8(type, status, p, len) TC_LOG_RAWDATA(TC_LOG_CMD_RAWDATA_U8, (type), (status), (p), (len))
/**
 * @brief       打印I8原始数据
 * @param[in]   type 数据类型
 * @param[in]   status 数据状态
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
#define TC_LOG_RAWDATA_I8(type, status, p, len) TC_LOG_RAWDATA(TC_LOG_CMD_RAWDATA_I8, (type), (status), (p), (len))
/**
 * @brief       打印U16原始数据
 * @param[in]   type 数据类型
 * @param[in]   status 数据状态
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
#define TC_LOG_RAWDATA_U16(type, status, p, len) TC_LOG_RAWDATA(TC_LOG_CMD_RAWDATA_U16, (type), (status), (p), (len))
/**
 * @brief       打印I16原始数据
 * @param[in]   type 数据类型
 * @param[in]   status 数据状态
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
#define TC_LOG_RAWDATA_I16(type, status, p, len) TC_LOG_RAWDATA(TC_LOG_CMD_RAWDATA_I16, (type), (status), (p), (len))
/**
 * @brief       打印U32原始数据
 * @param[in]   type 数据类型
 * @param[in]   status 数据状态
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
#define TC_LOG_RAWDATA_U32(type, status, p, len) TC_LOG_RAWDATA(TC_LOG_CMD_RAWDATA_U32, (type), (status), (p), (len))
/**
 * @brief       打印I32原始数据
 * @param[in]   type 数据类型
 * @param[in]   status 数据状态
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
#define TC_LOG_RAWDATA_I32(type, status, p, len) TC_LOG_RAWDATA(TC_LOG_CMD_RAWDATA_I32, (type), (status), (p), (len))

/**
 * @brief 系统状态
 *
 */
typedef enum
{
    TC_LOG_SYSTEM_NORMAL = 0, /*!< 常规状态 */
    TC_LOG_SYSTEM_HALT        /*!< 低功耗状态 */
} T_TcLogSystemStatus;

/**
 * @brief RAWDATA类型
 *
 */
typedef enum
{
    TC_LOG_RAWDATA_CUSTOM /*!< 自定义 */
} T_TcLogRawdataType;

/**
 * @brief 打印引脚类型
 *
 */
typedef enum
{
    PRINT_GPIO2 = 0,
    PRINT_GPIO3,
    PRINT_GPIO5,
    PRINT_GPIO6
} T_TcLogPrintPin;

/**
 * @brief       打印初始化
 * @param[in]   baud 波特率
 * @retval
 */
DECLARE_FAKE_VOID_FUNC(PrintInit, uint32_t);
// void PrintInit(uint32_t baud); // 打印初始化

/**
 * @brief       设置打印引脚
 * @param[in]   pin 引脚
 * @retval
 */
DECLARE_FAKE_VOID_FUNC(PrintSetPin, T_TcLogPrintPin);
// void PrintSetPin(T_TcLogPrintPin pin);

/**
 * @brief       进入低功耗
 * @param[in]   NULL
 * @retval
 */
DECLARE_FAKE_VOID_FUNC(PrintEnterSleep);
// void PrintEnterSleep(void);

/**
 * @brief       退出低功耗
 * @param[in]   NULL
 * @retval
 */
DECLARE_FAKE_VOID_FUNC(PrintWakeup);
// void PrintWakeup(void);

/**
 * @brief       打印初始化
 * @param[in]   baud 波特率
 * @retval
 */
#define TC_LOG_Init(baud) PrintInit(baud)

/**
 * @brief       设置打印引脚
 * @param[in]   pin 引脚
 * @retval
 */
#define TC_LOG_SetPin(pin) PrintSetPin(pin)

/**
 * @brief       带名称的符号表打印，内部使用
 * @param[in]   cmd 打印符号命令
 * @param[in]   name 名称
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
DECLARE_FAKE_VALUE_FUNC(int, TC_LOG_SYMBOL_NAMED, uint8_t, const char *, const void *, int);
// int TC_LOG_SYMBOL_NAMED(uint8_t cmd, const char *name, const void *p, int len);

/**
 * @brief       打印系统状态
 * @param[in]   status 系统状态
 * @retval
 */
DECLARE_FAKE_VALUE_FUNC(int, TC_LOG_SYSTEM_STATUS, T_TcLogSystemStatus);
// int TC_LOG_SYSTEM_STATUS(T_TcLogSystemStatus status);

/**
 * @brief       打印原始数据，内部使用
 * @param[in]   cmd 打印原始数据命令
 * @param[in]   type 数据类型
 * @param[in]   status 数据状态
 * @param[in]   p 数据地址
 * @param[in]   len 数据长度，单位byte
 * @retval
 */
DECLARE_FAKE_VALUE_FUNC(int, TC_LOG_RAWDATA, uint8_t, T_TcLogRawdataType, uint8_t, const void *, int);
// int TC_LOG_RAWDATA(uint8_t cmd, T_TcLogRawdataType type, uint8_t status, const void *p, int len);

#if LOG_TO_RAMBUFFER
/**
 * @brief       获取Ram缓冲区数据
 * @param[out]   bp 缓冲区指针，为NULL表示仅读取缓冲区数据长度
 * @retval       缓冲区数据长度
 */
DECLARE_FAKE_VALUE_FUNC(int, TC_LOG_GetRamBuffer, const char **);
// int TC_LOG_GetRamBuffer(const char **bp);
/**
 * @brief       清空缓冲区
 * @retval
 */
DECLARE_FAKE_VOID_FUNC(TC_LOG_ClrRamBuffer);
// void TC_LOG_ClrRamBuffer(void);
#endif

#endif
