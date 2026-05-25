/**
 *********************************************************
 * @brief   LIN从节点位置检测(SNPD)模块源文件。
 *          实现自动寻址(AA)、ADC裸数据输出、NAD配置、电流阈值管理及SNPD状态机。
 *
 * @file    lin_snpd.c
 * @author  AE/FAE team
 * @date    2024.01.01
 *********************************************************
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
 *********************************************************/

/**
 * @brief  输出ADC裸数据，用于自动寻址(AA)过程中记录各从节点的ADC原始值
 * @param  org_nad - 原始节点地址
 * @param  new_nad - 自动分配的新节点地址
 * @retval 无
 */
void lin_snpd_raw_adc_out(uint8_t org_nad, uint8_t new_nad)
{
#if (SNPD_TEST_MODE == SNPD_TEST_MODE_PRINT)
    uint16_t raw_code[20] = {0};
    uint16_t length = pal_lin_aa_raw_code_get(raw_code, 4);  /* 获取ADC原始码值 */

    for (uint16_t i = 1; i < length; i++)
    {
        LOG_SPND("nad = %d = %08x\r\n", new_nad, raw_code[i]);  /* 打印ADC值到日志 */
    }

#elif (SNPD_TEST_MODE == SNPD_TEST_MODE_LIN)
    uint8_t count = 0;
    count = new_nad % 0x10;  /* 根据新NAD计算数组索引 */

    if (count > AA_SLAVE_NUM - 1)
    {
        count = 15;  /* 索引越界时固定使用最后一个槽位 */
    }

    adc_raw_data[count].org_nad = org_nad;  /* 记录原始NAD */
    adc_raw_data[count].new_nad = new_nad;  /* 记录新NAD */

    for (uint8_t i = 0; i < 5; i++)
    {
        adc_raw_data[count].adc[i] = 0;  /* 清零ADC缓冲区 */
    }

    uint8_t raw_len = pal_lin_aa_raw_code_get(adc_raw_data[count].adc, 5);  /* 获取ADC原始码值 */

    for (uint8_t i = raw_len; i < 5; i++)
    {
        adc_raw_data[count].adc[i] = 11;  /* 未使用通道填充默认值 */
    }

#else
    (void) org_nad;  /* 空模式：抑制未使用参数警告 */
    (void) new_nad;
#endif
}

/**
 * @brief  进入SNPD自动寻址模式，配置电流阈值并初始化状态
 * @note   通过回调函数控制LED指示，读取系统配置中的电流阈值并做字节序转换，
 *         清零AA状态后调用底层AA进入函数
 * @retval 无
 */
static void lin_snpd_enter(void)
{
#if CFG_SUPPORT_LIN_SNPD && CFG_SUPPORT_LIN_SNPD_LED

    if (NULL != lin_snpd_ctx->enter_func)
    {
        lin_snpd_ctx->enter_func();
    }

#endif
    uint16_t aa_cur_th[4];
    aa_cur_th[0] = ((g_sys_cfgs.cur_th_st12 & 0xFF00) >> 8) | ((g_sys_cfgs.cur_th_st12 & 0xFF) << 8);
    aa_cur_th[1] = ((g_sys_cfgs.cur_th_st12 & 0xFF000000) >> 24) | ((g_sys_cfgs.cur_th_st12 & 0xFF0000) >> 8);
    aa_cur_th[2] = ((g_sys_cfgs.cur_th_st34 & 0xFF00) >> 8) | ((g_sys_cfgs.cur_th_st34 & 0xFF) << 8);
    aa_cur_th[3] = ((g_sys_cfgs.cur_th_st34 & 0xFF000000) >> 24) | ((g_sys_cfgs.cur_th_st34 & 0xFF0000) >> 8);

    /* aa and colormix mutex */
//    meas_pn_sample_suspend(LED_CHANNLE_0);
    memset(lin_snpd_ctx->status, 0, LIN_AA_STATUS_MAX);  /* 清零AA状态数组 */
#if (SNPD_TEST_MODE == SNPD_TEST_MODE_LIN)
    memset(adc_raw_data, 0, AA_SLAVE_NUM * sizeof(struct aa_adc_data));
#endif
    pal_lin_aa_enter(aa_cur_th);

}

/**
 * @brief  退出SNPD自动寻址模式
 * @note   调用底层AA退出函数，通过回调函数恢复LED指示
 * @retval 无
 */
