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

#include "diagnosticIII.h"
#include "utilities.h"
#include "store_manager.h"
#include "measure.h"
#include "colormixing.h"
#include "led_control.h"

bool lin_receive_msg_timeout = true;

/**
 * @brief  SID $BB LED配置写入(Boot版本SET)
 * @param  ptr - UDS请求报文指针; length - 报文长度
 * @note   支持多种LED参数写入/控制:
 *         - RGB参数/典型PN电压/白点配置/电流设置
 *         - PWM/Luv/RGBL/Cxy多种调光模式控制
 *         - PN触发/相对因子/静态PN采样设置
 *         - 寄存器配置/参数恢复出厂(清空存储+复位)
 *         成功回复正响应, 未知命令回复SUBFUNCTION_NOT_SUPPORTED
 * @retval None
 */
void lin_diag_led_config_set(uint8_t *ptr, uint16_t length)
{
    cm_led_param_t *ptr_led_param __attribute__((unused));
    uint8_t buffer[24] __attribute__((unused));
    led_type_e rgb __attribute__((unused));
    uint16_t command = (ptr[1] << 8) + ptr[2];
    led_channel_e channel __attribute__((unused));
    color_coordinate_t color_coordinate __attribute__((unused));
    color_rgbl_t color_rgbl __attribute__((unused));
    color_pwm_t color_pwm __attribute__((unused));
    uint8_t resp_type = POSITIVE;

    switch (command)
    {
        case COMMAND_SET_LED_RGB_PARAM:
        {
            ptr_led_param = (cm_led_param_t *)&buffer[0];
            channel = ptr[3];
            for (rgb = LED_R; rgb < LED_TYPE_MAX; rgb++)
            {
                ptr_led_param->color[rgb].led_temp = ptr[4 + rgb];
                ptr_led_param->color[rgb].xy.x = (uint16_t)endian_swap_func((uint8_t *)&ptr[7 + 4 * rgb], sizeof(uint16_t));
                ptr_led_param->color[rgb].xy.y = (uint16_t)endian_swap_func((uint8_t *)&ptr[9 + 4 * rgb], sizeof(uint16_t));
                ptr_led_param->color[rgb].intensity = (uint16_t)endian_swap_func((uint8_t *)&ptr[19 + 2 * rgb], sizeof(uint16_t));
            }

            store_generic_data_set(channel, LED_RGB_PARAM, &buffer[0], LED_RGB_SIZE);
            cm_load_led_params(channel);
        }
        break;

        case COMMAND_SET_LED_TYPICAL_PN_VOLT:
        {
            channel = ptr[11];
            for (uint8_t i = 0; i < LED_TEMP_PN_VOLT_SIZE; i += 2)
            {
                endian_swap_func(&ptr[3 + i], sizeof(uint16_t));
            }

            store_generic_data_set(channel, LED_PN_VOLT_PARAM, &ptr[3], LED_TEMP_PN_VOLT_SIZE);
            cm_load_led_params(channel);
        }
        break;

        case COMMAND_SET_WHITE_POINT_CONFIG:
        {
            for (uint8_t i = 0; i < sizeof(cm_white_point_param_t); i += 2)
            {
                endian_swap_func((uint8_t *)&ptr[3 + i], sizeof(uint16_t));
            }
            channel = ptr[13];
            store_generic_data_set(channel, LED_WHITE_COLOR_PARAM, &ptr[3], LED_WHITE_COLOR_SIZE);
            cm_load_led_params(channel);
        }
        break;

        case COMMAND_SET_LED_RGB_CURRENT:
        {
            channel = ptr[3];
            led_ctrl_current_set(channel, &ptr[4]);
        }
        break;

        case COMMAND_SET_LED_PWM_LIGHTING:
        {
            channel = ptr[8];
            color_pwm.pwm_r = ptr[3];
            color_pwm.pwm_g = ptr[4];
            color_pwm.pwm_b = ptr[5];
            color_pwm.time = ptr[6] << 8 | ptr[7];
            cm_set_target_pwm(channel, &color_pwm);
#ifdef EMC_TEST
            lin_receive_msg_timeout = false;
#endif
        }
        break;

        case COMMAND_SET_LED_LUV_LIGHTING:
        {
            channel = ptr[11];
            /* default use relative intensity */
            color_coordinate.uv.u = ptr[3] << 8 | ptr[4];
            color_coordinate.uv.v = ptr[5] << 8 | ptr[6];
            color_coordinate.level = ptr[7] << 8 | ptr[8];
            color_coordinate.time = ptr[9] << 8 | ptr[10];
            color_coordinate.dimming_mode = 0;

            cm_set_target_Luv(channel, &color_coordinate);
            cm_target_Luv_lighting(channel, 1);
        }
        break;

        case COMMAND_SET_LED_RGBL_LIGHTING:
        {
            channel = ptr[10];
            color_rgbl.r = ptr[3];
            color_rgbl.g = ptr[4];
            color_rgbl.b = ptr[5];
            color_rgbl.level = ptr[6] << 8 | ptr[7];
            color_rgbl.time = ptr[8] << 8 | ptr[9];
            cm_set_target_RGBL(channel, &color_rgbl);
            cm_target_RGBL_lighting(channel, 1);
        }
        break;

        case COMMAND_SET_LED_PN_VOLT_TRIGGER:
        {
            /* do nothing now*/
        }
        break;

        case COMMAND_SET_RELATIVE_FACTOR:
        {
            channel = LED_CHANNEL_0;
            store_generic_data_set(channel, LED_RELATIVE_FACTOR_PARAM, &ptr[3], LED_RELATIVE_FACTOR_SIZE);
            cm_load_led_params(channel);
        }
        break;

        case COMMAND_SET_STATIC_PN_SAMPLE:
        {
            channel = ptr[4];
            meas_pn_static_sample_status_set(channel, ptr[3]);
            // for (led_channel_e ch = 0; ch < LED_CHANNEL_MAX; ch++)
            // {
            //     meas_pn_static_sample_status_set(ch, ptr[3]);
            // }
        }
        break;

        case COMMAND_SET_LED_CXY_LIGHTING:
        case COMMAND_SET_WHITETEST_LIGHTING:
        {
            channel = ptr[11];
            color_coordinate.xy.x = ptr[3] << 8 | ptr[4];
            color_coordinate.xy.y = ptr[5] << 8 | ptr[6];
            color_coordinate.level = ptr[7] << 8 | ptr[8];
            color_coordinate.time = ptr[9] << 8 | ptr[10];
            color_coordinate.dimming_mode = 0;
            cm_set_target_Yxy(channel, &color_coordinate);
            cm_target_Yxy_lighting(channel,1);
        }
        break;

        case COMMAND_SET_LED_RGB_PARAM_RESET:
        {
            store_manager_clear();
            NVIC_SystemReset();
        }
        break;

        case COMMAND_SET_REG_CFG:
        {
            endian_swap_func((uint8_t *)&ptr[3], sizeof(uint32_t));
            endian_swap_func((uint8_t *)&ptr[7], sizeof(uint32_t));
            store_reg_param_set(&ptr[3], &ptr[7]);
        }
        break;

        default:
        {
            resp_type = NEGATIVE;
        }
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
