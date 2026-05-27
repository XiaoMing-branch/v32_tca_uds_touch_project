/**
 * @file touch_config.c
 * @brief 触摸配置与算法初始化实现
 *
 * 本文件实现触摸功能的硬件参数配置、算法模块初始化（滤波器、基线追踪器、判决器）、
 * 低功耗模式切换、噪音检测等功能。适用于门把手电容触摸方案（泰稀微方案）。
 *
 * @note 本文件为门把手多项目（doorctrl_m9_duotaiji）的触摸配置实现。
 *       配置参数需根据具体硬件设计进行调整。
 */
#include "si_include.h"
#include "touch_config.h"
#include "app.h"
#include "tc_halt.h"
#include "si_touch_port.h"
#include "touch_tool.h"
#include "tc_log.h"
#include "custom_diagnosticIII.h"

/** @brief 日志标签，用于模块日志输出 */
static const char *TAG = "TOUCH_CONFIG";

/** @brief 外部触摸数据结构体引用 */
extern lin_touch_data touch_data;

//diff峰值记录：690，650

//**************************************************************************************************
//***IOTouch配置

#if IOTOUCH_KEY1NUM
/** @brief 门把手滤波器配置参数（按键组1） */
const static T_SiFilterParaExtDoorctrl iotouchFilterParaDoorctrl1 =
{
    .gainers = {
        {
            .enable = 1,                    /*!< 增益器使能，1使能，0关闭--需要用户指定 */
            .offset = 6000                   /*!< 信号偏置值，原始数据会减去信号偏置值后再放入增益器，--需要用户指定 */
        },
        {
            .enable = 0,                    /*!< 增益器使能，1使能，0关闭--需要用户指定 */
            .offset = 10000                   /*!< 信号偏置值，原始数据会减去信号偏置值后再放入增益器，--需要用户指定 */
        },
    },
    .kfr = 4000,                            /*!< 跳频通道方差值，单位0.01--需要用户指定 */
};
/** @brief 门把手滤波器实例（按键组1） */
static T_SiFilterExtDoorctrl iotouchFilterDoorctrl1;
/** @brief 滤波值缓冲区（按键组1） */
static T_SiData iotouchFilterBufDoorctrl1[IOTOUCH_KEY1NUM];

/**
 * @brief 基线追踪器配置参数（按键组1，常规模式）
 * @note 使用双通道强平均基线追踪算法
 */
const static T_SiBaselineParaDoorctrl2MtStrongAvg iotouchBaselinePara1 =
{
    {
        .cntThreshold = 10,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 6,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 1,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 1,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 20,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 0, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 0, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 0 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    },
    {
        .cntThreshold = 10,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 6,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 1,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 0,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 20,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 1, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 5000, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 1200 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    }
}; /*!< iotouch基线追踪算法参数 */
/**
 * @brief 基线追踪器配置参数（按键组1，低功耗模式）
 * @note 快速步进参数，用于低功耗模式下快速追踪基线变化
 */
const static T_SiBaselineParaDoorctrl2MtStrongAvg iotouchBaselinePara12 =
{
    {
        .cntThreshold = 1,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 2,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 20,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 1,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 1,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 0, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 0, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 0 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    },
    {
        .cntThreshold = 1,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 2,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 20,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 0,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 1,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 1, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 5000, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 1200 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    }
}; /*!< iotouch基线追踪算法参数 */
/** @brief 基线追踪器实例（按键组1） */
static T_SiBaselineDoorctrl2MtStrongAvg iotouchBaseline1;
/** @brief 基线追踪器临时数据缓冲区（按键组1） */
static T_SiBaselineDataDoorctrl2MtStrongAvg iotouchBaselineTempData1[IOTOUCH_KEY1NUM];
/** @brief 基线值缓冲区（按键组1） */
static T_SiData iotouchBaselineBuf1[IOTOUCH_KEY1NUM];

/**
 * @brief 判决器配置参数（按键组1，双通道跳频）
 * @note 含检测通道、跳频通道、合并通道参数
 */
const static T_SiArbiterParaFreq10Doorctrl iotouchArbiterPara1 =
{
    .detectChannelNo = 0,                         /*!< 检测通道编号--需要用户指定 */
    .freqChannelNo = 1,                           /*!< 跳频通道编号--需要用户指定 */
    .pressEliminate = 0,                          /*!< 按压消抖次数--需要用户指定 */
    .releaseEliminate = 1,                        /*!< 释放消抖次数--需要用户指定 */
    .detectChannel =
    {
        .pressThreshold1 = 60,                    /*!< 1阶按压阈值--需要用户指定 */
        .pressThreshold2 = 200,//30,//40,         /*!< 2阶按压阈值--需要用户指定 */
        .forceValidThreshold = 580,
        .forceValidCalcTimeout = 30,
        .doubleForceValidTimeMs = 2500,
        .releaseThreshold = 9,                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        .threshold12TimeLow = 1,                  /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        .threshold12TimeHigh = 50,                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        .threshold2KeepTime = 50,                 /*!< 2阶按压检测保持时间，在这段时间内检测到符合2阶按压条件，即认为按压有效--需要用户指定 */

        .maxDiffLowThreshold = 200,//30,//45,                /*!< 最大diff阈值下边界--需要用户指定 */
        .maxDiffHighThreshold = 1500           /*!< 最大diff阈值上边界--需要用户指定 */
    },      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.满足最大阈值范围，都满足后才认为按压有效 */
    .freqChannel =
    {
        .lockBaselineThreshold = -60,
        .releaseThreshold = 5,
        .alwaysUpdataUnlockValue = 0,
        .unlockBaselineReInit = 0,
        .unlockBaselineSampNum = 300,
        .unlockBaselineThreshold = 300,
        .unlockBaselineEliminateMs = 20000,
        .unlockBaselineLockKeyMs = 120,
        .forceReleaseBaselineTimeMs = 7 * 60 * 1000UL,

        .quickUnlockBaseline =
        {
            .enable = 1,
            .preDetectMs = 100,
            .postDetectMs = 400,
            .postSampNum = 20,
            .postSampThreshold = 300
        },

        .lowThreshold = 50,                       /*!< 2阶信号稳定时跳频通道的diff低阈值--需要用户指定 */
        .highThreshold = 1000,                     /*!< 2阶信号稳定时跳频通道的diff高阈值--需要用户指定 */
    },                                            /*!< 跳频通道参数--需要用户指定 */
    .mergeChannel =
    {
        .lowThreshold = -400,
        .highThreshold = 1200,
        .forceValidLowThreshold = -700,
        .forceValidHighThreshold = 1200
    },
    .releaseLockBaselineMs = 500,                 /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    .threshold1ForceReleaseTimeS = 2,             /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    .threshold2ForceReleaseTimeS = 5              /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
}; /*!< iotouch判决器算法参数 */
/** @brief 判决器实例（按键组1） */
static T_SiArbiterFreq10Doorctrl iotouchArbiter1;

/** @brief 按键逻辑通道映射表（按键组1） */
const static uint8_t iotouchKeyMap1[IOTOUCH_KEY1NUM] = {1, 0};

/** @brief IOTouch触摸按键对象（按键组1） */
static T_SiObject iotouchObjectKey1;
/** @brief 原始值缓冲区（按键组1） */
static T_SiData iotouchKeyRawDataBuf1[IOTOUCH_KEY1NUM];

/**
 * @brief IOTouch按键状态改变回调函数（按键组1）
 * @param[in] keyNo    按键编号
 * @param[in] status   按键状态
 */
static void IOtouchKeyValueChanged1(uint8_t keyNo, T_SiKeyStatus status);

/**
 * @brief IOTouch判决器钩子函数（按键组1）
 * @param[in] obj  IOTouch对象指针
 */
static void IOtouchKeyArbiterHook1(T_SiObject *obj);

#if LOW_POWER_EN
/** @brief 基线参数调节器复合节点（按键组1）：低功耗模式下切换参数 */
static T_SiParaAdjusterComposite iotouchBaselineParaAdjComp1 = {0};
/** @brief 基线参数调节器叶子节点（按键组1）：常规模式参数 */
static T_SiParaAdjusterLeaf iotouchBaselineParaAdjLeafNormal1 = {0};
/** @brief 基线参数调节器叶子节点（按键组1）：低功耗模式参数 */
static T_SiParaAdjusterLeaf iotouchBaselineParaAdjLeafLp1 = {0};
#endif

#endif

#if IOTOUCH_KEY2NUM
/** @brief 门把手滤波器配置参数（按键组2） */
const static T_SiFilterParaExtDoorctrl iotouchFilterParaDoorctrl2 =
{
    .gainers = {
        {
            .enable = 1,                    /*!< 增益器使能，1使能，0关闭--需要用户指定 */
            .offset = 6000                   /*!< 信号偏置值，原始数据会减去信号偏置值后再放入增益器，--需要用户指定 */
        },
        {
            .enable = 0,                    /*!< 增益器使能，1使能，0关闭--需要用户指定 */
            .offset = 10000                   /*!< 信号偏置值，原始数据会减去信号偏置值后再放入增益器，--需要用户指定 */
        },
    },
    .kfr = 4000,                            /*!< 跳频通道方差值，单位0.01--需要用户指定 */
};
/** @brief 门把手滤波器实例（按键组2） */
static T_SiFilterExtDoorctrl iotouchFilterDoorctrl2;
/** @brief 滤波值缓冲区（按键组2） */
static T_SiData iotouchFilterBufDoorctrl2[IOTOUCH_KEY2NUM];

/**
 * @brief 基线追踪器配置参数（按键组2，常规模式）
 * @note 使用双通道强平均基线追踪算法
 */
const static T_SiBaselineParaDoorctrl2MtStrongAvg iotouchBaselinePara2 =
{
    {
        .cntThreshold = 10,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 6,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 1,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 1,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 20,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 0, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 0, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 0 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    },
    {
        .cntThreshold = 10,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 6,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 1,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 0,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 20,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 1, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 5000, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 1200 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    }
}; /*!< iotouch基线追踪算法参数 */
/**
 * @brief 基线追踪器配置参数（按键组2，低功耗模式）
 * @note 快速步进参数，用于低功耗模式下快速追踪基线变化
 */