static void lin_snpd_exit(void)
{
    pal_lin_aa_exit();
    /* aa and colormix mutex */
//    meas_pn_sample_resume(LED_CHANNLE_0);

#if CFG_SUPPORT_LIN_SNPD && CFG_SUPPORT_LIN_SNPD_LED

    if (NULL != lin_snpd_ctx->exit_func)
    {
        lin_snpd_ctx->exit_func();
    }

#endif
}

/**
 * @brief  选择并确认当前NAD，完成自动寻址选择步骤
 * @note   调用底层AA就绪函数，表示从节点已准备好接受新地址
 * @retval 无
 */
static void lin_snpd_select(void)
{
    pal_lin_aa_ready();
}

/**
 * @brief  获取SNPD模块指定状态值
 * @param  type - 状态类型（状态/NAD/步骤/选择标志/裸码标志）
 * @retval 对应状态值，若type无效则返回0
 */
uint8_t lin_snpd_status_get(lin_aa_status_e type)
{
    if (type >= LIN_AA_STATUS_MAX)
    {
        return 0;
    }

    return (lin_snpd_ctx->status[type]);
}

#if (CFG_SUPPORT_LIN_SNPD_TIMEOUT && CFG_SUPPORT_LIN_SNPD)
/**
 * @brief  SNPD超时检测，防止AA过程卡死
 * @note   非IDLE状态下超时计数递增，超过LIN_SNPD_TIMEOUT则强制退出AA并回到IDLE
 * @retval 无
 */
static void lin_snpd_timeout_check(void)
{
    if (LIN_AA_STATE_IDLE != lin_snpd_status_get(LIN_AA_STATUS_STATE))
    {
        if (lin_snpd_ctx->timeout >= LIN_SNPD_TIMEOUT)
        {
            lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_IDLE);
            lin_snpd_exit();
        }
        else
        {
            lin_snpd_ctx->timeout++;
        }
    }
    else
    {
        lin_snpd_ctx->timeout = 0;
    }
}
#endif

/**
 * @brief  设置SNPD模块指定状态值
 * @param  type - 状态类型
 * @param  value - 要设置的状态值
 * @retval 无
 * @note   若ctx为空或type无效则直接返回
 */
void lin_snpd_status_set(lin_aa_status_e type, uint8_t value)
{
    if (NULL == lin_snpd_ctx)
    {
        return;
    }

    if (type < LIN_AA_STATUS_MAX)
    {
        lin_snpd_ctx->status[type] = value;
    }
}

#if CFG_SUPPORT_LIN_SNPD
/**
 * @brief  SNPD主状态机处理函数，需在主循环中周期性调用
 * @note   状态机流转：IDLE -> ENTER -> SAVE -> EXIT -> IDLE
 *         - ENTER状态：启动AA并设置电流阈值
 *         - SAVE状态：将新NAD保存到Flash
 *         - EXIT状态：退出AA模式
 *         同时处理ADC裸数据输出和超时检测
 * @retval 无
 */
