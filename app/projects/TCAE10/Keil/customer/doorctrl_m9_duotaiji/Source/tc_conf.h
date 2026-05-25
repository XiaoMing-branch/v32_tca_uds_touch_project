
#ifndef __TC_CONF_H__
#define __TC_CONF_H__

/*每秒系统时钟ms数*/
#define TC_SYSTICK_HZ       1000

/*每种优先级最大支持任务数*/
#define TC_TASK_NUM     5

/*消息缓冲区大小*/
#define TC_MSG_NUM      3

/*定时器个数*/
#define TC_TIMER_NUM    5

/*Systick中断回调函数可注册个数*/
#define MAX_SYSTICK_ISR_CALLBACK_NUM        1           

#define SYSTICK_FREQ_HZ     (1000)     //systick中断频率

#define TC_SAVE_RAM_MODE    1          //节省ram

#endif
