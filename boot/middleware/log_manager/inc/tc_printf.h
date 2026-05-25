/**
 *****************************************************************************
 * @brief   tc_printf header file.
 *
 * @file    tc_printf.h
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

#ifndef __TC_PRINTF_H__
#define __TC_PRINTF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 换行符转换开关
 * @note  1: 输出时将 '\n' 自动转换为 "\r\n"
 */
#define CONVERT_CR_2_CRLF        1

/**
 * @brief 注册日志输出回调函数
 * @param func - 输出函数指针（原型 void func(unsigned char)）
 */
#define log_out_func(func) log_func = (void(*)(unsigned char))(func)

/**
 * @brief 全局日志输出函数指针
 * @note  由 log_out_func() 注册，由 tc_putc() 内部调用
 */
extern void (*log_func)(unsigned char);

/**
 * @brief  输出单个字符
 * @param  c - 待输出的字符
 * @retval 无
 */
void tc_putc(char c);

/**
 * @brief  输出字符串
 * @param  str - 待输出的字符串指针
 * @retval 无
 */
void tc_puts(const char *str);

/**
 * @brief  输出字符串到指定设备
 * @param  func - 目标设备输出回调函数指针
 * @param  str  - 待输出的字符串指针
 * @retval 无
 */
void tc_fputs(void (*func)(unsigned char), const char *str);

/**
 * @brief  向默认设备输出格式化字符串
 * @param  fmt - 格式化字符串指针
 * @retval 无
 */
void tc_printf(const char *fmt, ...);

/**
 * @brief  向内存缓冲区输出格式化字符串
 * @param  buff - 目标缓冲区指针
 * @param  fmt  - 格式化字符串指针
 * @retval 无
 */
void tc_sprintf(char *buff, const char *fmt, ...);

/**
 * @brief  向指定设备输出格式化字符串
 * @param  func - 目标设备输出回调函数指针
 * @param  fmt  - 格式化字符串指针
 * @retval 无
 */
void tc_fprintf(void (*func)(unsigned char), const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif /* __TC_PRINTF_H__ */
