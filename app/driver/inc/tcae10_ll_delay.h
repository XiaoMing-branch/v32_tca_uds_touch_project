/**
  ******************************************************************************
  * @brief  delay header file.
  *
  * @file   tcae10_ll_delay.h
  * @author AE/FAE team
  * @date
  ******************************************************************************
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <b>&copy; Copyright (c) 2020 Tinychip Microelectronics Co.,Ltd.</b>
  *
  ******************************************************************************
  */
  
  
  
#ifndef __TCAE10_LL_DELAY_H__
#define __TCAE10_LL_DELAY_H__



#include "tcae10.h"



 /* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif



/**
 * @brief  SysTick时钟源选择：HCLK
 */
#define SysTick_CLKSource_HCLK   0
/**
 * @brief  SysTick时钟源选择：STCLK
 */
#define SysTick_CLKSource_STCLK  1 



/**
 * @brief  毫秒级延时
 * @param u32Cnt - 延时毫秒数
 */
void delay1ms(uint32_t u32Cnt);
/**
 * @brief  100微秒级延时
 * @param u32Cnt - 延时100微秒的个数
 */
void delay100us(uint32_t u32Cnt);
/**
 * @brief  微秒级延时
 * @param u32Cnt - 延时微秒数
 */
void delay1us(uint32_t u32Cnt);
/**
 * @brief  配置SysTick时钟源
 * @param source - 时钟源选择（SysTick_CLKSource_HCLK 或 SysTick_CLKSource_STCLK）
 */
void ll_systick_clkconfig(uint32_t source);

#ifdef __cplusplus
}
#endif
#endif /* __TCAE10_LL_DELAY_H__ */