const static T_SiBaselineParaDoorctrl2MtStrongAvg iotouchBaselinePara22 =
{
    {
        .cntThreshold = 1,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 2,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 20,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 1,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 1,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 0, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 0, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 0 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    },
    {
        .cntThreshold = 1,              /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 2,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 20,                        /*!< 基线追踪器步--需要用户指定 */
        .quickDrop =
        {
            .enable = 0,                  /*!< quickDrop开关--需要用户指定 */
            .quickDropHoldCnt = 1,       /*!< 当满足quickDrop阈值一定次数后，启用quickDrop功能，建议不低于500ms--需要用户指定 */
            .quickDropThreshold = 20,      /*!< quickDrop阈值，当基线-采样值超过本阈值时（不是绝对值，是单方向值），会快速更新基线--需要用户指定 */
            .quickDropOff =
            {
                .forceEnableOnBoot = 1, /*!< quickDrop关闭时，上电启动时候会短暂使能quickdrop一段时间--需要用户指定 */
                .forceEnableTimeoutMs = 5000, /*!< forceEnableOnBoot在上电启动时使能的时间，单位ms--需要用户指定 */
                .safeThreshold = 1200 /*!< 当基线偏差绝对值超过本阈值时，会立刻更新基线，防止基线长时间锁死--需要用户指定 */
            }
        }
    }
}; /*!< iotouch基线追踪算法参数 */
/** @brief 基线追踪器实例（按键组2） */
static T_SiBaselineDoorctrl2MtStrongAvg iotouchBaseline2;
/** @brief 基线追踪器临时数据缓冲区（按键组2） */
static T_SiBaselineDataDoorctrl2MtStrongAvg iotouchBaselineTempData2[IOTOUCH_KEY2NUM];
/** @brief 基线值缓冲区（按键组2） */
static T_SiData iotouchBaselineBuf2[IOTOUCH_KEY2NUM];

/**
 * @brief 判决器配置参数（按键组2，双通道跳频）
 * @note 含检测通道、跳频通道、合并通道参数
 */
