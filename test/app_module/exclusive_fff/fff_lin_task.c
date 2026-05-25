#include "fff_tc.h"
#include "fff_tc_log.h"
#include "fff_tcae10_ll_def.h"
#include "fff_tc_usermsg.h"
#include "fff_tc_halt.h"
#include "fff_app.h"
#include "fff_lin_process.h"
// #include "fff_lin_snpd.h"
#include "fff_diagnosticIII.h"
#include "fff_lin_task.h"
// #include "fff_pal_lin_comm.h"

static const char *TAG = "APP";

static volatile uint8_t linWakeupFlag = 0;
static T_TcTask *linTask = NULL;
static uint8_t linCommSincePowerOn = 0;

static void LinTask(uint32_t msg, void *param);    //Lin任务
static int LinHaltFilterWakeupCallback(void);      //Lin唤醒回调
static void LinHaltMonitorCallback(void);          //Lin低功耗监控器
extern void lin_goto_idle_state(void);
extern void lin_uncd_frame_handle(void);

DEFINE_FAKE_VOID_FUNC(LinInit);
DEFINE_FAKE_VALUE_FUNC(unsigned char,LinCommSincePowerOn);
DEFINE_FAKE_VOID_FUNC(LinDestroy);
DEFINE_FAKE_VALUE_FUNC(int,LinCanEnterSleep);
