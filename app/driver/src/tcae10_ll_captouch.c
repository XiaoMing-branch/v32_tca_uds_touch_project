/**
  ******************************************************************************
  * @brief   captouch  driver source file..
  *
  * @file   captouch.c
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

#include "tcae10_ll_captouch.h"


/**
 * @brief   使能电容触摸中断
 * @param   touch_int - 要使能的中断标志，可取@ref captouch interrupt Definitions中的值或组合
 * @note    写IMR寄存器清除对应位可使能中断（低电平有效）
 * @retval  None
 */
void CapTouch_InterruptEnable(uint8_t touch_int)
{
    CAPTOUCH->IMR &= ~(touch_int);               /* 清除IMR中对应位，使能中断 */
}

/**
 * @brief   禁能电容触摸中断
 * @param   touch_int - 要禁能的中断标志，可取@ref captouch interrupt Definitions中的值或组合
 * @note    写IMR寄存器设置对应位可禁能中断
 * @retval  None
 */
void CapTouch_InterruptDisable(uint8_t touch_int)
{
    CAPTOUCH->IMR |= touch_int;                  /* 设置IMR中对应位，禁能中断 */
}

/**
 * @brief   清除电容触摸中断标志
 * @param   touch_int - 要清除的中断标志，可取@ref captouch interrupt Definitions中的值或组合
 * @retval  None
 */
void CapTouch_InterruptClear(uint8_t touch_int)
{
    CAPTOUCH->ICR |= touch_int;                  /* 写ICR清除对应中断标志 */
}
/**
 * @brief   使能或禁能电容触摸检测
 * @param   state - ENABLE使能触摸检测，DISABLE禁能
 * @retval  None
 */
void CapTouch_Enable(FunctionalState state)
{
    CAPTOUCH->CTRL1_F.CAPTOUCH_EN = (state ? 1 : 0);  /* 设置触摸使能位 */
}

/**
 * @brief   软件触发电容触摸检测
 * @param   None
 * @note    写CTRL0寄存器的SW_TRIG位启动一次触摸检测
 * @retval  None
 */
void CapTouch_SoftwareTrig(void)
{
    CAPTOUCH->CTRL0_F.SW_TRIG = 1;              /* 写1触发软件触摸检测 */
}

/**
 * @brief   设置电容触摸频率跳变（Hopping）
 * @param   period - 跳变周期（0~7）
 * @note    频率跳变用于改善EMC性能，将扫描频率在多个频点间跳变。
 *         跳变计数值从0开始递增到period，避免跳变计数值为0的bug
 * @retval  None
 */
void CapTouch_Hopping(uint8_t period)
{
    uint8_t i;
    uint8_t shift_cnt = 0;

    CAPTOUCH->CTRL4 &= ~((uint32_t)0x7U << (period * 3));  /* 清除目标周期的跳变值 */
    for (i = 0; i <= period; ++i)                            /* 设置跳变计数值：0,1,2,3... */
    {
        CAPTOUCH->CTRL4 &= ~((uint32_t)0x7U << (i * 3));    /* 清除当前频点的跳变值 */
        shift_cnt = i * 3;                                   /* 每3位对应一个频点 */
        CAPTOUCH->CTRL4 |= ((uint32_t)i << shift_cnt);       /* 设置跳变计数值 */
    }
    CAPTOUCH->CTRL4_F.HOP_PERIOD = period;                  /* 设置跳变周期 */
}
    CAPTOUCH->CTRL4_F.HOP_PERIOD = period;
}

/**
 * @brief   获取电容触摸中断状态
 * @param   touch_int - 要查询的中断标志，可取@ref captouch interrupt Definitions中的值或组合
 * @retval  返回指定中断标志的状态（true=中断发生）
 */
bool CapTouch_InterruptStatusGet(uint8_t touch_int)
{
    return (ADC->ISR & (touch_int)) ;            /* 读取ISR寄存器获取中断状态 */
}

/**
 * @brief   初始化电容触摸外设
 * @param   touch_cfg - 指向触摸配置结构体的指针
 * @note    配置CTRL0~CTRL3所有控制寄存器，包括校准选择、屏蔽驱动、
 *         通道选择、采样时间、传输循环、IDAC电流等
 * @retval  None
 */
void CapTouch_Init(CapTouch_InitConfig_t *touch_cfg)
{
    /*CTRL0*/
    CAPTOUCH->CTRL0_F.CAL_SEL = touch_cfg->Calibration_Sel;
    CAPTOUCH->CTRL0_F.SHLD_I = touch_cfg->Shield_Iout;
    CAPTOUCH->CTRL0_F.SHLD_SEL = touch_cfg->Shield_Sel;
    CAPTOUCH->CTRL0_F.CHNL_SEL = touch_cfg->Channel_Sel;
    /*CTRL1*/
    CAPTOUCH->CTRL1_F.SAMP_OVF_TIME = touch_cfg->Sample_Ovf_Time;
    CAPTOUCH->CTRL1_F.SAMP_OVF_EN = touch_cfg->Sample_Ovf_Enable;
    CAPTOUCH->CTRL1_F.TRAN_LOOP = touch_cfg->Transfer_Loop;
    CAPTOUCH->CTRL1_F.TW_TRIG_EN = touch_cfg->Tinywork_Trigger_Enable;
    CAPTOUCH->CTRL1_F.TRIG_ADC_EN = touch_cfg->Trigger_adc_en;
    CAPTOUCH->CTRL1_F.DSTSW_DIS = touch_cfg->dstsw_dis;
    CAPTOUCH->CTRL1_F.CMP_EN = touch_cfg->cmp_en;
    CAPTOUCH->CTRL1_F.SHLD_EN = touch_cfg->shld_en;
    CAPTOUCH->CTRL1_F.CREF_SEL = touch_cfg->cref_sel;
    CAPTOUCH->CTRL1_F.CAPTOUCH_MODE = touch_cfg->Captouch_Mode;
    /*CTRL2*/
    CAPTOUCH->CTRL2_F.INIT_TIME = touch_cfg->Init_time;
    CAPTOUCH->CTRL2_F.TRAN_TIME = touch_cfg->tran_time;
    CAPTOUCH->CTRL2_F.CHG_TIME = touch_cfg->chg_time;
    /*CTRL3*/
    CAPTOUCH->CTRL3_F.IDAC_INP = touch_cfg->idac_inp;
    CAPTOUCH->CTRL3_F.IDAC_INN = touch_cfg->idac_inn;
    CAPTOUCH->CTRL3_F.SW_IDAC_SEL_P = touch_cfg->sw_idac_sel_p;
    CAPTOUCH->CTRL3_F.SW_IDAC_SEL_N = touch_cfg->sw_idac_sel_n;
    CAPTOUCH->CTRL3_F.IDAC_EN = touch_cfg->idac_en;

}



