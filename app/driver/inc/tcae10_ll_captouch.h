/**
  ******************************************************************************
  * @brief  captouch haeader header file.
  *
  * @file   tcae10_ll_captouch.h
  * @author	AE/FAE team
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
  
  
  
#ifndef __TCAE10_LL_CAPTOUCH_H__
#define __TCAE10_LL_CAPTOUCH_H__

#include "tcae10.h"
#include <stdbool.h>

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup TCAE01_DRIVER
  * @{
  */


/** @defgroup captouch interrupt Definitions
  * @{
  */
#define	CAPTOUCH_PENDING_TRIGGER		CAPTOUCH_IMR_IMR_SET(8)
#define	CAPTOUCH_SAMP_OVF				CAPTOUCH_IMR_IMR_SET(4)
#define	CAPTOUCH_TRAN_DONE				CAPTOUCH_IMR_IMR_SET(2)
#define	CAPTOUCH_TRIG_DONE				CAPTOUCH_IMR_IMR_SET(1)
/**
  * @}
  */

/** @defgroup Calibration Select Definitions
  * @{
  */
#define	CHARGE_IDAC_REFERENECE_CAP		0
#define	DISCHARGE_REFERENCE_CAP_IDAC	1
#define	CHARGE_IDAC_CHANNEL_SENSOR		2
#define	DISCHARGE_CHANNEL_SENSOR_IDAC	3
/**
  * @}
  */

/** @defgroup Shield select Definitions
  * @{
  */
#define	CHANNEL_0_AS_SHIELD				0
#define	CHANNEL_1_AS_SHIELD				1
#define	CHANNEL_2_AS_SHIELD				2
#define	CHANNEL_3_AS_SHIELD				3
#define	CHANNEL_4_AS_SHIELD				4
#define	SHIELD_PIN_AS_SHIELD			6
#define	SOURCE_FOLLOW_SHIELD			7
/**
  * @}
  */
  
  
/** @defgroup Channel select Definitions
  * @{
  */   
#define	CAPTOUCH_CHANNEL_0						0
#define	CAPTOUCH_CHANNEL_1						1
#define	CAPTOUCH_CHANNEL_2						2
#define	CAPTOUCH_CHANNEL_3						3
#define	CAPTOUCH_CHANNEL_4						4
/**
  * @}
  */
  
/** @defgroup Select cref  Definitions
  * @{
  */  
#define	CHARGE_DISCHARGE_SENSOR_CAP		0
#define	CHARGE_DISCHARGE_REFERENCE_CAP	1
/**
  * @}
  */
  

/** @defgroup Captouch Mode Definitions
  * @{
  */  
typedef enum
{
	CALIBRATION_MODE = 0,
	CHARGE_MODE,
	DISCHARGE_MODE,	
	CHARGE_DISCHARGE_BALANCE_MODE	
}Captouch_Mode_t;
/**
  * @}
  */


typedef struct
{
	//CTRL0
	uint8_t				Calibration_Sel;
	
	uint8_t				Shield_Iout;
	
	uint8_t				Shield_Sel;
	
	uint8_t				Channel_Sel;
	
	//CTRL1
	uint16_t			Sample_Ovf_Time;/*Max time to wait for ADC sample ,(N+1)*fclk*/
	
	uint8_t				Transfer_Loop;/*Transfer loops for each touch sensor time , N+1*/
		
	bool				Sample_Ovf_Enable;/*Sample wait time overflow detect enable*/
	
	bool				Tinywork_Trigger_Enable;/*Enable Tinywork hardware trigger touch*/
	
	bool				Trigger_adc_en;/*Enable to trigger ADC after transfer done*/
	
	bool				dstsw_dis;/*Channel switch selection, 
								only work for the selected channel, suggest to keep at 0*/ 
	bool				cmp_en;/*Compensation Enable*/
	
	bool				shld_en;/*Shield Enable*/
	
	uint8_t				cref_sel;/*Select cref */
	
	Captouch_Mode_t		Captouch_Mode;
	
	bool				Captouch_en;
	
	/*CTRL2*/
	uint16_t			Init_time;/*Initial time, (N+1)*fclk*/
	
	uint8_t				tran_time;/*Transfer phase time for each transfer loop, (N+1)*fclk*/
	
	uint8_t				chg_time;/*Charge or discharge phase time for each transfer loop, (N+1)*fclk*/
	
	/*CTRL3*/
	uint8_t				idac_inp;/*IDAC charge current select*/
	
	uint8_t				idac_inn;/*IDAC discharge current select*/
	
	uint8_t				idac_time;/*IDAC transfer time , (N+1)*fclk*/
	
	uint8_t				sw_idac_sel_p;/*IDAC charge mode*/
	
	uint8_t				sw_idac_sel_n;/*IDAC discharge mode*/
	
	uint8_t				sw_idac_model;/*software mode control for idac*/
	
	bool				idac_en;/*IDAC enable, 
								need to be set for calibration mode or compensation mode*/


}CapTouch_InitConfig_t;

/**
 * @brief  使能电容触摸中断
 * @param touch_int - 触摸中断标志位
 */
void CapTouch_InterruptEnable(uint8_t touch_int);
/**
 * @brief  禁能电容触摸中断
 * @param touch_int - 触摸中断标志位
 */
void CapTouch_InterruptDisable(uint8_t touch_int);
/**
 * @brief  清除电容触摸中断标志
 * @param touch_int - 触摸中断标志位
 */
void CapTouch_InterruptClear(uint8_t touch_int);
/**
 * @brief  使能/禁能电容触摸模块
 * @param state - ENABLE: 使能，DISABLE: 禁能
 */
void CapTouch_Enable(FunctionalState state);
/**
 * @brief  初始化电容触摸模块
 * @param touch_cfg - 触摸初始化配置结构体指针
 */
void CapTouch_Init(CapTouch_InitConfig_t *touch_cfg);
/**
 * @brief  软件触发电容触摸转换
 */
void CapTouch_SoftwareTrig(void);
/**
 * @brief  设置电容触摸跳频周期
 * @param period - 跳频周期值
 */
void CapTouch_Hopping(uint8_t period);
/**
 * @brief  获取电容触摸中断状态
 * @param touch_int - 触摸中断标志位
 * @retval true: 中断已触发，false: 中断未触发
 */
bool CapTouch_InterruptStatusGet(uint8_t touch_int);

#ifdef __cplusplus
}
#endif
/**
  * @}
  */
#endif /* __TCAE10_LL_CAPTOUCH_H__ */
