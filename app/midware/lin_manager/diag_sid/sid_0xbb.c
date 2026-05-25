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

bool lin_receive_msg_timeout = true;

/********************************************************
** \brief   lin_diag_led_config_get
** \param   uint8_t*                    ptr
** \param   uint16_t                    length
** \retval  None
*********************************************************/
void lin_diag_led_config_set(uint8_t *ptr, uint16_t length)
{
    LedCoordinate_t *ptr_led_param __attribute__((unused));
    uint8_t buffer[24] __attribute__((unused));
    uint16_t command = (ptr[1] << 8) + ptr[2];
    uint8_t resp_type = POSITIVE;

    switch (command)
    {
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

        case COMMAND_SET_TEMPERATURE_ADJUST:
        {
            meas_pn_sample_status_set(LED_CHANNLE_0, ptr[4]);
        }
        break;

        case COMMAND_SET_LED_RGB_CURRENT:
        {
            pal_led_current_set(LED_CHANNLE_0, ptr[4]);
        }
        break;

        case COMMAND_SET_LED_PWM_LIGHTING:
        {
            cm_set_target_pwm(ptr[3], ptr[4], ptr[5], ptr[6] << 8 | ptr[7]);
#ifdef EMC_TEST
            lin_receive_msg_timeout = false;
#endif
        }
        break;

        case COMMAND_SET_LED_LUV_LIGHTING:
        {
            /* default use relative intensity */
            cm_set_target_Luv(ptr[3] << 8 | ptr[4], 0, ptr[5] << 8 | ptr[6], ptr[7] << 8 | ptr[8], ptr[9] << 8 | ptr[10]);
            cm_target_Luv_lighting(1);
        }
        break;

        case COMMAND_SET_LED_RGBL_LIGHTING:
        {
            cm_set_target_RGBL(ptr[3], ptr[4], ptr[5], ptr[6] << 8 | ptr[7], ptr[8] << 8 | ptr[9]);
            cm_target_RGBL_lighting(1);
        }
        break;

        case COMMAND_SET_LED_PN_VOLT_TRIGGER:
        {
            /* do nothing now*/
        }
        break;

        case COMMAND_SET_RELATIVE_FACTOR:
        {
            store_generic_data_set(LED_CHANNLE_0, LED_RELATIVE_FACTOR_PARAM, &ptr[3], LED_RELATIVE_FACTOR_SIZE);
            cm_load_led_params(false);
        }
        break;

        case COMMAND_SET_STATIC_PN_SAMPLE:
        {
            meas_pn_static_sample_status_set(LED_CHANNLE_0, ptr[3]);
        }
        break;

        case COMMAND_SET_LED_CXY_LIGHTING:
        {
            cm_set_target_Yxy(ptr[3] << 8 | ptr[4], ptr[5] << 8 | ptr[6], ptr[7] << 8 | ptr[8], 0, ptr[9] << 8 | ptr[10]);
            cm_target_Yxy_lighting(1);
        }
        break;

        case COMMAND_SET_WHITETEST_LIGHTING:
        {
            cm_set_target_Yxy(ptr[3] << 8 | ptr[4], ptr[5] << 8 | ptr[6], ptr[7] << 8 | ptr[8], 0, ptr[9] << 8 | ptr[10]);
            cm_target_Yxy_lighting(1);
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
