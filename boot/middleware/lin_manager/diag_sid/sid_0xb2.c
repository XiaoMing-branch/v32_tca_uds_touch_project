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
#include "logging.h"

#define LOG_DIAG(...)  //do{log_debug("[DIAG_0xB2] " __VA_ARGS__);}while(0)
extern l_u8 ld_read_by_id_callout(l_u8 id, l_u8 *data);

/**
 * @brief  SID $B2 ReadByIdentifier读取LIN从节点标识符
 * @param  ptr - UDS请求报文指针
 * @param  length - 报文长度
 * @note   请求格式: ptr[2-5]=SupplierID+FunctionID验证; ptr[1]=读取ID
 *         LIN_PRODUCT_IDENT(0): 返回SupplierID+FunctionID+Variant
 *         SERIAL_NUMBER(1): 返回SUBFUNCTION_NOT_SUPPORTED(当前不支持)
 *         LIN_READ_USR_DEF_MIN~MAX: 通过ld_read_by_id_callout回调读取用户自定义数据
 *         先验证SupplierID/FunctionID匹配，不匹配则静默终止
 * @retval None
 */
void lin_diag_read_by_identifier(uint8_t *ptr, uint16_t length)
{
    uint8_t id;
    uint16_t supid, fid;

    /* Get supplier and function indentification in request */
    supid = (uint16_t)(ptr[3] << 8) | ptr[2];
    fid = (uint16_t)(ptr[5] << 8) | ptr[4];
    /* Check Supplier ID and Function ID */

    if (((supid != product_id.supplier_id) && (supid != LD_ANY_SUPPLIER)) || \
        ((fid != product_id.function_id) && (fid != LD_ANY_FUNCTION)))
    {
        tl_slaveresp_cnt = 0;
#if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
        tl_service_status = LD_SERVICE_IDLE;
#endif /* End (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_) */
        return;
    }

    id = ptr[1];

    switch (ptr[1])
    {
        case LIN_PRODUCT_IDENT:
            ptr[1] = (uint8_t)(product_id.supplier_id & 0xFF);
            ptr[2] = (uint8_t)(product_id.supplier_id >> 8);
            ptr[3] = (uint8_t)(product_id.function_id & 0xFF);
            ptr[4] = (uint8_t)(product_id.function_id >> 8);
            ptr[5] = product_id.variant;
            lin_diag_positive_notify(ptr[0], &ptr[1], 5);
            break;

        case SERIAL_NUMBER:
            lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
            break;

        default:
            if (id >= LIN_READ_USR_DEF_MIN && id <= LIN_READ_USR_DEF_MAX)
            {
                uint8_t data_callout[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                uint8_t data_len = 0;
                uint8_t retval = ld_read_by_id_callout(id, data_callout);

                /*If the User ID is supported, make positive response*/
                if (retval == LD_POSITIVE_RESPONSE)
                {
                    for (uint8_t i = 0; i < 5; i++)
                    {
                        if (data_callout[i] != 0xFF)
                        {
                            for (uint8_t j = 0; j < 5; j ++)
                            {
                                if (data_callout[4 - j] != 0xFF)
                                {
                                    data_len = 5 - j;
                                    break;
                                }
                            }

                            lin_diag_positive_notify(ptr[0], &data_callout[0], data_len);
                            break;
                        }

                        /* If all data_callout is 0xFF, then make negative response*/
                        if (i == 4)
                        {
                            /* Make a negative slave response PDU */
                            lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
                        }

                    }
                }
                else if (retval == LD_NEGATIVE_RESPONSE)
                    /*If the User ID is not supported, make negative response*/
                {
                    /* Make a negative slave response PDU */
                    lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
                }
                else if (retval == LD_ID_NO_RESPONSE)
                {
                    /*Do not answer*/
                    //                     tl_slaveresp_cnt = 0;
                    // #if (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_)
                    //                     tl_service_status = LD_SERVICE_IDLE;
                    // #endif /* End (_TL_FRAME_SUPPORT_ == _TL_MULTI_FRAME_) */
                }

            }
            else
            {
                /* Make a negative slave response PDU */
                lin_diag_negative_notify(ptr[0], SUBFUNCTION_NOT_SUPPORTED);
            }

            break;
    } /* End of switch */
}

