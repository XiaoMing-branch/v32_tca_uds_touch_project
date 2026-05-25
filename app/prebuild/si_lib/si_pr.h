/**
*****************************************************************************
* @brief  模式识别，暂未使用
* @file   si_pr.h
* @author AE/FAE team
* @date   28/JUL/2023
*****************************************************************************
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <b>&copy; Copyright (c) 2023 Tinychip Microelectronics Co.,Ltd.</b>
*
*****************************************************************************
*/

#ifndef __SI_PR_H__
#define __SI_PR_H__

#ifdef __cplusplus
extern "C" {
#endif

//! @cond

//bitmap：点阵图描述结构体
typedef struct
{
    uint8_t isOnce;              //图像是否一笔完成，1表示一笔完成，0表示不是
    uint16_t width;              //X宽度
    uint16_t height;             //Y宽度
    int16_t  beginX;             //起点X
    int16_t  beginY;             //起点Y
    int16_t  endX;               //终点X
    int16_t  endY;               //终点Y
    int16_t  beginX2;            //起点X2
    int16_t  beginY2;            //起点Y2
    int16_t  endX2;              //终点X2
    int16_t  endY2;              //终点Y2
    uint8_t *buf;                //缓冲区
} T_SiBitmap;                    //点阵图描述符

//计算bitmap需要缓冲区长度
#define SiBitmapBufLen(width,height)        ((((width)+7)>>3) * (height))

//将bitmap内容清空
#define SiBitmapClear(bitmap)       (memset((bitmap).buf,0x0,SiBitmapBufLen((bitmap).width,(bitmap).height)))
//将bitmap对应x,y处置1
#define SiBitmapSetPix(bitmap,x,y)         ((bitmap).buf[(y)*(((bitmap).width+7)>>3) + ((x)>>3)] |= (0x1<<((x)&0x7)))
//读取bitmap对应x,y处是否有值，0表示无，其它表示有
#define SiBitmapGetPix(bitmap,x,y)         ((bitmap).buf[(y)*(((bitmap).width+7)>>3) + ((x)>>3)] & (0x1<<((x)&0x7)))

//*****************************************************************************************
typedef union
{
    struct
    {
        uint8_t houghLDBuf[300];               //霍夫变换缓冲区
    } slider;         //滑条识别缓冲区
    struct
    {
        uint8_t houghLDBuf[300];               //霍夫变换缓冲区
    } torf;         //true或者false识别缓冲区
    struct
    {
        uint8_t houghLDBuf[300];               //霍夫变换缓冲区
    } circle;         //圆识别缓冲区
} T_SiPrWorkBuf;      //模式识别工作缓冲区

//*****************************************************************************************
//滑条模式识别

typedef struct
{
    uint8_t minLineLen;   //最小线条长度，单位像素点--需要用户指定
    uint8_t maxOffset;    //最大偏离长度，例如水平滑动，若Y轴偏离值大于maxOffset认为滑动无效--需要用户指定
} T_SiPrSliderPara;       //算法识别参数

//最大支持bitmap 16*16，检测到有效值返回SI_RT_OK，未检测到有效值返回SI_RTERR_DISMATCH，sliderDirect：滑条方向，sliderDist：滑条距离
T_SiErrRt SiPrSlider(const T_SiBitmap *bitmap, T_SiPrWorkBuf *workBuf, const T_SiPrSliderPara *para, T_SiSliderDirect *sliderDirect, int16_t *sliderDist);

//*****************************************************************************************
//对号或错号识别

typedef struct
{
    uint8_t minLineLen;                //最小线条长度，单位像素点--需要用户指定
    uint8_t trueMaxLineNum;            //对号最大允许线条个数--需要用户指定
    uint8_t falseMaxLineNum;           //错号最大允许线条个数--需要用户指定
    uint8_t falseMinCenterPointsNum;   //错号最小中心点像素点个数--需要用户指定
} T_SiPrTorFPara;       //算法识别参数

//最大支持bitmap 16*16，检测到有效值返回SI_RT_OK，isTrue为1表示对号，为0表示错号，未检测到有效值返回SI_RTERR_DISMATCH
T_SiErrRt SiPrTorF(const T_SiBitmap *bitmap, T_SiPrWorkBuf *workBuf, const T_SiPrTorFPara *para, uint8_t *isTrue);

//*****************************************************************************************
//圆模式识别

typedef struct
{
    uint8_t score;             //得分阈值--需要用户指定
    uint8_t similarityDist;    //圆周围点允许的偏差--需要用户指定
    uint16_t similarityCnt;    //霍夫空间中圆交点个数阈值--需要用户指定
} T_SiPrCirclePara;       //算法识别参数

//最大支持bitmap 16*16，检测到有效值返回SI_RT_OK，未检测到有效值返回SI_RTERR_DISMATCH
T_SiErrRt SiPrCircle(const T_SiBitmap *bitmap, T_SiPrWorkBuf *workBuf, const T_SiPrCirclePara *para);

//! @endcond       //doxygen中隐藏

#ifdef __cplusplus
}
#endif

#endif
