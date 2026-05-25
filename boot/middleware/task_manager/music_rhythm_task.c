/**
 * Copyright (C) 2018 Rebo, All rights reserved.
 * @file    music_rhythm_app.c
 * @brief
 */

/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
#include "osal.h"
#include "music_rhythm_task.h"
#include "parse_music_api.h"
#include "light_effect_api.h"
#include "parse_algo_main.h"
#include "light_effect_main.h"
#include "colormixing.h"
#include <string.h>
#include <stdint.h>
#include "lin.h"
/*******************************************************************************
 *    MACRO DEFINITIONS
 ******************************************************************************/
extern uint8_t           lin_pFrameBuf[];
/*******************************************************************************
 *    STATIC FUNCTION DECLARATIONS
 ******************************************************************************/
/** @brief 效果属性[0_0_0] — 呼吸灯效果，数据源为音乐输出通道0 */
const static effect_attri_t k_effect_attr0_0_0 =
	{
		.effect_type = effect_breath,
		.source = &g_music_msg_output.output[0],
};

/** @brief 效果属性[1_0_0] — 炫彩效果，数据源为音乐输出通道1 */
const static effect_attri_t k_effect_attr1_0_0 =
	{
		.effect_type = effect_colorful,
		.source = &g_music_msg_output.output[1],
};

/** @brief 效果属性[2_0_0] — 混色效果，数据源为音乐输出通道1，灯位0 */
const static effect_attri_t k_effect_attr2_0_0 =
	{
		.effect_type = effect_color_mix,
		.source = &g_music_msg_output.output[1],
		.attri.coolormix.lamp_loc = 0,
};

/** @brief 颜色数组[0_0_0] — 12种颜色的渐变调色板（白色→青蓝→青绿→黄绿） */
const static effect_color_rgb_t k_color_array_0_0_0[12] =
	{
		{255, 255, 255},
		{76, 151, 220},
		{76, 161, 213},
		{77, 174, 202},
		{78, 187, 190},
		{78, 198, 178},
		{79, 209, 164},
		{79, 219, 149},
		{80, 228, 131},
		{81, 237, 110},
		{115, 232, 101},
		{149, 219, 100},
};

/** @brief 画笔属性[0_0_0] — 单色画笔，使用k_color_array_0_0_0颜色序列，速度1 */
const static brush_attri_u k_brush_0_0_0 =
	{
		.color_single.type = BRUSH_TYPE_SINGLE,
		.color_single.color_len = 12u,
		.color_single.color = k_color_array_0_0_0,
		.color_single.color_bg = {0x00, 0x00, 0x00},
		.color_single.speed = 1u};

/** @brief 颜色数组[1_0_0] — 3种霓虹色（橙、紫、青绿） */
const static effect_color_rgb_t k_color_array_1_0_0[3] =
	{
		{255, 128, 0},
		{128, 0, 255},
		{0, 255, 128},
};

/** @brief 画笔属性[1_0_0] — 单色画笔，使用k_color_array_1_0_0颜色序列，速度1 */
const static brush_attri_u k_brush_1_0_0 =
	{
		.color_single.type = BRUSH_TYPE_SINGLE,
		.color_single.color_len = 3u,
		.color_single.color = k_color_array_1_0_0,
		.color_single.color_bg = {0x00, 0x00, 0x00},
		.color_single.speed = 1u};

/** @brief 效果设计[0_0] — 呼吸灯 + 渐变画笔 */
static effect_design_t g_design_0_0[1] =
	{
		{.effect_attri = &k_effect_attr0_0_0, .brush_attri = &k_brush_0_0_0},
	};

/** @brief 效果设计[1_0] — 炫彩 + 霓虹画笔 */
static effect_design_t g_design_1_0[1] =
	{
		{.effect_attri = &k_effect_attr1_0_0, .brush_attri = &k_brush_1_0_0},
	};

/** @brief 效果设计[2_0] — 混色 + 渐变画笔 */
static effect_design_t g_design_2_0[1] =
	{
		{.effect_attri = &k_effect_attr2_0_0, .brush_attri = &k_brush_0_0_0},
	};

/** @brief 效果设计集线器 — 包含3种主效果设计（呼吸灯/炫彩/混色） */
const static effect_design_hub_t g_vm_design_hub[3] =
	{
		{&g_design_0_0[0], _SIZE(g_design_0_0)},
		{&g_design_1_0[0], _SIZE(g_design_1_0)},
		{&g_design_2_0[0], _SIZE(g_design_2_0)},
};

/** @brief 效果属性[1_1_1] — 待机呼吸灯，数据源为待机计数 */
const static effect_attri_t k_effect_attr1_1_1 =
	{
		.effect_type = effect_breath,
		.source = &g_effect_cfg.standby_cnt,
};

/** @brief 效果设计[1_1] — 待机呼吸灯 + 渐变画笔 */
static effect_design_t g_design_1_1[1] =
	{
		{.effect_attri = &k_effect_attr1_1_1, .brush_attri = &k_brush_0_0_0},
	};

/** @brief 待机效果设计集线器 — 包含1种待机效果（呼吸灯） */
const static effect_design_hub_t g_vm_design_standby[1] =
	{
		{&g_design_1_1[0], SIZE(g_design_1_1)},
};

