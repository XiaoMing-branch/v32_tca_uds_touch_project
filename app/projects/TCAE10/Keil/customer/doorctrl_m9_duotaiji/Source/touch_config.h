#ifndef __TOUCH_CONFIG_H__
#define __TOUCH_CONFIG_H__

#define TOUCH_CLK_DIV       0                           //时钟分频，48M分频
#define TOUCH_ADC_DIV       1                           //Touch的Adc时钟分频，24M分频

#define TOUCH_NORMAL_MONITOR_TIMEOUTMS      1000        //touch普通模式下监控器超时时间
#define TOUCH_HALT_MONITOR_TIMEOUT          3           //touch低功耗监控器超时时间

#define IOTOUCH_KEY1NUM             2
#define IOTOUCH_KEY2NUM             2

#define IOTOUCH_LP_KEYNUM           2

#define TOUCH_FAST_FREQ                 200                 //快扫频率
#define TOUCH_SLOW_FREQ                 32                  //慢扫频率
#define TOUCH_SLEEP_FREQ                20                  //低功耗模式下扫描频率
#define TOUCH_SLEEP_MASK_FREQ           20                   //低功耗模式下，被屏蔽的通道，间隔多少周期后扫描一次

//#define TOUCH_FAST2SLOW_SWITCHTIMEOUT   3000              //快慢扫切换时间

#define TOUCH_TRIG_IN_SLEEP_MODE        0                   //sleep模式中trig，以降低功耗
#define TOUCH_EXTSYNC_TRIG_ENABLE       0                   //外部同步trig开关

#define TOUCH_SKIP_STARTUP_DATAS        30                  //上电时丢弃一些不稳定的touch数据

#define NOISE_DETECT_VARIANCE_KEYNUM    1                   //方差噪音检测通道个数

//配置触摸功能和算法参数
void TouchConfig(T_SiAlgoObject *algo);

//触摸准备进入低功耗
void TouchEnterHaltHook(void);

//触摸从低功耗唤醒
void TouchWakeupHook(void);

void TouchKeyForceSyncBaseline(void);       //强制拉升baseline为raw值

void TouchKeyCallback(uint8_t keyNo, T_SiKeyStatus status);

#endif
