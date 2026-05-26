/* PRQA S 0292 7 #3255 - Special characters in comments, no impact on code functionality */
/**
 *****************************************************************************
 * @brief   SID $B2 通过标识符读取数据（ReadByIdentifier）处理源文件
 *
 * @file    sid_0xb2.c
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
#include "fff_tc_log.h"
#else
/* PRQA S 0380 1 #3256 - Macro count exceeds C99 limit, supported by compiler extension */
#include "diagnosticIII.h"
#include "tc_log.h"
#endif

/* PRQA S 1534 1 #3261 - Unused macro defined for future extension and configuration compatibility */
#define LOG_DIAG(...)  //do{log_debug("[DIAG_0xB2] " __VA_ARGS__);}while(0)
/* PRQA S 3218 1 #3209 - File scope static variable used in one function, intentional design */
static const char *TAG = "B2";
extern l_u8 ld_read_by_id_callout(l_u8 id, l_u8 *data);

/**
 * @brief  SID $B2 通过标识符读取数据（ReadByID）处理函数
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   LIN协议读取操作，支持：
 *         - LIN_PRODUCT_IDENT：读取产品ID（Supplier ID + Function ID + Variant）
 *         - SERIAL_NUMBER：暂不支持，返回SUBFUNCTION_NOT_SUPPORTED
 *         - 用户自定义ID范围（LIN_READ_USR_DEF_MIN~MAX）：通过ld_read_by_id_callout()回调处理
 *         执行前先校验Supplier ID和Function ID匹配性。
 * @retval None (通过 lin_diag_positive_notify / lin_diag_negative_notify 返回)
 */
/* PRQA S 2071 2 #3269 - Language extension used for compiler and hardware optimization */
/* PRQA S 2889 1 #3257 - Multiple return statements for logical clarity and efficiency */
__WEAK void lin_diagservice_read_by_identifier(uint8_t *ptr, uint16_t length)
{
    (void)length;
    uint8_t id;
    uint16_t supid, fid;

    /* Get supplier and function indentification in request */
    supid = ((uint16_t)ptr[3] << 8) | ptr[2];
    fid = ((uint16_t)ptr[5] << 8) | ptr[4];
    /* Check Supplier ID and Function ID */

    if (((supid != (uint16_t)product_id.supplier_id) && (supid != (uint16_t)LD_ANY_SUPPLIER)) || \
        ((fid != (uint16_t)product_id.function_id) && (fid != (uint16_t)LD_ANY_FUNCTION)))
    {
        tl_slaveresp_cnt = 0;
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
        tl_service_status = LD_SERVICE_IDLE;
#endif /* End (_TL_FRAME_SUPPORT_ = = _TL_MULTI_FRAME_) */
/* PRQA S 3432 3 #3267 - Macro arguments are safely used without unintended operator precedence issues */
/* PRQA S 2742 2 #2742 - The controlling expression is constant false by design (e.g., release log disabled). */
/* PRQA S 2880 1 #2880 - Code is unreachable due to constant false condition, which is intentional for log level control. */
        TC_LOGI(TAG, "no reply %x-%x\n", supid, fid);
        return;
    }

    id = ptr[1];

    switch (ptr[1])
    {
        case LIN_PRODUCT_IDENT:
            ptr[1] = (uint8_t)(product_id.supplier_id & 0xFFu);
            ptr[2] = (uint8_t)(product_id.supplier_id >> 8);
            ptr[3] = (uint8_t)(product_id.function_id & 0xFFu);
            ptr[4] = (uint8_t)(product_id.function_id >> 8);
            ptr[5] = product_id.variant;
            lin_diag_positive_notify(ptr[0], &ptr[1], 5);
            break;

        case SERIAL_NUMBER:
            lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
            break;

        default:
            if ((id >= (uint8_t)LIN_READ_USR_DEF_MIN) && (id <= (uint8_t)LIN_READ_USR_DEF_MAX))
            {
                uint8_t data_callout[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                uint8_t data_len = 0;
                uint8_t retval = ld_read_by_id_callout(id, data_callout);

                /*If the User ID is supported, make positive response*/
                if (retval == (uint8_t)LD_POSITIVE_RESPONSE)
                {
                    for (uint8_t i = 0; i < 5u; i++)
                    {
                        if (data_callout[4u - i] != 0xFFu)
                        {
                            data_len = 5u - i;
                            break;
                        }
                    }

                    if (data_len > 0u)
                    {
                        lin_diag_positive_notify(ptr[0], &data_callout[0], data_len);
                    }
                    else
                    {
                        /* Make a negative slave response PDU */
                        lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
                    }
                }
                else if (retval == (uint8_t)LD_NEGATIVE_RESPONSE)
                    /*If the User ID is not supported, make negative response*/
                {
                    /* Make a negative slave response PDU */
                    lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
                }
                else if (retval == (uint8_t)LD_ID_NO_RESPONSE)
                {
                    /*Do not answer*/
                }else{(void)0;}
            }
            else
            {
                /* Make a negative slave response PDU */
                lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
            }

            break;
    } /* End of switch */
}

