/**
 *****************************************************************************
 * @brief   lin dianosticiii source file.
 *
 * @file    diagnosticiii.c
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

#include "test_config.h"
#ifdef ENABLE_TEST_MODE
#include "fff_diagnosticIII.h"
#include "fff_utilities.h"
#include "fff_store_manager.h"
#include "fff_measure.h"
#include "fff_colormixing.h"
#else
#include "diagnosticIII.h"
#include "utilities.h"
#include "store_manager.h"
#include "measure.h"
#include "colormixing.h"
#endif

/**
 * @brief  LIN接收超时控制标志
 * @note   全局变量，控制LIN报文接收是否启用超时机制
 *         - true: 正常模式，接收超时有效
 *         - false: EMC测试模式(由EMC_TEST宏控制)，禁用接收超时以避免测试中断
 *         该变量在COMMAND_SET_LED_PWM_LIGHTING命令中通过EMC_TEST宏置false
 */
bool lin_receive_msg_timeout = true;

/**
 * @brief  0xBB SID - 设置LED/设备参数配置（Customer LED Configuration SET）
 *         根据命令字段(command)写入LED RGB色坐标、白点配置、PN电压典型值、
 *         电流、PWM灯光控制、Luv/RGBL/Yxy灯光控制、寄存器配置等
 * @param  ptr - UDS请求报文指针（ptr[1]<<8|ptr[2]为子命令）
 * @param  length - 报文长度
 * @note   解析ptr中的命令字段，通过switch-case分发处理各个SET子命令，
 *         成功调用lin_diag_positive_notify()，失败返回SUBFUNCTION_NOT_SUPPORTED
 * @retval None
 */