const static T_SiArbiterParaFreq10Doorctrl iotouchArbiterPara2 =
{
    .detectChannelNo = 0,                         /*!< 检测通道编号--需要用户指定 */
    .freqChannelNo = 1,                           /*!< 跳频通道编号--需要用户指定 */
    .pressEliminate = 0,                          /*!< 按压消抖次数--需要用户指定 */
    .releaseEliminate = 1,                        /*!< 释放消抖次数--需要用户指定 */
    .detectChannel =
    {
        .pressThreshold1 = 60,                    /*!< 1阶按压阈值--需要用户指定 */
        .pressThreshold2 = 150,//30,//40,         /*!< 2阶按压阈值--需要用户指定 */
        .forceValidThreshold = 550,
        .forceValidCalcTimeout = 30,
        .doubleForceValidTimeMs = 2500,
        .releaseThreshold = 9,                    /*!< 释放阈值，范围3-9，表示：1阶按压阈值的0.3-0.9--需要用户指定 */

        .threshold12TimeLow = 1,                  /*!< 从1阶按压阈值到2阶按压阈值的最短时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        .threshold12TimeHigh = 50,                /*!< 从1阶按压阈值到2阶按压阈值的最长时间，单位是采样个数，1到2阶时间范围在[threshold12TimeLow,threshold12TimeHigh]有效--需要用户指定 */
        .threshold2KeepTime = 50,                 /*!< 2阶按压检测保持时间，在这段时间内检测到符合2阶按压条件，即认为按压有效--需要用户指定 */

        .maxDiffLowThreshold = 150,//30,//45,                /*!< 最大diff阈值下边界--需要用户指定 */
        .maxDiffHighThreshold = 1500           /*!< 最大diff阈值上边界--需要用户指定 */
    },      /*!< 按压检测，1.按压信号达到一二阶阈值，2.从一阶到二阶时间满足阈值范围，3.满足最大阈值范围，都满足后才认为按压有效 */
    .freqChannel =
    {
        .lockBaselineThreshold = -60,
        .releaseThreshold = 5,
        .alwaysUpdataUnlockValue = 0,
        .unlockBaselineReInit = 0,
        .unlockBaselineSampNum = 300,
        .unlockBaselineThreshold = 300,
        .unlockBaselineEliminateMs = 20000,
        .unlockBaselineLockKeyMs = 120,
        .forceReleaseBaselineTimeMs = 7 * 60 * 1000UL,

        .quickUnlockBaseline =
        {
            .enable = 1,
            .preDetectMs = 100,
            .postDetectMs = 400,
            .postSampNum = 20,
            .postSampThreshold = 300
        },

        .lowThreshold = 50,                       /*!< 2阶信号稳定时跳频通道的diff低阈值--需要用户指定 */
        .highThreshold = 1000,                     /*!< 2阶信号稳定时跳频通道的diff高阈值--需要用户指定 */
    },                                            /*!< 跳频通道参数--需要用户指定 */
    .mergeChannel =
    {
        .lowThreshold = -400,
        .highThreshold = 1200,
        .forceValidLowThreshold = -700,
        .forceValidHighThreshold = 1200
    },
    .releaseLockBaselineMs = 500,                 /*!< 释放后锁定baseline一段时间不更新，单位Ms--需要用户指定 */
    .threshold1ForceReleaseTimeS = 2,             /*!< 阈值1强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
    .threshold2ForceReleaseTimeS = 5              /*!< 阈值2强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
}; /*!< iotouch判决器算法参数 */
/** @brief 判决器实例（按键组2） */
static T_SiArbiterFreq10Doorctrl iotouchArbiter2;

/** @brief 按键逻辑通道映射表（按键组2） */
const static uint8_t iotouchKeyMap2[IOTOUCH_KEY2NUM] = {3, 2};

/** @brief IOTouch触摸按键对象（按键组2） */
static T_SiObject iotouchObjectKey2;
/** @brief 原始值缓冲区（按键组2） */
static T_SiData iotouchKeyRawDataBuf2[IOTOUCH_KEY2NUM];

/**
 * @brief IOTouch按键状态改变回调函数（按键组2）
 * @param[in] keyNo    按键编号
 * @param[in] status   按键状态
 */
static void IOtouchKeyValueChanged2(uint8_t keyNo, T_SiKeyStatus status);

/**
 * @brief IOTouch判决器钩子函数（按键组2）
 * @param[in] obj  IOTouch对象指针
 */
static void IOtouchKeyArbiterHook2(T_SiObject *obj);

#if LOW_POWER_EN
/** @brief 基线参数调节器复合节点（按键组2）：低功耗模式下切换参数 */
static T_SiParaAdjusterComposite iotouchBaselineParaAdjComp2 = {0};
/** @brief 基线参数调节器叶子节点（按键组2）：常规模式参数 */
static T_SiParaAdjusterLeaf iotouchBaselineParaAdjLeafNormal2 = {0};
/** @brief 基线参数调节器叶子节点（按键组2）：低功耗模式参数 */
static T_SiParaAdjusterLeaf iotouchBaselineParaAdjLeafLp2 = {0};
#endif

#endif

#if LOW_POWER_EN

/** @brief IOTouch低功耗滤波器（无滤波模式） */
static T_SiFilterNone iotouchLpFilter;
/** @brief IOTouch低功耗滤波值缓冲区 */
static T_SiData iotouchLpFilterBuf[IOTOUCH_LP_KEYNUM];

/**
 * @brief 低功耗基线追踪器配置参数
 * @note 使用快速唤醒基线追踪算法（quickdrop）
 */
const static T_SiBaselineParaKeyWkupQuickdrop iotouchLpBaselinePara =
{
    .firstSkipDataCount = 0,
    .firstCntThreshold = 1,
    .quickDropHoldCnt = 6, /**< 建议不低于500ms */
    .quickDropThreshold = 20,
    .cntThreshold = 10,
    .step = 5
}; /*!< iotouch基线追踪算法参数 */
/** @brief 低功耗基线追踪器实例 */
static T_SiBaselineKeyWkupQuickdrop iotouchLpBaseline;
/** @brief 低功耗基线追踪器临时数据缓冲区 */
static T_SiBaselineDataKeyWkupQuickdrop iotouchLpBaselineTempData[IOTOUCH_LP_KEYNUM];
/** @brief 低功耗基线值缓冲区 */
static T_SiData iotouchLpBaselineBuf[IOTOUCH_LP_KEYNUM];

/**
 * @brief 低功耗判决器配置参数
 * @note 使用绝对值唤醒判决算法
 */
const static T_SiArbiterParaKeyMtAbsWkup iotouchLpArbiterPara =
{
    .wkupThresholds = {30, 30}
}; /*!< iotouch判决器算法参数 */
/** @brief 低功耗判决器实例 */
static T_SiArbiterKeyMtAbsWkup iotouchLpArbiter;

/** @brief 低功耗按键逻辑通道映射表 */
const static uint8_t iotouchLpKeyMap[IOTOUCH_LP_KEYNUM] = {0, 2};

/** @brief 低功耗IOTouch触摸按键对象 */
static T_SiObject iotouchLpObjectKey;
/** @brief 低功耗原始值缓冲区 */
static T_SiData iotouchLpKeyRawDataBuf[IOTOUCH_LP_KEYNUM];

/**
 * @brief IOTouch低功耗判决器钩子函数
 * @param[in] obj  IOTouch对象指针
 */
static void IOtouchLpKeyArbiterHook(T_SiObject *obj);

#endif

//**************************************************************************************************
//***噪音检测器配置

#if NOISE_DETECT_VARIANCE_KEYNUM
/** @brief 噪音检测退出条件配置参数（快速方差检测） */
const static T_SiNoiseExitConditionParaFastVariance noiseExitConditionVariancePara =
{
    .channelNo = 2,                                /*!< 逻辑通道编号，--需要用户指定 */
    .windowLen = 80,                                /*!< 窗口长度，--需要用户指定 */
    .eliminateTimeMs = 2000,                         /*!< 消抖时间，持续一段时间低于阈值，认为可以退出--需要用户指定 */
    .threshold = 250                          /*!< 噪音检测阈值--需要用户指定 */
};
/** @brief 噪音检测退出条件实例 */
static T_SiNoiseExitConditionFastVariance noiseExitConditionVariance = {0};

/**
 * @brief 噪音检测算法参数（轻量级差分检测）
 */
const static T_SiNoiseParaLiteDiff noiseDetectVariancePara =
{
    .thresholdSign = 0,                   /*!< 阈值的符号，取-1，0，1，当为0时取diff的绝对值，否则diff值乘以thresholdSign后再与detectThreshold比较--需要用户指定 */
    .baseline =
    {
        .cntThreshold = 10,               /*!< 采样计数器阈值，到达后更新基线--需要用户指定 */
        .firstCntThreshold = 6,          /*!< 首次采样计数器阈值，到达后更新基线--需要用户指定 */
        .step = 1                       /*!< 基线追踪器步进--需要用户指定 */
    },
    .arbiter =
    {
        .detectEliminate = 0,           /*!< 检测消抖次数--需要用户指定 */
        .detectThreshold = 50,      /*!< 噪音检测阈值--需要用户指定 */
        .releaseThreshold = 7,           /*!< 释放阈值，范围3-9，表示：检测阈值的0.3-0.9--需要用户指定 */
        .releaseDelayS = 2,              /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位S--需要用户指定 */
        .releaseDelayDeciS = 0          /*!< 当噪音检测小于阈值后，延迟1段时间再清噪音检测标志，单位0.1S--需要用户指定 */
    },
    .forceReleaseTimeS = 2             /*!< 强制释放时间，单位为秒，设为0时表示永不释放--需要用户指定 */
};

/** @brief 噪音检测算法实例 */
static T_SiNoiseLiteDiff noiseDetectVariance = {0};
/** @brief 噪音检测临时数据缓冲区 */
static T_SiNoiseDataLiteDiff noiseDetectVarianceTempData[NOISE_DETECT_VARIANCE_KEYNUM] = {0};
/** @brief 噪音检测数据缓冲区 */
static T_SiNoiseData noiseDetectVarianceBuf[NOISE_DETECT_VARIANCE_KEYNUM] = {0};
/** @brief 噪音检测逻辑通道映射表 */
const static uint8_t noiseDetectVarianceKeyMap[NOISE_DETECT_VARIANCE_KEYNUM] = {4};
/** @brief 噪音检测对象 */
static T_SiNoiseObject noiseDetectVarianceObject = {0};
/** @brief 噪音检测原始值缓冲区 */
static T_SiData noiseDetectVarianceRawDataBuf[NOISE_DETECT_VARIANCE_KEYNUM] = {0};

/**
 * @brief 噪声检测器钩子函数
 * @param[in] obj       噪音检测对象指针
 * @param[in] keyNum    按键编号
 * @param[in] noiseBuf  噪音数据缓冲区
 */
static void NoiseDetectVarianceHook(T_SiNoiseObject *obj, uint8_t keyNum, const T_SiNoiseData *noiseBuf);

/**
 * @brief 噪音检测状态变化回调函数
 * @param[in] status 噪音状态，非0表示检测到噪音，0表示噪音消失
 */
static void DoorCtrlNoiseDetectVarianceStatusChanged(uint8_t status);
/** @brief 噪音退出条件数组 */
static T_SiNoiseExitConditionBase *noiseExitConditions[] = {(T_SiNoiseExitConditionBase *) &noiseExitConditionVariance};
/** @brief 受噪音影响的信号集对象数组 */
static T_SiObject *noiseSiObjs[] =
{
#if IOTOUCH_KEY1NUM
    &iotouchObjectKey1,
#endif
#if IOTOUCH_KEY2NUM
    &iotouchObjectKey2,
#endif
};
/**
 * @brief 噪音检测器描述符
 * @note 定义噪音检测的匹配类型、动作、退出条件等完整配置
 */
static T_SiNoiseDetect doorctrlNoiseDetectVariance =
{
    .matchType = SI_NOISE_DETECT_MATCH_ANY,             /*!< 匹配类型，所有或任意--需要用户指定 */
    .mask = (0x1 << NOISE_DETECT_VARIANCE_KEYNUM) - 1,  /*!< 待检测噪声通道掩码--需要用户指定 */
    .action = SI_NOISE_ACTION_LOCK,                         /*!< 检测到噪音时执行的动作类型--需要用户指定 */
    .siObjIsArray = 1,                           /*!< 信号集对象是否是数组，0表示不是，1表示是数组--需要用户指定 */
    .siObjArrayNum = sizeof(noiseSiObjs) / sizeof(noiseSiObjs[0]), /*!< 信号集对象是数组时，数组长度--需要用户指定 */
    .siObjs = noiseSiObjs,                              /*!< 信号集对象数组--需要用户指定 */
    .statusChanged = DoorCtrlNoiseDetectVarianceStatusChanged,          /*!< 噪音状态发生改变，status非0表示检测到噪音，0表示噪音消失，可以为NULL--需要用户指定 */
    .statusChanged2 = NULL,         /*!< 噪音状态发生改变，status非0表示检测到噪音，0表示噪音消失，可以为NULL--用户可选配置 */
    .hook = NULL,   /*!< 回调接口，可以为NULL--用户可选配置 */

    .exitResetSiObject = 1,                      /*!< 退出时Reset信号集描述符--需要用户指定 */

    .exitConditionNum = sizeof(noiseExitConditions) / sizeof(noiseExitConditions[0]), /*!< 退出条件个数，为0表示立即退出--需要用户指定 */
    .exitConditions = noiseExitConditions,                      /*!< 退出条件--需要用户指定 */
    .exitMatchType = SI_NOISE_EXIT_MATCH_ALL,    /*!< 退出条件匹配类型--需要用户指定 */
    .exitTimeoutMs = 1800000                           /*!< 退出条件强制超时时间，0表示永不强制超时--需要用户指定 */
};
#endif

//**************************************************************************************************

/**
 * @brief 配置触摸功能和算法参数
 *
 * 本函数完成触摸算法的完整初始化流程，包括：
 * - 硬件时序参数配置（touchcfg, adccfg）
 * - 触摸通道配置（touchChannels）
 * - 增益器配置（touchGainers, touchSleepGainers）
 * - 扫描调度器创建
 * - 噪音检测器初始化与注册
 * - IOTouch按键组（KEY1/KEY2）的滤波器、基线追踪器、判决器初始化与注册
 * - 低功耗模式下的参数调节器配置
 *
 * @param[in] algo  算法对象指针，由上层传入的算法管理对象
 * @retval 无返回值（void）
 *
 * @note 函数内部包含多个条件编译块（#if），实际初始化的模块取决于编译宏配置。
 *       初始化失败时会通过 TC_LOGE 输出错误日志，但不会 halt 系统。
 */
void TouchConfig(T_SiAlgoObject *algo)
{
    uint8_t keyNum;
    T_SiErrRt rt;

    /** @brief 触摸硬件通用配置 */
    static const TOUCH_HalConfig_Type touchcfg =
    {
        .init_time = 0xFF,                 /**< 初始时间，0x0-0xFFF，对低功耗影响大 */
        .hop_period = 7,                   /**< 跳频，0-7 */
        .clock_divider = TOUCH_CLK_DIV,    /**< 时钟分频 */
    };
    /** @brief ADC 配置参数 */
    static const TOUCH_HalAdcConfig_Type adccfg =
    {
        .clock_divider = TOUCH_ADC_DIV, /**< 时钟分频 */
        .vcm_sel = ADC_VCM_SEL_NULL,   /**< VCM，共模输入电压 */
        .i_sel = ADC_IBIAS_0p5x,       /**< ibias */
        .vref_sel = ADC_VREF_1500,     /**< Vref参考电压选择 */
        .samp_cycle = 0x7,              /**< 采样周期 */
        .init_cycle = 0x0               /**< init周期 */
    };
    /** @brief 触摸电荷传输节点配置，定义充放电参数 */
    static TOUCH_HalCharge_Type touchNode;
    /** @brief 触摸通道配置表 */
    static const TOUCH_HalChConfig_Type touchChannels[] =
    {
        {
            .channel = CAPTOUCH_CHANNEL_3,      /**< 通道编号，CHANNEL_0 - CHANNEL_4 (UnLock) */
            .cref_sel = CHARGE_DISCHARGE_REFERENCE_CAP,     /**< 电荷转移方向选择，0表示cref->pad，1相反 */
            .captouch_mode = CHARGE_DISCHARGE_BALANCE_MODE,  /**< 充放电模式 */
            .shield = {
                .shld_en = 0,              /**< 防护开关 */
                .shld_sel = SOURCE_FOLLOW_SHIELD,            /**< 防护通道选择 */
                .shld_i = 0x3                                /**< 防护电流，0-7 */
            },                                               /**< 防护通道配置 */
            .compensate = {
                .cmp_en = 1,                                 /**< 补偿总开关 */
                .idac_inp = 0xFF,                            /**< 充电补偿电流，0x0-0xFF */
                .idac_p_en = 1,                              /**< 充电补偿开关 */
                .idac_inn = 0xFF,                            /**< 放电补偿电流，0x0-0xFF */
                .idac_n_en = 1,                              /**< 放电补偿开关 */
                .idac_time = 0x8                             /**< 补偿时间，0x0-0xFF */
            }                                                /**< 寄生电容补偿：补偿电容 = 补偿时间 * 补偿电流 */
        },
        {
            .channel = CAPTOUCH_CHANNEL_3,      /**< 通道编号，CHANNEL_0 - CHANNEL_4 (UnLock) */
            .cref_sel = CHARGE_DISCHARGE_REFERENCE_CAP,     /**< 电荷转移方向选择 */
            .captouch_mode = CHARGE_DISCHARGE_BALANCE_MODE,  /**< 充放电模式 */
            .shield = {
                .shld_en = 0,              /**< 防护开关 */
                .shld_sel = SOURCE_FOLLOW_SHIELD,            /**< 防护通道选择 */
                .shld_i = 0x3                                /**< 防护电流，0-7 */
            },                                               /**< 防护通道配置 */
            .compensate = {
                .cmp_en = 1,                                 /**< 补偿总开关 */
                .idac_inp = 0xFF,                            /**< 充电补偿电流 */
                .idac_p_en = 1,                              /**< 充电补偿开关 */
                .idac_inn = 0xFF,                            /**< 放电补偿电流 */
                .idac_n_en = 1,                              /**< 放电补偿开关 */
                .idac_time = 0x8                             /**< 补偿时间 */
            }                                                /**< 寄生电容补偿 */
        },
        {
            .channel = CAPTOUCH_CHANNEL_2,      /**< 通道编号，CHANNEL_0 - CHANNEL_4 (Lock) */
            .cref_sel = CHARGE_DISCHARGE_REFERENCE_CAP,     /**< 电荷转移方向选择 */
            .captouch_mode = CHARGE_DISCHARGE_BALANCE_MODE,  /**< 充放电模式 */
            .shield = {
                .shld_en = 0,              /**< 防护开关 */
                .shld_sel = SOURCE_FOLLOW_SHIELD,            /**< 防护通道选择 */
                .shld_i = 0x3                                /**< 防护电流，0-7 */
            },                                               /**< 防护通道配置 */
            .compensate = {
                .cmp_en = 1,                                 /**< 补偿总开关 */
                .idac_inp = 0xFF,                            /**< 充电补偿电流 */
                .idac_p_en = 1,                              /**< 充电补偿开关 */
                .idac_inn = 0xFF,                            /**< 放电补偿电流 */
                .idac_n_en = 1,                              /**< 放电补偿开关 */
                .idac_time = 0x8                             /**< 补偿时间 */
            }                                                /**< 寄生电容补偿 */
        },
        {
            .channel = CAPTOUCH_CHANNEL_2,      /**< 通道编号，CHANNEL_0 - CHANNEL_4 (Lock) */
            .cref_sel = CHARGE_DISCHARGE_REFERENCE_CAP,     /**< 电荷转移方向选择 */
            .captouch_mode = CHARGE_DISCHARGE_BALANCE_MODE,  /**< 充放电模式 */
            .shield = {
                .shld_en = 0,              /**< 防护开关 */
                .shld_sel = SOURCE_FOLLOW_SHIELD,            /**< 防护通道选择 */
                .shld_i = 0x3                                /**< 防护电流，0-7 */
            },                                               /**< 防护通道配置 */
            .compensate = {
                .cmp_en = 1,                                 /**< 补偿总开关 */
                .idac_inp = 0xFF,                            /**< 充电补偿电流 */
                .idac_p_en = 1,                              /**< 充电补偿开关 */
                .idac_inn = 0xFF,                            /**< 放电补偿电流 */
                .idac_n_en = 1,                              /**< 放电补偿开关 */
                .idac_time = 0x8                             /**< 补偿时间 */
            }                                                /**< 寄生电容补偿 */
        },
        {
            .channel = CAPTOUCH_CHANNEL_4,                   /**< 通道编号，CHANNEL_0 - CHANNEL_4 */
            .cref_sel = CHARGE_DISCHARGE_REFERENCE_CAP,     /**< 电荷转移方向选择 */
            .captouch_mode = CHARGE_MODE,  /**< 充放电模式 */
            .shield = {
                .shld_en = 0,                                /**< 防护开关 */
                .shld_sel = SOURCE_FOLLOW_SHIELD,            /**< 防护通道选择 */
                .shld_i = 0x7                                /**< 防护电流，0-7 */
            },                                               /**< 防护通道配置 */
            .compensate = {
                .cmp_en = 0,                                 /**< 补偿总开关 */
                .idac_inp = 0x7,                             /**< 充电补偿电流 */
                .idac_p_en = 0,                              /**< 充电补偿开关 */
                .idac_inn = 0x7,                             /**< 放电补偿电流 */
                .idac_n_en = 0,                              /**< 放电补偿开关 */
                .idac_time = 0x15                            /**< 补偿时间 */
            }                                                /**< 寄生电容补偿 */
        }
    };
    /** @brief 增益器配置表（正常工作模式） */
    static const TOUCH_HalGainer_Type touchGainers[] =
    {
        {
            .tran_loop = 255,       /**< 电荷转移次数 */
            .tran_time = 0x3F,      /**< 电荷转移时间，0x0-0xFF，对低功耗影响大 */
            .chg_time = 0x3F,       /**< 充放电时间，0x0-0xFF，对低功耗影响大 */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数，0-15 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .tran_loop = 255,       /**< 电荷转移次数 */
            .tran_time = 0x6,       /**< 电荷转移时间，0x0-0xFF */
            .chg_time = 0x6,        /**< 充放电时间，0x0-0xFF */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .tran_loop = 255,       /**< 电荷转移次数 */
            .tran_time = 0x3F,      /**< 电荷转移时间，0x0-0xFF */
            .chg_time = 0x3F,       /**< 充放电时间，0x0-0xFF */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .tran_loop = 255,       /**< 电荷转移次数 */
            .tran_time = 0x6,       /**< 电荷转移时间，0x0-0xFF */
            .chg_time = 0x6,        /**< 充放电时间，0x0-0xFF */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .tran_loop = 196,       /**< 电荷转移次数 */
            .tran_time = 0x17,      /**< 电荷转移时间，0x0-0xFF */
            .chg_time = 0x17,       /**< 充放电时间，0x0-0xFF */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            }
        }
    };
    /** @brief 增益器配置表（休眠模式） */
    static const TOUCH_HalGainer_Type touchSleepGainers[] =
    {
        {
            .init_time = 0x3F,
            .tran_loop = 31,        /**< 电荷转移次数 */
            .tran_time = 0x1F,      /**< 电荷转移时间，0x0-0xFF，对低功耗影响大 */
            .chg_time = 0x1F,       /**< 充放电时间，0x0-0xFF，对低功耗影响大 */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数，0-15 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .init_time = 0x3F,
            .tran_loop = 31,        /**< 电荷转移次数 */
            .tran_time = 0x1F,      /**< 电荷转移时间 */
            .chg_time = 0x1F,       /**< 充放电时间 */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .init_time = 0x3F,
            .tran_loop = 31,        /**< 电荷转移次数 */
            .tran_time = 0x1F,      /**< 电荷转移时间 */
            .chg_time = 0x1F,       /**< 充放电时间 */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .init_time = 0x3F,
            .tran_loop = 31,        /**< 电荷转移次数 */
            .tran_time = 0x1F,      /**< 电荷转移时间 */
            .chg_time = 0x1F,       /**< 充放电时间 */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            },
        },
        {
            .init_time = 0x3F,
            .tran_loop = 15,        /**< 电荷转移次数 */
            .tran_time = 0xF,       /**< 电荷转移时间 */
            .chg_time = 0xF,        /**< 充放电时间 */
            .pga =
            {
                .enable = 0,        /**< PGA使能 */
                .pga_gain_sel = ADC_GAIN_X1, /**< PGA运放倍数 */
                .buf_en = 0,        /**< Buffer开关 */
                .vcr_en = 0,        /**< VCR开关 */
                .vcr_sel = ADC_VCR_SEL_236_7 /**< VCR电压选择 */
            }
        }
    };