/*******************************************************************************
 *    CONSTANT VARIABLES DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    STATIC VARIABLES DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    GLOBAL VARIABLES DEFINITIONS
 ******************************************************************************/

/**
 * @brief 灯效注册消息 — 注册所有效果设计到灯效引擎
 * @note  包含主效果集线器(3种)、待机效果集线器(1种)、
 *        亮度等级20，回调函数在初始化时注册
 */
le_register_msg_t g_vm_register_msg1 =
	{
		.design_hub = &g_vm_design_hub[0],
		.design_major_max = 3u,
		.design_std_hub = g_vm_design_standby,
		.design_std_hub_max = 1u,
		.intensity_level = 20u,
		.light_output_cb = NULL,
		.mute_cb = NULL,
		//.is_active = TRUE
};

/*******************************************************************************
 *    FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief  静音检测回调函数
 * @param  reg_dev - 注册设备指针（le_register_msg_t类型）
 * @retval 无
 */
static void mute_detect_cb(void *reg_dev)
{
	// uint8 idx;
	// le_register_msg_t* dev = (le_register_msg_t*)reg_dev;

	// idx = (uint8)((g_standby_cnt / STANDBY_CNT_MAX) % (uint32)dev->design_std_hub_max);

	// if (dev->design_std_hub_use != idx)
	// {
	// 	dev->design_std_hub_use = idx;
	// 	UNUSED(light_effect_calibration(dev));
	// }
}

/**
 * @brief  灯光输出回调函数
 *         将灯效引擎生成的RGBA颜色数据输出到LED硬件
 * @param  color  - RGBI颜色数据指针
 * @param  led_num - LED数量
 * @retval 无
 */
static void light_output(effect_color_rgbi_t *color, int led_num)
{
	// to light led by using color
	cm_set_target_RGBL(color->R, color->G, color->B, (uint16_t)(color->intensity)*39, 0);
	cm_target_RGBL_lighting(1);
}

/**
 * @brief  频率信号更新处理函数
 *         从LIN接收帧数据拷贝到频率缓冲区，供音乐解析算法使用
 * @param  freq    - 频率数据缓冲区指针
 * @param  freq_len - 频率数据长度（需≥8字节）
 * @retval 无
 */
static void freq_singal_update_handle(uint8 *freq, int freq_len)
{
	if (freq_len < 8u)
	{
		return;
	}

	// need to fill freq by using lin data
	memcpy((void *)freq, (const void *)&(lin_pFrameBuf[0]), 8u);
}


/**
 * @brief  音乐律动应用任务初始化
 *         初始化音乐解析算法和灯效算法，注册效果设计和回调函数，
 *         创建音乐律动处理任务
 * @retval STD_E_OK    - 初始化成功
 * @retval STD_E_NOT_OK - 初始化失败
 */
std_rtn_type music_rhythm_app_task_init(void)
{
	parse_cfg_t parse_init = {0};
	effect_init_t effect_init;

	// 1. initialize The parsing algorithm
	parse_init.use_fre_spec = TRUE;
	parse_init.use_major_ratio = TRUE;
	parse_init.use_minor_ratio = TRUE;
	parse_init.fre_spec_len = FREQ_LEN_USE;
	if ((parse_music_init(&parse_init)) != STD_E_OK)
	{
		return STD_E_NOT_OK;
	}

	// 2. initialize the light effect algorithm
	/* @note: Due to different systems, the data sources here are not the same,
		so the registration method is used here to give the data to the lamp effect generation module */
	effect_init.max_value = FREQ_MAX_NUM;
	if ((light_effect_init(&effect_init)) != STD_E_OK)
	{
		return STD_E_NOT_OK;
	}

	UNUSED(light_effect_unregister_all());
	UNUSED(light_effect_register(&g_vm_register_msg1));
	g_vm_register_msg1.mute_cb = mute_detect_cb;
	g_vm_register_msg1.light_output_cb = light_output;

	g_vm_register_msg1.design_major_use = 2;
	light_effect_calibration_all();

	// to start a task to run function : music_rhythm_app_task!!!
	// task_register_periodic_func(TASK_PRIO_HIGH, TIME_10_MS, music_rhythm_app_task);
	OS_TASK_CREATE(music_rhythm_app_task, 10, 10, 10);
	return STD_E_OK;
}

/**
 * @brief  音乐律动应用主任务
 *         从LIN总线获取频率数据，调用音乐解析算法解析频谱，
 *         偶数周期调用灯效算法生成灯光效果
 * @param  无
 * @retval 无
 */
void music_rhythm_app_task(void)
{
	uint8 freq[8u] = {0};
	static uint32 g_sys_run_cnt = 0u;
	if (l_flg_tst_LI0_ALE_MSG_flag())
	{
		l_flg_clr_LI0_ALE_MSG_flag();
		freq_singal_update_handle(freq, 8);
		set_freq_data(freq, 8u);
	}

	(void)parse_music_task();
	if (g_sys_run_cnt % 2u == 0u)
	{
		// generate light effect
		UNUSED(light_effect_task());
	}
}