void lin_diag_led_config_set(uint8_t *ptr, uint16_t length)
{
    LedCoordinate_t *ptr_led_param __attribute__((unused));
    uint8_t buffer[24] __attribute__((unused));
    uint16_t command = (ptr[1] << 8) + ptr[2];
    uint8_t resp_type = POSITIVE;

    switch (command)
    {
        /**
         * @brief  子命令: 设置LED RGB色坐标和亮度参数
         * @note   解析ptr[4..23]中的RGB色温、色坐标xy和亮度intensity，
         *         所有uint16经大端转换后存入LedCoordinate_t结构体，
         *         调用store_generic_data_set()保存到存储区，
         *         再调用cm_load_led_params(false)重载配色参数
         */
        case COMMAND_SET_LED_RGB_PARAM:
        {
            ptr_led_param = (LedCoordinate_t *)&buffer[0];

            ptr_led_param->red.temperature = ptr[4];
            ptr_led_param->green.temperature = ptr[5];
            ptr_led_param->blue.temperature = ptr[6];
            ptr_led_param->red.x = (uint16_t)endian_swap_func((uint8_t *)&ptr[7], sizeof(uint16_t));
            ptr_led_param->red.y = (uint16_t)endian_swap_func((uint8_t *)&ptr[9], sizeof(uint16_t));
            ptr_led_param->green.x = (uint16_t)endian_swap_func((uint8_t *)&ptr[11], sizeof(uint16_t));
            ptr_led_param->green.y = (uint16_t)endian_swap_func((uint8_t *)&ptr[13], sizeof(uint16_t));
            ptr_led_param->blue.x = (uint16_t)endian_swap_func((uint8_t *)&ptr[15], sizeof(uint16_t));
            ptr_led_param->blue.y = (uint16_t)endian_swap_func((uint8_t *)&ptr[17], sizeof(uint16_t));
            ptr_led_param->red.intensity = (uint16_t)endian_swap_func((uint8_t *)&ptr[19], sizeof(uint16_t));
            ptr_led_param->green.intensity = (uint16_t)endian_swap_func((uint8_t *)&ptr[21], sizeof(uint16_t));
            ptr_led_param->blue.intensity = (uint16_t)endian_swap_func((uint8_t *)&ptr[23], sizeof(uint16_t));

            store_generic_data_set(LED_CHANNLE_0, LED_RGB_PARAM, &buffer[0], LED_RGB_SIZE);
            cm_load_led_params(false);
        }
        break;

        /**
         * @brief  子命令: 设置LED典型PN电压
         * @note   解析ptr[3..]中PN电压数据，所有uint16经大端转换后，
         *         调用store_generic_data_set()保存到存储区，
         *         再调用cm_load_led_params(false)重载配色参数
         */
        case COMMAND_SET_LED_TYPICAL_PN_VOLT:
        {

            for (uint8_t i = 0; i < LED_TEMP_PN_VOLT_SIZE; i += 2)
            {
                endian_swap_func(&ptr[3 + i], sizeof(uint16_t));
            }

            store_generic_data_set(LED_CHANNLE_0, LED_PN_VOLT_PARAM, &ptr[3], LED_TEMP_PN_VOLT_SIZE);
            cm_load_led_params(false);
        }
        break;

        /**
         * @brief  子命令: 设置白点配置参数
         * @note   解析ptr[3..]中白点配置数据，所有uint16经大端转换后，
         *         调用store_generic_data_set()保存，
         *         再调用cm_load_led_params(false)重载配色参数
         */
        case COMMAND_SET_WHITE_POINT_CONFIG:
        {
            for (uint8_t i = 0; i < sizeof(CommLedGeneralParam_t); i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[3 + i], sizeof(uint16_t));
            }

            store_generic_data_set(LED_CHANNLE_0, LED_WHITE_COLOR_PARAM, &ptr[3], LED_WHITE_COLOR_SIZE);
            cm_load_led_params(false);
        }
        break;

        /**
         * @brief  子命令: 设置温度补偿开关
         * @note   调用meas_pn_sample_status_set()设置LED通道的温度采样状态
         */
        case COMMAND_SET_TEMPERATURE_ADJUST:
        {
            meas_pn_sample_status_set(LED_CHANNLE_0, ptr[4]);
        }
        break;

        /**
         * @brief  子命令: 设置LED RGB工作电流
         * @note   调用pal_led_current_set()设置LED通道的驱动电流值
         */
        case COMMAND_SET_LED_RGB_CURRENT:
        {
            pal_led_current_set(LED_CHANNLE_0, ptr[4]);
        }
        break;

        /**
         * @brief  子命令: PWM调光控制
         * @note   调用cm_set_target_pwm()设置R/G/B三通道PWM值和持续时间，
         *         在EMC_TEST模式下将lin_receive_msg_timeout置false以禁用接收超时
         */
        case COMMAND_SET_LED_PWM_LIGHTING:
        {
            cm_set_target_pwm(ptr[3], ptr[4], ptr[5], ptr[6] << 8 | ptr[7]);
#ifdef EMC_TEST
            lin_receive_msg_timeout = false;
#endif
        }
        break;

        /**
         * @brief  子命令: Luv颜色空间调光控制
         * @note   调用cm_set_target_Luv()设置L/u/v目标和相对亮度，
         *         然后cm_target_Luv_lighting(1)执行点亮
         */
        case COMMAND_SET_LED_LUV_LIGHTING:
        {
            /* default use relative intensity */
            cm_set_target_Luv(ptr[3] << 8 | ptr[4], 0, ptr[5] << 8 | ptr[6], ptr[7] << 8 | ptr[8], ptr[9] << 8 | ptr[10]);
            cm_target_Luv_lighting(1);
        }
        break;

        /**
         * @brief  子命令: RGBL调光控制
         * @note   调用cm_set_target_RGBL()设置R/G/B/L目标和亮度，
         *         然后cm_target_RGBL_lighting(1)执行点亮
         */
        case COMMAND_SET_LED_RGBL_LIGHTING:
        {
            cm_set_target_RGBL(ptr[3], ptr[4], ptr[5], ptr[6] << 8 | ptr[7], ptr[8] << 8 | ptr[9]);
            cm_target_RGBL_lighting(1);
        }
        break;

        /**
         * @brief  子命令: PN电压触发（当前为空操作）
         * @note   保留命令，当前未实现具体功能
         */
        case COMMAND_SET_LED_PN_VOLT_TRIGGER:
        {
            /* do nothing now*/
        }
        break;

        /**
         * @brief  子命令: 设置相对亮度系数
         * @note   调用store_generic_data_set()保存相对因子到存储区，
         *         调用cm_load_led_params(false)重载配色参数使新系数生效
         */
        case COMMAND_SET_RELATIVE_FACTOR:
        {
            store_generic_data_set(LED_CHANNLE_0, LED_RELATIVE_FACTOR_PARAM, &ptr[3], LED_RELATIVE_FACTOR_SIZE);
            cm_load_led_params(false);
        }
        break;

        /**
         * @brief  子命令: 设置静态PN采样使能
         * @note   调用meas_pn_static_sample_status_set()使能/禁用LED通道静态PN采样
         */
        case COMMAND_SET_STATIC_PN_SAMPLE:
        {
            meas_pn_static_sample_status_set(LED_CHANNLE_0, ptr[3]);
        }
        break;

        /**
         * @brief  子命令: Cx/Cy/Y颜色空间调光控制
         * @note   调用cm_set_target_Yxy()设置目标色坐标和亮度，
         *         然后cm_target_Yxy_lighting(1)执行点亮
         */
        case COMMAND_SET_LED_CXY_LIGHTING:
        {
            cm_set_target_Yxy(ptr[3] << 8 | ptr[4], ptr[5] << 8 | ptr[6], ptr[7] << 8 | ptr[8], 0, ptr[9] << 8 | ptr[10]);
            cm_target_Yxy_lighting(1);
        }
        break;

        /**
         * @brief  子命令: 白平衡测试调光
         * @note   调用cm_set_target_Yxy()设置白平衡目标色坐标和亮度（与CXY调光相同），
         *         然后cm_target_Yxy_lighting(1)执行点亮
         */
        case COMMAND_SET_WHITETEST_LIGHTING:
        {
            cm_set_target_Yxy(ptr[3] << 8 | ptr[4], ptr[5] << 8 | ptr[6], ptr[7] << 8 | ptr[8], 0, ptr[9] << 8 | ptr[10]);
            cm_target_Yxy_lighting(1);
        }
        break;

        /**
         * @brief  子命令: 恢复出厂设置并复位
         * @note   调用store_manager_clear()擦除所有存储配置，
         *         调用NVIC_SystemReset()执行系统复位
         */
        case COMMAND_SET_LED_RGB_PARAM_RESET:
        {
            store_manager_clear();
            NVIC_SystemReset();
        }
        break;

        /**
         * @brief  子命令: 设置寄存器配置
         * @note   解析ptr[3..6]为寄存器地址、ptr[7..10]为写入值，
         *         均经大端转换后调用store_reg_param_set()写入
         */
        case COMMAND_SET_REG_CFG:
        {
            endian_swap_func((uint8_t *)&ptr[3], sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[7], sizeof(uint32_t));
            store_reg_param_set(&ptr[3], &ptr[7]);
        }
        break;

        default:
            resp_type = NEGATIVE;
            break;
    }

    if (POSITIVE == resp_type)
    {
        lin_diag_positive_notify(ptr[0], NULL, 0);
    }
    else
    {
        lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
    }

}