//    static const uint8_t removeResiduesEnableTable[] =
//    {
//        0x1,
//        0x0,
//        0x0,
//        0x0
//    };
    /** @brief 低通滤波器配置表 */
    static TOUCH_HalLowPassFilter_Type lowPassTable[] =
    {
        {.gain = 1},
        {.gain = 1},
        {.gain = 1},
        {.gain = 1},
        {.gain = 0},
    };
    /** @brief 双边采样配置表 */
    static TOUCH_HalDoubleSamp_Type doubleSampTable[] =
    {
        {.enable = 1},
        {.enable = 1},
        {.enable = 1},
        {.enable = 1},
        {.enable = 1}
    };
    /** @brief 噪音规避配置表（正常工作模式） */
    static TOUCH_HalNoiseAvoid_Type noiseAvoidTable[] =
    {
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 0,                      /**< 粗调 */
            .fineTune = 0,                         /**< 微调 */
            .fineStep = 0                          /**< 微调步进 */
        }
    };
    /** @brief 噪音规避配置表（休眠模式） */
    static TOUCH_HalNoiseAvoid_Type sleepNoiseAvoidTable[] =
    {
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 10,                      /**< 粗调 */
            .fineTune = 1,                         /**< 微调 */
            .fineStep = 1                          /**< 微调步进，建议为1 */
        },
        {
            .coarseTune = 0,                      /**< 粗调 */
            .fineTune = 0,                         /**< 微调 */
            .fineStep = 0                          /**< 微调步进 */
        }
    };

    /** @brief 触摸调度描述符，仅触摸模式 */
    static TOUCH_HalDispatch_OnlyTouch_Type dispatch;
    /** @brief 扫描调度器参数配置 */
    static const TOUCH_HalDispatch_OnlyTouchPara_Type dispatchPara =
    {
        .fast_freq = TOUCH_FAST_FREQ,
        .slow_freq = TOUCH_SLOW_FREQ,
        .sleep_freq = TOUCH_SLEEP_FREQ,
        .sleep_mask_freq = TOUCH_SLEEP_MASK_FREQ,
        .skip_poweron_datas = TOUCH_SKIP_STARTUP_DATAS,
        .fast_remove_residues_enable = TRUE,
        .scan_type = TOUCH_SCAN_MANY,     /**< 扫描类型 */
        .scan_many_num = 2,               /**< 每次扫描通道数（TOUCH_SCAN_MANY模式） */
    };
    /** @brief 触摸硬件接口指针，由 Touch_HalChargeCreate 创建 */
    TOUCH_HalInterface_Type *touchInterface;
    /** @brief 扫描调度器配置表 */
    TOUCH_HalScanerTable_Type dispatchScaner;

    memset(&dispatchScaner, 0x0, sizeof(dispatchScaner));
    dispatchScaner.scaner_len = 1;                           /**< 扫描器个数 */
    touchInterface = Touch_HalChargeCreate(&touchNode, &touchcfg, &adccfg);   /**< 创建触摸硬件接口，绑定电荷传输节点与ADC配置 */
    dispatchScaner.scaners[0].touch_node = touchInterface;                    /**< 触摸硬件接口 */
    dispatchScaner.scaners[0].channel_len = sizeof(touchChannels) / sizeof(touchChannels[0]); /**< 通道数量 */
    dispatchScaner.scaners[0].channel_table = touchChannels;                 /**< 通道配置表 */
    dispatchScaner.scaners[0].gainer_table = touchGainers;                  /**< 增益器配置表 */
    dispatchScaner.scaners[0].sleep_gainer_table = touchSleepGainers;       /**< 休眠增益器配置表 */
    dispatchScaner.scaners[0].noise_avoid_table = noiseAvoidTable;          /**< 噪音规避表 */
    dispatchScaner.scaners[0].sleep_noise_avoid_table = sleepNoiseAvoidTable; /**< 休眠噪音规避表 */
