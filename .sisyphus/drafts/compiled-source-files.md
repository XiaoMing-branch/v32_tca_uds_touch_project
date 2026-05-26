# TCA 项目编译源文件清单

> 基于 KEIL uvprojx 项目文件 + Objects/.d 依赖文件逆向提取
> 
> **项目根目录**: `D:\Users\18065\Desktop\SJM\V32\TCA\tca_uds_touch_project`

---

## 目录

- [1. App 固件（doorctrl_lin）](#1-app-固件doorctrl_lin)
- [2. Boot 固件（bootloader_seres）](#2-boot-固件bootloader_seres)
- [3. 编译依赖库（预构建）](#3-编译依赖库预构建)
- [4. 总结统计](#4-总结统计)

---

## 1. App 固件（doorctrl_lin）

**项目文件**: `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/doorctrl_lin.uvprojx`  
**目标芯片**: TCAE10 (ARM Cortex-M0+)  
**输出 Objects**: `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/Objects/`

**路径解析基准**: `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/`  
- `..\..\..\..\..\` → 上5级到 `app\`  
- `.\` → 项目目录自身

### 1.1 汇编文件

| .o 文件 | 源文件路径 |
|---|---|
| `startup_tcae10.o` | `app/device/common/src/startup_tcae10.s` |

### 1.2 C 源文件（52个）

> 按项目分组排列

#### app 组（项目本地源码）

| .o 文件 | 源文件路径 |
|---|---|
| `app.o` | `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/Source/app.c` |
| `custom_diagnosticiii.o` | `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/Source/custom_diagnosticIII.c` |
| `touch_config.o` | `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/Source/touch_config.c` |
| `lin_cfg.o` | `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/lin_cfg/lin_cfg.c` |
| `lin_frame.o` | `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/Source/lin_frame.c` |

#### driver 组（TCAE10 LL驱动）

| .o 文件 | 源文件路径 |
|---|---|
| `tcae10_ll_adc.o` | `app/driver/src/tcae10_ll_adc.c` |
| `tcae10_ll_captouch.o` | `app/driver/src/tcae10_ll_captouch.c` |
| `tcae10_ll_cortex.o` | `app/driver/src/tcae10_ll_cortex.c` |
| `tcae10_ll_delay.o` | `app/driver/src/tcae10_ll_delay.c` |
| `tcae10_ll_flash.o` | `app/driver/src/tcae10_ll_flash.c` |
| `tcae10_ll_gpio.o` | `app/driver/src/tcae10_ll_gpio.c` |
| `tcae10_ll_lpm.o` | `app/driver/src/tcae10_ll_lpm.c` |
| `tcae10_ll_pwm.o` | `app/driver/src/tcae10_ll_pwm.c` |
| `tcae10_ll_sci.o` | `app/driver/src/tcae10_ll_sci.c` |
| `tcae10_ll_sys.o` | `app/driver/src/tcae10_ll_sys.c` |
| `tcae10_ll_timer.o` | `app/driver/src/tcae10_ll_timer.c` |
| `tcae10_ll_uart.o` | `app/driver/src/tcae10_ll_uart.c` |
| `tcae10_ll_wdg.o` | `app/driver/src/tcae10_ll_wdg.c` |

#### midware 组（通用中间件）

| .o 文件 | 源文件路径 |
|---|---|
| `tc_halt.o` | `app/midware/common/tc_halt.c` |
| `misc.o` | `app/midware/common/misc.c` |
| `lin_task.o` | `app/midware/task_manager/lin_task.c` |
| `tc_log.o` | `app/midware/tclog/tc_log.c` |
| `store_manager.o` | `app/midware/store_manager/store_manager.c` |

#### lin_manager 组（LIN诊断协议栈）

| .o 文件 | 源文件路径 |
|---|---|
| `diagnosticiii.o` | `app/midware/lin_manager/diagnosticIII.c` |
| `lin_process.o` | `app/midware/lin_manager/lin_process.c` |
| `lin_wakeup.o` | `app/midware/lin_manager/lin_wakeup.c` |

#### diag_sid 组（UDS SID处理器，14个）

| .o 文件 | 源文件路径 |
|---|---|
| `sid_0x11.o` | `app/midware/lin_manager/diag_sid/sid_0x11.c` |
| `sid_0x2e.o` | `app/midware/lin_manager/diag_sid/sid_0x2e.c` |
| `sid_0x2f.o` | `app/midware/lin_manager/diag_sid/sid_0x2f.c` |
| `sid_0x32.o` | `app/midware/lin_manager/diag_sid/sid_0x32.c` |
| `sid_0x3e.o` | `app/midware/lin_manager/diag_sid/sid_0x3e.c` |
| `sid_0xb0.o` | `app/midware/lin_manager/diag_sid/sid_0xb0.c` |
| `sid_0xb1.o` | `app/midware/lin_manager/diag_sid/sid_0xb1.c` |
| `sid_0xb2.o` | `app/midware/lin_manager/diag_sid/sid_0xb2.c` |
| `sid_0xb3.o` | `app/midware/lin_manager/diag_sid/sid_0xb3.c` |
| `sid_0xb5.o` | `app/midware/lin_manager/diag_sid/sid_0xb5.c` |
| `sid_0xb6.o` | `app/midware/lin_manager/diag_sid/sid_0xb6.c` |
| `sid_0xb7.o` | `app/midware/lin_manager/diag_sid/sid_0xb7.c` |
| `sid_0xbc.o` | `app/midware/lin_manager/diag_sid/sid_0xbc.c` |
| `sid_0xbd.o` | `app/midware/lin_manager/diag_sid/sid_0xbd.c` |

#### touch 组（电容触摸中间件）

| .o 文件 | 源文件路径 |
|---|---|
| `touch_halnode.o` | `app/midware/touch/touch_halnode.c` |
| `si_touch_port.o` | `app/midware/touch/si_touch_port.c` |
| `touch_tool.o` | `app/midware/touch/touch_tool.c` |
| `touch_haldispatch.o` | `app/midware/touch/touch_haldispatch.c` |

#### pal 组（PAL抽象层）

| .o 文件 | 源文件路径 |
|---|---|
| `pal_lin_comm.o` | `app/platform/pal/pal_lin/pal_lin_comm.c` |
| `pal_store.o` | `app/platform/pal/pal_store/pal_store.c` |
| `pal_pmu.o` | `app/platform/pal/pal_pmu/pal_pmu.c` |
| `utilities.o` | `app/platform/pal/utilities/utilities.c` |

#### os 组（LiteTask RTOS）

| .o 文件 | 源文件路径 |
|---|---|
| `tc.o` | `app/os/litetask/tc.c` |
| `tc_port.o` | `app/os/litetask/tc_port.c` |

#### device 组

| .o 文件 | 源文件路径 |
|---|---|
| `system_tcae10.o` | `app/device/common/src/system_tcae10.c` |

### 1.3 App 文件汇总

| 类别 | 数量 |
|---|---|
| 汇编(.s) | 1 |
| C源文件(.c) | 51 |
| **总计** | **52** |

---

## 2. Boot 固件（bootloader_seres）

**项目文件**: `boot/examples/tcpl03x/demo_apps/bootloader_seres/bootloader.uvprojx`  
**目标芯片**: TCPL03X (ARM Cortex-M0+)  
**输出 Objects**: `boot/examples/tcpl03x/demo_apps/bootloader_seres/Objects/`

**路径解析基准**: `boot/examples/tcpl03x/demo_apps/bootloader_seres/`  
- `..\..\..\..\` → 上4级到 `boot\`  
- `.\` → 项目目录自身

### 2.1 汇编文件

| .o 文件 | 源文件路径 |
|---|---|
| `startup_tcpl03x.o` | `boot/platform/devices/tcpl03x/armcc/startup_tcpl03x.s` |

### 2.2 C 源文件（20个）

#### source 组（项目本地源码，5个）

| .o 文件 | 源文件路径 |
|---|---|
| `app.o` | `boot/examples/tcpl03x/demo_apps/bootloader_seres/source/app.c` |
| `aes_cmac.o` | `boot/examples/tcpl03x/demo_apps/bootloader_seres/source/aes_cmac.c` |
| `dfu_uds_manager.o` | `boot/examples/tcpl03x/demo_apps/bootloader_seres/source/dfu_uds_manager.c` |
| `tcpl03x_ll_flash_cus.o` | `boot/examples/tcpl03x/demo_apps/bootloader_seres/source/tcpl03x_ll_flash_cus.c` |
| `pal_lin_tl_slv_cus.o` | `boot/examples/tcpl03x/demo_apps/bootloader_seres/source/pal_lin_tl_slv_cus.c` |

#### middleware 组（3个）

| .o 文件 | 源文件路径 |
|---|---|
| `logging.o` | `boot/middleware/log_manager/logging.c` |
| `tc_printf.o` | `boot/middleware/log_manager/tc_printf.c` |

> `dfu_uds_manager.c` 分组在 middleware 但在 `source/` 目录下

#### platform/device 组（1个）

| .o 文件 | 源文件路径 |
|---|---|
| `system_tcpl03x.o` | `boot/platform/devices/tcpl03x/system_tcpl03x.c` |

#### platform/driver 组（5个TCPL03X LL驱动）

| .o 文件 | 源文件路径 |
|---|---|
| `tcpl03x_ll_cortex.o` | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_cortex.c` |
| `tcpl03x_ll_gpio.o` | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_gpio.c` |
| `tcpl03x_ll_sci.o` | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sci.c` |
| `tcpl03x_ll_sys.o` | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sys.c` |
| `tcpl03x_ll_wdg.o` | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_wdg.c` |

#### platform/pal 组（7个PAL模块）

| .o 文件 | 源文件路径 |
|---|---|
| `pal_lin_comm.o` | `boot/platform/pal/pal_lin/pal_lin_comm.c` |
| `pal_log.o` | `boot/platform/pal/pal_log/pal_log.c` |
| `pal_store.o` | `boot/platform/pal/pal_store/pal_store.c` |
| `pal_systick.o` | `boot/platform/pal/pal_systick/pal_systick.c` |
| `pal_wdg.o` | `boot/platform/pal/pal_wdg/pal_wdg.c` |
| `utilities.o` | `boot/platform/pal/utilities/utilities.c` |

### 2.3 未编译的源码（项目目录中存在但不在工程中）

| 文件 | 说明 |
|---|---|
| `boot/examples/.../source/flash_drv.c` | 存在源码但未被 uvprojx 引用，无对应 .o |

### 2.4 Boot 文件汇总

| 类别 | 数量 |
|---|---|
| 汇编(.s) | 1 |
| C源文件(.c) | 20 |
| **总计** | **21** |

---

## 3. 编译依赖库（预构建）

App 和 Boot 项目均引用了预编译库文件（`FileType=4`，非源码）：

| 固件 | 库文件 |
|---|---|
| **App** | `silib.lib` — 电容触摸算法库 |
| **App** | `linlib_seres.lib` — LIN协议栈库（赛力斯定制） |
| **Boot** | `linlib_seres.lib` — LIN协议栈库 |

这些库的路径定义在 uvprojx 的 `<IncludePath>` 或通过 `ArmAdsPath` 环境变量搜索，不作为源码编译。

---

## 4. 总结统计

| 固件 | 汇编(.s) | C(.c) | 总计 | 预构建库 |
|---|---|---|---|---|
| **App (doorctrl_lin)** | 1 | 51 | **52** | silib.lib + linlib_seres.lib |
| **Boot (bootloader_seres)** | 1 | 20 | **21** | linlib_seres.lib |
| **合计** | 2 | 71 | **73** | — |

### 结构对比

```
Project Root/
├── app/                              # App 固件
│   ├── device/common/src/            #  启动+系统文件 (2)
│   ├── driver/src/                   #  LL驱动层 (13)
│   ├── midware/                      #  中间件层
│   │   ├── common/                   #   通用 (2)
│   │   ├── task_manager/             #   任务管理 (1)
│   │   ├── tclog/                    #   日志 (1)
│   │   ├── store_manager/            #   存储管理 (1)
│   │   ├── lin_manager/              #   LIN诊断核心 (3)
│   │   │   └── diag_sid/             #   SID处理器 (14)
│   │   └── touch/                    #   电容触摸 (4)
│   ├── platform/pal/                 #  PAL抽象层 (4)
│   └── os/litetask/                  #  RTOS内核 (2)
│   └── projects/.../                 #  项目本地源码 (5)
│
├── boot/                             # Boot 固件
│   ├── platform/devices/tcpl03x/     #  启动+系统文件 (2)
│   ├── platform/drivers/             #  LL驱动层 (5)
│   ├── platform/pal/                 #  PAL抽象层 (6)
│   ├── middleware/log_manager/        #  日志管理 (2)
│   └── examples/.../source/          #  项目源码 (5)
```

### 路径解析规则

| 项目 | 基准目录 | 上级相对路径解析 |
|---|---|---|
| App | `app/projects/TCAE10/Keil/customer/doorctrl_m9_duotaiji/` | `..\..\..\..\..\` → `app\` |
| Boot | `boot/examples/tcpl03x/demo_apps/bootloader_seres/` | `..\..\..\..\` → `boot\` |

---

> **数据来源**: 
> - `.uvprojx` 项目XML中的 `<Files>` → `<Source>` → `<FilePath>` 条目
> - `Objects/*.d` 依赖文件中的 `.o → .c/.s` 映射
> - `Objects/*.o` 文件列表验证