void lin_snpd_process_handle(void)
{
    if (NULL == lin_snpd_ctx)
    {
        return;
    }

#if (CFG_SUPPORT_LIN_SNPD_TIMEOUT && CFG_SUPPORT_LIN_SNPD)
    /* timeout check */
    lin_snpd_timeout_check();
#endif

    uint8_t aa_step = lin_snpd_status_get(LIN_AA_STATUS_STEP);

    if (!aa_step)
    {
        return;
    }

    if (LIN_SNPD_CMD_ENTER == aa_step || LIN_SNPD_CMD_EXIT == aa_step)
    {
        if (LIN_SNPD_CMD_EXIT == aa_step)
        {
            lin_snpd_exit();
        }

        lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_IDLE);
    }

    switch (lin_snpd_status_get(LIN_AA_STATUS_STATE))
    {
        case LIN_AA_STATE_IDLE:
            if (LIN_SNPD_CMD_ENTER == aa_step)
            {
                lin_snpd_enter();  /* 进入AA模式：设置电流阈值、清零状态 */
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_ENTER);  /* 切换到ENTER状态 */
            }

            break;

        case LIN_AA_STATE_ENTER:
            /* NAD配置完成，确认并进入SAVE步骤 */
            if (LIN_SNPD_CMD_NAD == aa_step && lin_snpd_status_get(LIN_AA_STATUS_SELECT))
            {
                lin_configured_NAD = lin_snpd_status_get(LIN_AA_STATUS_NAD);  /* 暂存配置的NAD */
                lin_snpd_select();  /* 确认NAD选择 */
                lin_snpd_status_set(LIN_AA_STATUS_SELECT, 0);  /* 清除选择标志 */
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_SAVE);  /* 切换到SAVE状态 */
            }

            /* 有ADC裸数据需要输出时处理 */
            if (lin_snpd_status_get(LIN_AA_STATUS_RAW_CODE))
            {
                lin_snpd_status_set(LIN_AA_STATUS_RAW_CODE, false);
                lin_snpd_raw_adc_out((uint8_t)g_sys_cfgs.org_nad, lin_snpd_status_get(LIN_AA_STATUS_NAD));
            }

            break;

        case LIN_AA_STATE_SAVE:
            if (LIN_SNPD_CMD_SAVE == aa_step)
            {
                /* 将new nad 保存到flash中to do */
                lin_snpd_nad_write(lin_configured_NAD);  /* 将NAD写入Flash持久化 */
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_EXIT);  /* 切换到EXIT状态 */
            }

            break;

        case LIN_AA_STATE_EXIT:
            if (LIN_SNPD_CMD_EXIT == aa_step)
            {
                lin_snpd_exit();  /* 退出AA模式：调用底层退出和恢复回调 */
                lin_snpd_status_set(LIN_AA_STATUS_STATE, LIN_AA_STATE_IDLE);  /* 回到IDLE状态 */
            }

            break;

        default:
            break;
    }

    lin_snpd_status_set(LIN_AA_STATUS_STEP, 0);
}
#endif

/**
 * @brief  初始化SNPD模块，注册上下文
 * @param  ctx - SNPD上下文结构体指针（包含超时计数、状态数组、进入/退出回调）
 * @retval 无
 */
void lin_snpd_init(lin_snpd_context_t *ctx)
{
    lin_snpd_ctx = ctx;
}

/**
 * @brief  DFU模式下自动配置节点地址
 * @note   使用定时器计数值的低4位作为新NAD，写入Flash
 * @retval 无
 */
void autoaddress_config_for_dfu(void)
{
    uint32_t value = ll_timer_counter_get();
    lin_configured_NAD = value & 0x0F;
    lin_snpd_nad_write(lin_configured_NAD);
}

/**
 * @brief  读取当前节点地址(NAD)
 * @param  nad - 输出参数，用于存储读取到的NAD值
 * @retval 无
 */
void lin_snpd_nad_read(uint8_t *nad)
{
    *nad = (uint8_t)g_sys_cfgs.nad;
}

#ifdef CFG_LIN_CONFORM_TEST
/**
 * @brief  读取LIN帧ID配置（仅一致性测试使用）
 * @note   当前实现为空，预留用于读取帧ID配置到RAM
 * @retval 无
 */
void lin_snpd_id_read(void)
{
//    memcpy(lin_configuration_RAM, (*g_sys_cfgs.frame_id_cfg), SYSTEM_ID_CFG_SIZE);
}
#endif

/**
 * @brief  写入节点地址(NAD)并保存到Flash
 * @param  nad - 要写入的新节点地址
 * @retval 无
 * @note   同时更新内存中的系统配置结构体并持久化到存储
 */
void lin_snpd_nad_write(uint8_t nad)
{
    g_sys_cfgs.nad = nad;
    store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
}

/**
 * @brief  读取电流阈值配置
 * @param  st12 - 输出参数，通道1-2的电流阈值
 * @param  st34 - 输出参数，通道3-4的电流阈值
 * @retval 无
 */
void lin_snpd_cur_th_get(uint32_t *st12, uint32_t *st34)
{
    *st12 = g_sys_cfgs.cur_th_st12;
    *st34 = g_sys_cfgs.cur_th_st34;
}

/**
 * @brief  设置电流阈值配置并保存到Flash
 * @param  st12 - 通道1-2的电流阈值指针
 * @param  st34 - 通道3-4的电流阈值指针
 * @retval 无
 * @note   更新内存配置后持久化到存储设备
 */
void lin_snpd_cur_th_set(uint32_t *st12, uint32_t *st34)
{
    g_sys_cfgs.cur_th_st12 = *st12;
    g_sys_cfgs.cur_th_st34 = *st34;

    store_system_data_set(SYSTEM_CFG_PARAM, (uint8_t *)&g_sys_cfgs, SYSTEM_CFG_SIZE);
}