//    dispatchScaner.scaners[0].remove_residues_enable_table = removeResiduesEnableTable;
    dispatchScaner.scaners[0].iodisable_channelmask = (0x1 << sizeof(touchChannels) / sizeof(touchChannels[0])) - 1;  /**< IO禁用通道掩码 */
    dispatchScaner.scaners[0].sleep_iodisable_channelmask = (0x1 << sizeof(touchChannels) / sizeof(touchChannels[0])) - 1;  /**< 休眠IO禁用通道掩码 */
    dispatchScaner.scaners[0].low_pass_table = lowPassTable;                /**< 低通滤波器表 */
    dispatchScaner.scaners[0].double_samp_table = doubleSampTable;          /**< 双边采样表 */
    dispatchScaner.scaners[0].sleep_channelmask = 0x2 | 0x8 | 0x10;        /**< 休眠通道掩码 */

    touchDispatch = Touch_HalDispatchOnlyTouchCreate(&dispatch, &dispatchScaner, &dispatchPara); /**< 创建仅触摸调度器，注入扫描器配置与调度参数 */

    //**********************************************************************************************
    //噪音检测器配置
#if NOISE_DETECT_VARIANCE_KEYNUM
    if ((rt = SiNoiseExitConditionFastVarianceInit(&noiseExitConditionVariance, &noiseExitConditionVariancePara)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "Noise Exit Diff init fail:%s", SiErrRtDesp(rt));
    }

    if ((rt = SiNoiseLiteDiffNodeInit(&noiseDetectVariance, &noiseDetectVariancePara, NOISE_DETECT_VARIANCE_KEYNUM, noiseDetectVarianceTempData, NOISE_DETECT_VARIANCE_KEYNUM, noiseDetectVarianceBuf)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "Noise detect Variance init fail:%s", SiErrRtDesp(rt));
    }
    SiNoiseRegisterHook(&noiseDetectVariance, NoiseDetectVarianceHook);
    //噪音检测对象初始化
    if ((rt = SiNoiseObjectNodeInit(&noiseDetectVarianceObject, NOISE_DETECT_VARIANCE_KEYNUM, noiseDetectVarianceKeyMap, noiseDetectVarianceRawDataBuf, &noiseDetectVariance)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "Noise Object Variance init fail:%s", SiErrRtDesp(rt));
    }
    //注册噪音检测对象
    if ((rt = SiNoiseObjectRegister(algo, &noiseDetectVarianceObject)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "Noise Object Variance register fail:%s", SiErrRtDesp(rt));
    }
#endif

    //**********************************************************************************************
    //IOTouch配置

#if IOTOUCH_KEY1NUM
    keyNum = sizeof(iotouchKeyMap1) / sizeof(iotouchKeyMap1[0]);      //计算按键个数

    //IOTouch滤波器初始化
    if ((rt = SiFilterExtDoorctrlNodeInit(&iotouchFilterDoorctrl1, &iotouchFilterParaDoorctrl1, keyNum, iotouchFilterBufDoorctrl1)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Filter Doorctrl init fail:%s", SiErrRtDesp(rt));
    }
    //IOTouch基线追踪器初始化
    if ((rt = SiBaselineDoorctrl2MtStrongAvgNodeInit(&iotouchBaseline1, &iotouchBaselinePara1, keyNum, iotouchBaselineTempData1, keyNum, iotouchBaselineBuf1)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Baseline init fail:%s", SiErrRtDesp(rt));
    }
    //IOTouch判决器初始化
    if ((rt = SiArbiterFreq10DoorctrlNodeInit(&iotouchArbiter1, &iotouchArbiterPara1, IOtouchKeyValueChanged1)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Arbiter init fail:%s", SiErrRtDesp(rt));
    }
    /** @brief 注册按键组1判决器钩子，每个采样周期回调一次 */
    SiArbiterRegisterHook(&iotouchArbiter1, IOtouchKeyArbiterHook1);
    //IOTouch触摸对象初始化
    if ((rt = SiObjectNodeInit(&iotouchObjectKey1, SI_TYPE_KEY, SI_GROW_DIRECT_UP, keyNum, iotouchKeyMap1, iotouchKeyRawDataBuf1, &iotouchFilterDoorctrl1, &iotouchBaseline1, &iotouchArbiter1, NULL)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Object init fail:%s", SiErrRtDesp(rt));
    }
    //注册IOTouch触摸对象
    if ((rt = SiObjectRegister(algo, &iotouchObjectKey1)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Object register fail:%s", SiErrRtDesp(rt));
    }

#if LOW_POWER_EN
    //**************************************************************************************************
    //***IOTouch参数调节器配置
    SiParaAdjusterCompositeNodeInit(&iotouchBaselineParaAdjComp1, lpParaAdjusterSelectCallback);
    SiParaAdjusterLeafNodeInit(&iotouchBaselineParaAdjLeafNormal1, &iotouchBaselinePara1, &SiGetPara(iotouchBaseline1), sizeof(SiGetPara(iotouchBaseline1)));
    SiParaAdjusterLeafNodeInit(&iotouchBaselineParaAdjLeafLp1, &iotouchBaselinePara12, &SiGetPara(iotouchBaseline1), sizeof(SiGetPara(iotouchBaseline1)));
    //构建参数组合模块
    SiParaAdjusterAddChild((T_SiParaAdjusterBase *)&iotouchBaselineParaAdjComp1, (T_SiParaAdjusterBase *)&iotouchBaselineParaAdjLeafNormal1);
    SiParaAdjusterAddChild((T_SiParaAdjusterBase *)&iotouchBaselineParaAdjComp1, (T_SiParaAdjusterBase *)&iotouchBaselineParaAdjLeafLp1);
    /** @brief 为按键组1绑定基线参数调节器，支持常规/低功耗双模式切换 */
    SiObjectSetParaAdjuster(&iotouchObjectKey1, (T_SiParaAdjusterBase *)&iotouchBaselineParaAdjComp1);
#endif
#endif

#if IOTOUCH_KEY2NUM
    keyNum = sizeof(iotouchKeyMap2) / sizeof(iotouchKeyMap2[0]);      //计算按键个数

    //IOTouch滤波器初始化
    if ((rt = SiFilterExtDoorctrlNodeInit(&iotouchFilterDoorctrl2, &iotouchFilterParaDoorctrl2, keyNum, iotouchFilterBufDoorctrl2)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Filter Doorctrl init fail:%s", SiErrRtDesp(rt));
    }
    //IOTouch基线追踪器初始化
    if ((rt = SiBaselineDoorctrl2MtStrongAvgNodeInit(&iotouchBaseline2, &iotouchBaselinePara2, keyNum, iotouchBaselineTempData2, keyNum, iotouchBaselineBuf2)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Baseline init fail:%s", SiErrRtDesp(rt));
    }
    //IOTouch判决器初始化
    if ((rt = SiArbiterFreq10DoorctrlNodeInit(&iotouchArbiter2, &iotouchArbiterPara2, IOtouchKeyValueChanged2)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Arbiter init fail:%s", SiErrRtDesp(rt));
    }
    /** @brief 注册按键组2判决器钩子，每个采样周期回调一次 */
    SiArbiterRegisterHook(&iotouchArbiter2, IOtouchKeyArbiterHook2);
    //IOTouch触摸对象初始化
    if ((rt = SiObjectNodeInit(&iotouchObjectKey2, SI_TYPE_KEY, SI_GROW_DIRECT_UP, keyNum, iotouchKeyMap2, iotouchKeyRawDataBuf2, &iotouchFilterDoorctrl2, &iotouchBaseline2, &iotouchArbiter2, NULL)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Object init fail:%s", SiErrRtDesp(rt));
    }
    //注册IOTouch触摸对象
    if ((rt = SiObjectRegister(algo, &iotouchObjectKey2)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouch Object register fail:%s", SiErrRtDesp(rt));
    }

#if LOW_POWER_EN
    //**************************************************************************************************
    //***IOTouch参数调节器配置
    SiParaAdjusterCompositeNodeInit(&iotouchBaselineParaAdjComp2, lpParaAdjusterSelectCallback);
    SiParaAdjusterLeafNodeInit(&iotouchBaselineParaAdjLeafNormal2, &iotouchBaselinePara2, &SiGetPara(iotouchBaseline2), sizeof(SiGetPara(iotouchBaseline2)));
    SiParaAdjusterLeafNodeInit(&iotouchBaselineParaAdjLeafLp2, &iotouchBaselinePara22, &SiGetPara(iotouchBaseline2), sizeof(SiGetPara(iotouchBaseline2)));
    //构建参数组合模块
    SiParaAdjusterAddChild((T_SiParaAdjusterBase *)&iotouchBaselineParaAdjComp2, (T_SiParaAdjusterBase *)&iotouchBaselineParaAdjLeafNormal2);
    SiParaAdjusterAddChild((T_SiParaAdjusterBase *)&iotouchBaselineParaAdjComp2, (T_SiParaAdjusterBase *)&iotouchBaselineParaAdjLeafLp2);
    /** @brief 为按键组2绑定基线参数调节器，支持常规/低功耗双模式切换 */
    SiObjectSetParaAdjuster(&iotouchObjectKey2, (T_SiParaAdjusterBase *)&iotouchBaselineParaAdjComp2);
#endif
#endif

#if LOW_POWER_EN
    //**************************************************************************************************
    //***IOTouch低功耗配置

    keyNum = sizeof(iotouchLpKeyMap) / sizeof(iotouchLpKeyMap[0]);    //计算按键个数，低功耗借用普通模式下的按键表

    //IOTouch滤波器初始化
    if ((rt = SiFilterNoneNodeInit(&iotouchLpFilter, keyNum, iotouchLpFilterBuf)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouchLp :Filter init fail:%s", SiErrRtDesp(rt));
    }
    //IOTouch基线追踪器初始化
    if ((rt = SiBaselineKeyWkupQuickdropNodeInit(&iotouchLpBaseline, &iotouchLpBaselinePara, keyNum, iotouchLpBaselineTempData, keyNum, iotouchLpBaselineBuf)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouchLp Baseline init fail:%s", SiErrRtDesp(rt));
    }
    //IOTouch判决器初始化
    if ((rt = SiArbiterKeyMtAbsWkupNodeInit(&iotouchLpArbiter, &iotouchLpArbiterPara)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouchLp Arbiter init fail:%s", SiErrRtDesp(rt));
    }
    SiArbiterRegisterHook(&iotouchLpArbiter, IOtouchLpKeyArbiterHook);
    //IOTouch触摸对象初始化
#if defined(TOUCH_DEDICATE_IO_MODE) || defined(TOUCH_SHARE_IO_MODE)
    if ((rt = SiObjectNodeInit(&iotouchLpObjectKey, SI_TYPE_KEY, SI_GROW_DIRECT_DOWN, keyNum, iotouchLpKeyMap, iotouchLpKeyRawDataBuf, &iotouchLpFilter, &iotouchLpBaseline, &iotouchLpArbiter, NULL)) != SI_RT_OK)
#else
    if ((rt = SiObjectNodeInit(&iotouchLpObjectKey, SI_TYPE_KEY, SI_GROW_DIRECT_UP, keyNum, iotouchLpKeyMap, iotouchLpKeyRawDataBuf, &iotouchLpFilter, &iotouchLpBaseline, &iotouchLpArbiter, NULL)) != SI_RT_OK)
#endif
    {
        TC_LOGE(TAG, "IOTouchLp Object init fail:%s", SiErrRtDesp(rt));
    }
    //注册IOTouch触摸对象
    if ((rt = SiObjectRegister(algo, &iotouchLpObjectKey)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "IOTouchLp Object register fail:%s", SiErrRtDesp(rt));
    }
    SiObjectSetStatus(&iotouchLpObjectKey, SI_OBJECT_STATUS_LOCK);                       //平时锁定，低功耗模式下运行

#endif

#if NOISE_DETECT_VARIANCE_KEYNUM
    //***注册噪音检测描述符
    if ((rt = SiNoiseDetectRegister((T_SiNoiseBase *)&noiseDetectVariance, &doorctrlNoiseDetectVariance)) != SI_RT_OK)
    {
        TC_LOGE(TAG, "Touch Noise Detect Variance register fail:%s", SiErrRtDesp(rt));
    }
#endif
}

/**
 * @brief IOTouch按键状态改变公共回调函数
 *
 * 管理按键掩码（keyMask），当任意按键按下时置位对应位，释放时清除对应位。
 * 当按键掩码从0变非0或从非0变0时（即所有按键都释放时），触发门状态变化回调。
 *
 * @param[in] keyNo    按键编号（0基）
 * @param[in] status   按键状态（SI_KEY_PRESS / SI_KEY_RELEASE）
 *
 * @note 本函数维护 keyMask 状态，仅在整体状态发生变化时调用 TouchKeyCallback。
 *       按键状态变化通过 TC_LOGI 输出日志。
 */
static void IOtouchKeyValueChanged(uint8_t keyNo, T_SiKeyStatus status)
{
    static uint8_t keyMask = 0;
    uint8_t lastState;
    uint8_t curState;

    lastState = (keyMask ? 1 : 0);

    if (status == SI_KEY_PRESS)
    {
        keyMask |= (0x1 << keyNo);
        TC_LOGI(TAG, "k %d press", keyNo + 1);
    }
    if (status == SI_KEY_RELEASE)
    {
        keyMask &= ~(0x1 << keyNo);
        TC_LOGI(TAG, "k %d release", keyNo + 1);
    }

    curState = (keyMask ? 1 : 0);

    if (lastState != curState)
    {
        if (curState)
        {
            TC_LOGI(TAG, "door open");
        }
        if (status == SI_KEY_RELEASE)
        {
            TC_LOGI(TAG, "door close");
        }
        TouchKeyCallback(0, status);
    }
}

/**
 * @brief 触摸按键回调函数（弱符号，可被用户重写）
 *
 * 当门状态发生变化时被 IOtouchKeyValueChanged 调用。
 * 默认实现为空函数，用户可在其它文件中定义同名函数覆盖。
 *
 * @param[in] keyNo    按键编号
 * @param[in] status   按键状态
 *
 * @note 本函数使用 __WEAK 属性声明，允许用户在外部覆盖实现。
 */
__WEAK void TouchKeyCallback(uint8_t keyNo, T_SiKeyStatus status)
{
    (void)keyNo;
    (void)status;
}

#if IOTOUCH_KEY1NUM
/**
 * @brief IOTouch按键状态改变回调函数（按键组1）
 *
 * 更新 touch_data.key_val 中按键组1对应的位（bit0），
 * 并调用公共回调 IOtouchKeyValueChanged。
 *
 * @param[in] keyNo    按键编号
 * @param[in] status   按键状态（SI_KEY_PRESS / SI_KEY_RELEASE）
 */
static void IOtouchKeyValueChanged1(uint8_t keyNo, T_SiKeyStatus status)
{
    if (status == SI_KEY_PRESS)
    {
        touch_data.key_val |= (0x1 << 0);
    }
    if (status == SI_KEY_RELEASE)
    {
        touch_data.key_val &= ~(0x1 << 0);
    }
    IOtouchKeyValueChanged(0, status);
}

/**
 * @brief IOTouch判决器钩子函数（按键组1）
 *
 * 在每个采样周期被判决器回调，执行以下功能：
 * - 低功耗模式下检测按键触发以复位休眠超时或强制唤醒
 * - 计算滤波值与基线的差分值（diffValues）
 * - 更新 touch_data 中的原始值、基线值、差分值（+32768 转为无符号存储）
 * - 调试模式下周期性输出 diff 日志
 *
 * @param[in] obj  IOTouch对象指针，用于查询基线状态等
 *
 * @note diffValues = 滤波值 - 基线值
 *       存储时 +32768 转换为 uint16 范围存储，避免负数
 */
static void IOtouchKeyArbiterHook1(T_SiObject *obj)
{
#if LOW_POWER_EN
    if (!SiArbiterCanEnterHalt(obj))
    {
        HaltTimeoutReset();
    }
    for (int i = 0; i < IOTOUCH_KEY1NUM; ++i)
    {
        if (iotouchFilterBufDoorctrl1[i] - iotouchBaselineBuf1[i] >= iotouchArbiterPara1.detectChannel.pressThreshold1)
        {
            TouchForceWakeup(); //强制从低功耗唤醒
        }
    }
#endif

    T_SiData diffValues[IOTOUCH_KEY1NUM];
    for (int i = 0; i < IOTOUCH_KEY1NUM; ++i)
    {
        diffValues[i] = iotouchFilterBufDoorctrl1[i] - iotouchBaselineBuf1[i];

        touch_data.key1_raw[i] = iotouchFilterBufDoorctrl1[i] + 32768;
        touch_data.key1_base[i] = iotouchBaselineBuf1[i] + 32768;
        touch_data.key1_diff[i] = diffValues[i] + 32768;
    }

#if DEBUG_PRINT_EN
    static int printcnt = 0;
    if (++printcnt % 25 != 0)
    {
        return;
    }

    if (SiBaselineIsReady(obj, 0))
    {
//        TC_LOG_SYMBOL_I16("raw1", iotouchFilterBufDoorctrl1, sizeof(iotouchFilterBufDoorctrl1[0]));
//        TC_LOG_SYMBOL_I16("base1", iotouchBaselineBuf1, sizeof(iotouchBaselineBuf1[0]));

//        int32_t uv = iotouchArbiter1.data.freqChannel.unlockValue;
//        TC_LOG_SYMBOL_I32("uv1", &uv, sizeof(uv));
//        uint8_t lb = iotouchArbiter.data.freqChannel.lockBaseline * 100;
//        TC_LOG_SYMBOL_U8("lb", &lb, sizeof(lb));
//        TC_LOG_SYMBOL_I16("pv1", &iotouchArbiter1.data.freqChannel.quickUnlockBaseline.postValue, sizeof(iotouchArbiter1.data.freqChannel.quickUnlockBaseline.postValue));

        uint8_t printSpeedNoise = 0;

        if (iotouchArbiter1.data.status)
        {
            printSpeedNoise = 1;
        }
        if (printSpeedNoise)
        {
//            TC_LOG_SYMBOL_I16("fv1", &iotouchArbiter.data.freqChannel.diff2Stage, sizeof(iotouchArbiter.data.freqChannel.diff2Stage));
//            TC_LOG_SYMBOL_I16("mv1", &iotouchArbiter.data.mergeChannel.value, sizeof(iotouchArbiter.data.mergeChannel.value));
//            TC_LOG_SYMBOL_I16("mvv1", &iotouchArbiter.data.mergeChannel.validValue, sizeof(iotouchArbiter.data.mergeChannel.validValue));
//            TC_LOG_SYMBOL_I16("mfvv1", &iotouchArbiter.data.mergeChannel.forceValidValue, sizeof(iotouchArbiter.data.mergeChannel.forceValidValue));
//            TC_LOG_SYMBOL_I16("noise", noiseValues, sizeof(noiseValues));
//            TC_LOG_SYMBOL_I16("noiseKeepT", noiseKeepTimeCnt, sizeof(noiseKeepTimeCnt));
//            TC_LOG_SYMBOL_I16("maxdiff", maxDiffValues, sizeof(maxDiffValues));
//            TC_LOG_SYMBOL_U16("threshold12TimeCnt", threshold12TimeCnt, sizeof(threshold12TimeCnt));
//            TC_LOG_SYMBOL_U16("th2KeepT", threshold2KeepTime, sizeof(threshold2KeepTime));
        }
        TC_LOG_SYMBOL_I16("diff1", diffValues, sizeof(diffValues[0]));
        TC_LOG_SYMBOL_I16("rain1", &diffValues[1], sizeof(diffValues[1]));
    }
#endif
}
#endif

#if IOTOUCH_KEY2NUM
/**
 * @brief IOTouch按键状态改变回调函数（按键组2）
 *
 * 更新 touch_data.key_val 中按键组2对应的位（bit1），
 * 并调用公共回调 IOtouchKeyValueChanged。
 *
 * @param[in] keyNo    按键编号
 * @param[in] status   按键状态（SI_KEY_PRESS / SI_KEY_RELEASE）
 */
static void IOtouchKeyValueChanged2(uint8_t keyNo, T_SiKeyStatus status)
{
    if (status == SI_KEY_PRESS)
    {
        touch_data.key_val |= (0x1 << 1);
    }
    if (status == SI_KEY_RELEASE)
    {
        touch_data.key_val &= ~(0x1 << 1);
    }
    IOtouchKeyValueChanged(1, status);
}

/**
 * @brief IOTouch判决器钩子函数（按键组2）
 *
 * 在每个采样周期被判决器回调，执行以下功能：
 * - 低功耗模式下检测按键触发以复位休眠超时或强制唤醒
 * - 计算滤波值与基线的差分值（diffValues）
 * - 更新 touch_data 中的原始值、基线值、差分值（+32768 转为无符号存储）
 * - 调试模式下周期性输出 diff 日志
 *
 * @param[in] obj  IOTouch对象指针，用于查询基线状态等
 *
 * @note diffValues = 滤波值 - 基线值
 *       存储时 +32768 转换为 uint16 范围存储，避免负数
 */
static void IOtouchKeyArbiterHook2(T_SiObject *obj)
{
#if LOW_POWER_EN
    if (!SiArbiterCanEnterHalt(obj))
    {
        HaltTimeoutReset();
    }
    for (int i = 0; i < IOTOUCH_KEY2NUM; ++i)
    {
        if (iotouchFilterBufDoorctrl2[i] - iotouchBaselineBuf2[i] >= iotouchArbiterPara2.detectChannel.pressThreshold1)
        {
            TouchForceWakeup(); //强制从低功耗唤醒
        }
    }
#endif

    T_SiData diffValues[IOTOUCH_KEY2NUM];
    for (int i = 0; i < IOTOUCH_KEY2NUM; ++i)
    {
        diffValues[i] = iotouchFilterBufDoorctrl2[i] - iotouchBaselineBuf2[i];
        touch_data.key2_raw[i] = iotouchFilterBufDoorctrl2[i] + 32768;
        touch_data.key2_base[i] = iotouchBaselineBuf1[i] + 32768;
        touch_data.key2_diff[i] = diffValues[i] + 32768;
    }

#if DEBUG_PRINT_EN
    static int printcnt = 0;
    if (++printcnt % 25 != 0)
    {
        return;
    }

    if (SiBaselineIsReady(obj, 0))
    {
//        TC_LOG_SYMBOL_I16("raw2", iotouchFilterBufDoorctrl2, sizeof(iotouchFilterBufDoorctrl2[0]));
//        TC_LOG_SYMBOL_I16("base2", iotouchBaselineBuf2, sizeof(iotouchBaselineBuf2[0]));

//        int32_t uv = iotouchArbiter2.data.freqChannel.unlockValue;
//        TC_LOG_SYMBOL_I32("uv2", &uv, sizeof(uv));
//        uint8_t lb = iotouchArbiter.data.freqChannel.lockBaseline * 100;
//        TC_LOG_SYMBOL_U8("lb", &lb, sizeof(lb));
//        TC_LOG_SYMBOL_I16("pv", &iotouchArbiter.data.freqChannel.quickUnlockBaseline.postValue, sizeof(iotouchArbiter.data.freqChannel.quickUnlockBaseline.postValue));

        uint8_t printSpeedNoise = 0;

        if (iotouchArbiter2.data.status)
        {
            printSpeedNoise = 1;
        }
        if (printSpeedNoise)
        {
//            TC_LOG_SYMBOL_I16("fv1", &iotouchArbiter.data.freqChannel.diff2Stage, sizeof(iotouchArbiter.data.freqChannel.diff2Stage));
//            TC_LOG_SYMBOL_I16("mv1", &iotouchArbiter.data.mergeChannel.value, sizeof(iotouchArbiter.data.mergeChannel.value));
//            TC_LOG_SYMBOL_I16("mvv1", &iotouchArbiter.data.mergeChannel.validValue, sizeof(iotouchArbiter.data.mergeChannel.validValue));
//            TC_LOG_SYMBOL_I16("mfvv1", &iotouchArbiter.data.mergeChannel.forceValidValue, sizeof(iotouchArbiter.data.mergeChannel.forceValidValue));
//            TC_LOG_SYMBOL_I16("noise", noiseValues, sizeof(noiseValues));
//            TC_LOG_SYMBOL_I16("noiseKeepT", noiseKeepTimeCnt, sizeof(noiseKeepTimeCnt));
//            TC_LOG_SYMBOL_I16("maxdiff", maxDiffValues, sizeof(maxDiffValues));
//            TC_LOG_SYMBOL_U16("threshold12TimeCnt", threshold12TimeCnt, sizeof(threshold12TimeCnt));
//            TC_LOG_SYMBOL_U16("th2KeepT", threshold2KeepTime, sizeof(threshold2KeepTime));
        }
        TC_LOG_SYMBOL_I16("diff2", diffValues, sizeof(diffValues[0]));
        TC_LOG_SYMBOL_I16("rain2", &diffValues[1], sizeof(diffValues[1]));
    }
#endif
}
#endif

#if LOW_POWER_EN
/**
 * @brief IOTouch低功耗判决器钩子函数
 *
 * 在低功耗模式下每个采样周期被回调，当基线就绪时
 * 可输出调试日志（仅在 DEBUG_PRINT_EN 使能时生效）。
 *
 * @param[in] obj  IOTouch对象指针
 *
 * @note 当前实现中仅在基线就绪时做空检查，实际业务逻辑为空。
 *       调试日志已被注释掉，需要时可取消注释。
 */
static void IOtouchLpKeyArbiterHook(T_SiObject *obj)
{
    if (SiBaselineIsReady(obj, 0))
    {
#if DEBUG_PRINT_EN
//        TC_LOG_SYMBOL_I16("iotouchLpFilterBuf", iotouchLpFilterBuf, sizeof(iotouchLpFilterBuf));
//        TC_LOG_SYMBOL_I16("iotouchLpBaselineBuf", iotouchLpBaselineBuf, sizeof(iotouchLpBaselineBuf));
#endif
    }
}
#endif

/**
 * @brief 触摸准备进入低功耗钩子函数
 *
 * 在系统进入低功耗前被调用，执行以下操作：
 * - 锁定噪音检测对象
 * - 锁定常规模式 IOTouch 对象（KEY1/KEY2）
 * - 开启低功耗模式 IOTouch 对象
 * - 复位低功耗滤波器、基线追踪器、仲裁器
 * - 切换到低功耗调度器
 *
 * @note 仅在 LOW_POWER_EN 使能时执行低功耗相关逻辑。
 *       调度器切换通过 SiSetScheduler 完成。
 */
void TouchEnterHaltHook(void)
{
    T_SiErrRt rt;

#if NOISE_DETECT_VARIANCE_KEYNUM
    SiNoiseObjectSetStatus(&noiseDetectVarianceObject, SI_NOISE_OBJECT_STATUS_LOCK);
//    SiNoiseResetAll(&noiseDetectVarianceObject);
#endif

    //**********************************************************************************************
    //IOTouch
#if IOTOUCH_KEY1NUM
    SiObjectSetStatus(&iotouchObjectKey1, SI_OBJECT_STATUS_LOCK);   //锁定常规模式的iotouch
#endif
#if IOTOUCH_KEY2NUM
    SiObjectSetStatus(&iotouchObjectKey2, SI_OBJECT_STATUS_LOCK);   //锁定常规模式的iotouch
#endif

#if LOW_POWER_EN
    SiObjectSetStatus(&iotouchLpObjectKey, SI_OBJECT_STATUS_RUN);  //开启低功耗模式的iotouch

    if ((rt = SiFilterResetAll(&iotouchLpObjectKey)) != SI_RT_OK)    //低功耗滤波器复位
    {
        TC_LOGE(TAG, "IOTouchLp filter reset fail:%s", SiErrRtDesp(rt));
    }
    if ((rt = SiBaselineResetAll(&iotouchLpObjectKey)) != SI_RT_OK)  //低功耗基线追踪器复位
    {
        TC_LOGE(TAG, "IOTouchLp baseline reset fail:%s", SiErrRtDesp(rt));
    }
    if ((rt = SiArbiterReset(&iotouchLpObjectKey)) != SI_RT_OK)      //低功耗仲裁器复位
    {
        TC_LOGE(TAG, "IOTouchLp arbiter reset fail:%s", SiErrRtDesp(rt));
    }
#endif

    //**********************************************************************************************
    //其它低功耗相关

    if ((rt = SiSetScheduler(&touchAlgoObject, SI_SCHEDULER_LOWPOWER)) != SI_RT_OK)   //切换到低功耗调度器
    {
        TC_LOGE(TAG, "TouchLp set scheduler fail:%s", SiErrRtDesp(rt));
    }
}

/**
 * @brief 触摸从低功耗唤醒钩子函数
 *
 * 在系统从低功耗唤醒后被调用，执行以下操作：
 * - 开启噪音检测对象
 * - 开启常规模式 IOTouch 对象（KEY1/KEY2）
 * - 锁定低功耗模式 IOTouch 对象
 * - 切换回运行时调度器
 *
 * @note 与 TouchEnterHaltHook 成对使用，完成低功耗状态的逆操作。
 *       调度器切换通过 SiSetScheduler 完成。
 */
void TouchWakeupHook(void)
{
    T_SiErrRt rt;

#if NOISE_DETECT_VARIANCE_KEYNUM
    SiNoiseObjectSetStatus(&noiseDetectVarianceObject, SI_NOISE_OBJECT_STATUS_RUN);
#endif

    //**********************************************************************************************
    //IOTouch
#if IOTOUCH_KEY1NUM
    SiObjectSetStatus(&iotouchObjectKey1, SI_OBJECT_STATUS_RUN);     //开启常规模式的iotouch
#endif
#if IOTOUCH_KEY2NUM
    SiObjectSetStatus(&iotouchObjectKey2, SI_OBJECT_STATUS_RUN);     //开启常规模式的iotouch
#endif

#if LOW_POWER_EN
    SiObjectSetStatus(&iotouchLpObjectKey, SI_OBJECT_STATUS_LOCK);  //锁定低功耗模式的iotouch
#endif

    //**********************************************************************************************
    //其它低功耗相关
    if ((rt = SiSetScheduler(&touchAlgoObject, SI_SCHEDULER_BALANCE)) != SI_RT_OK)   //切换到运行时调度器
    {
        TC_LOGE(TAG, "Touch set scheduler fail:%s", SiErrRtDesp(rt));
    }
}

/**
 * @brief 锁定管理低功耗的 T_SiObject
 *
 * 将低功耗 IOTouch 对象状态设为 SI_OBJECT_STATUS_LOCK，
 * 禁止其在低功耗模式下运行。
 *
 * @note 仅在 LOW_POWER_EN 使能时生效。
 */
void TouchHaltLockLpSiObject(void)
{
#if LOW_POWER_EN
    SiObjectSetStatus(&iotouchLpObjectKey, SI_OBJECT_STATUS_LOCK);     //锁定低功耗object
#endif
}

/**
 * @brief 解锁管理低功耗的 T_SiObject
 *
 * 将低功耗 IOTouch 对象状态设为 SI_OBJECT_STATUS_RUN，
 * 允许其在低功耗模式下运行。
 *
 * @note 仅在 LOW_POWER_EN 使能时生效。
 */
void TouchHaltUnlockLpSiObject(void)
{
#if LOW_POWER_EN
    SiObjectSetStatus(&iotouchLpObjectKey, SI_OBJECT_STATUS_RUN);     //开启低功耗object
#endif
}

/**
 * @brief 锁定常规模式的 T_SiObject
 *
 * 将常规 IOTouch 对象（KEY1/KEY2）和噪音检测对象状态设为 LOCK，
 * 暂停其在正常运行模式下的处理。
 *
 * @note 同时锁定噪音检测对象（NOISE_DETECT_VARIANCE_KEYNUM 使能时）。
 */
void TouchHaltLockSiObject(void)
{
#if IOTOUCH_KEY1NUM
    SiObjectSetStatus(&iotouchObjectKey1, SI_OBJECT_STATUS_LOCK);   //锁定常规模式的iotouch
#endif
#if IOTOUCH_KEY2NUM
    SiObjectSetStatus(&iotouchObjectKey2, SI_OBJECT_STATUS_LOCK);   //锁定常规模式的iotouch
#endif

#if NOISE_DETECT_VARIANCE_KEYNUM
    SiNoiseObjectSetStatus(&noiseDetectVarianceObject, SI_NOISE_OBJECT_STATUS_LOCK);
#endif
}

/**
 * @brief 解锁常规模式的 T_SiObject
 *
 * 将常规 IOTouch 对象（KEY1/KEY2）和噪音检测对象状态设为 RUN，
 * 恢复其在正常运行模式下的处理。
 *
 * @note 同时解锁噪音检测对象（NOISE_DETECT_VARIANCE_KEYNUM 使能时）。
 */
void TouchHaltUnlockSiObject(void)
{
#if IOTOUCH_KEY1NUM
    SiObjectSetStatus(&iotouchObjectKey1, SI_OBJECT_STATUS_RUN);     //开启常规模式的iotouch
#endif
#if IOTOUCH_KEY2NUM
    SiObjectSetStatus(&iotouchObjectKey2, SI_OBJECT_STATUS_RUN);     //开启常规模式的iotouch
#endif

#if NOISE_DETECT_VARIANCE_KEYNUM
    SiNoiseObjectSetStatus(&noiseDetectVarianceObject, SI_NOISE_OBJECT_STATUS_RUN);
#endif
}

#if NOISE_DETECT_VARIANCE_KEYNUM
/**
 * @brief 噪音检测状态变化回调函数
 *
 * 当噪音检测状态发生变化时被调用：
 * - 检测到噪音时输出 "Variance Noise detected!" 日志并延长休眠超时周期
 * - 噪音消失时输出 "Variance Noise release!" 日志并恢复休眠超时周期
 *
 * @param[in] status 噪音状态，非0表示检测到噪音，0表示噪音消失
 *
 * @note 低功耗模式下通过 HaltTimeoutChgPeriod 调整休眠超时周期。
 */
static void DoorCtrlNoiseDetectVarianceStatusChanged(uint8_t status)
{
    if (status)
    {
        TC_LOGI(TAG, "Variance Noise detected!");

#if LOW_POWER_EN
        HaltTimeoutChgPeriod(10000);
#endif
    }
    else
    {
        TC_LOGI(TAG, "Variance Noise release!");

#if LOW_POWER_EN
        HaltTimeoutChgPeriod(5000);
#endif
    }
}
#endif

#if NOISE_DETECT_VARIANCE_KEYNUM
/**
 * @brief 噪声检测器钩子函数
 *
 * 在每个噪音检测周期被回调，执行以下功能：
 * - 低功耗模式下检测到噪音时复位休眠超时，阻止进入低功耗
 * - 更新 touch_data.noise_raw 噪音原始值
 * - 调试模式下周期性输出噪音检测数据
 *
 * @param[in] obj      噪音检测对象指针
 * @param[in] keyNum   按键编号
 * @param[in] noiseBuf 噪音数据缓冲区指针
 *
 * @note 噪音原始值 +32768 转为无符号存储。
 *       通过 SiNoiseIsDetect3 判断是否检测到噪音。
 */
static void NoiseDetectVarianceHook(T_SiNoiseObject *obj, uint8_t keyNum, const T_SiNoiseData *noiseBuf)
{
#if LOW_POWER_EN
#if IOTOUCH_KEY1NUM
    if (SiNoiseIsDetect3(&iotouchObjectKey1))    //检测到噪音不进低功耗
    {
        HaltTimeoutReset();
    }
#endif
#if IOTOUCH_KEY2NUM
    if (SiNoiseIsDetect3(&iotouchObjectKey2))    //检测到噪音不进低功耗
    {
        HaltTimeoutReset();
    }
#endif
#endif

    touch_data.noise_raw = noiseDetectVarianceRawDataBuf[0] + 32768;

#if DEBUG_PRINT_EN == 1
    static int printcnt = 0;
    if (++printcnt % 25 != 0)
    {
        return;
    }

//    TC_LOG_SYMBOL_I16("nraw", noiseDetectVarianceRawDataBuf, sizeof(noiseDetectVarianceRawDataBuf));
    TC_LOG_SYMBOL_I16("nv", noiseDetectVarianceBuf, sizeof(noiseDetectVarianceBuf));
//		TC_LOG_SYMBOL_I16("e1", &noiseExitConditionVariance.base.noiseValue, sizeof(noiseExitConditionVariance.base.noiseValue));
#endif
}
#endif

/**
 * @brief 强制同步基线值为当前滤波值
 *
 * 遍历所有按键组（KEY1/KEY2），将每个通道的基线值
 * 强制设置为当前滤波值（raw值），用于基线校准场景。
 *
 * @note 通过 SiBaselineSet 接口设置基线值。
 *       仅在对应按键组使能时执行。
 */
void TouchKeyForceSyncBaseline(void)
{
#if IOTOUCH_KEY1NUM
    for (int i = 0; i < IOTOUCH_KEY1NUM; ++i)
    {
        SiBaselineSet(&iotouchObjectKey1, i, iotouchFilterBufDoorctrl1[i]);
    }
#endif
#if IOTOUCH_KEY2NUM
    for (int i = 0; i < IOTOUCH_KEY2NUM; ++i)
    {
        SiBaselineSet(&iotouchObjectKey2, i, iotouchFilterBufDoorctrl2[i]);
    }
#endif
}
