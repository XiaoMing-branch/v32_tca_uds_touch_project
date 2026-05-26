# 编译源文件注释补充与维护计划

## TL;DR

> **Quick Summary**: 基于编译源文件清单，对尚未完整注释的约25个文件补充 Doxygen 函数头注释（@brief/@param/@retval格式），添加复杂逻辑内联注释，完善UDS变量/宏的精确定义注释，并对已有注释进行核验与修复。
>
> **Deliverables**:
> - App/Boot共约25个文件的函数级 @brief 注释补充
> - UDS 相关变量、宏、枚举的精确注释
> - 复杂函数实现部分的内联注释
> - 已有注释的核验与一致性维护
>
> **Estimated Effort**: Large
> **Parallel Execution**: YES - 6 waves
> **Critical Path**: 已有注释核验 → 各文件注释补充 → UDS精确注释 → 复杂逻辑注释 → 最终验证

---

## Context

### 原始请求
根据编译路径（.o → .c 映射）确定的实际参与编译的文件清单，对其中尚未完整注释的文件：
1. 补充函数级Doxygen注释
2. 查看并修正现有注释疏漏
3. 不改代码，只加注释
4. UDS变量/定义增加精确注释
5. 难理解的函数实现部分加内联注释
6. 工作内容核验

### 编译文件清单
源文件清单见 `.sisyphus/drafts/compiled-source-files.md`，基于 KEIL uvprojx + Objects/.d 文件提取。

### 当前注释状态总览

| 状态 | 含义 | 文件数 |
|---|---|---|
| ✅ DONE | ≥5个@brief，基本完整 | 约32个 |
| ⚠️ PARTIAL | 1~4个@brief，仅文件头 | 约21个 |
| ❌ NONE | 0个@brief | 1个 |

### 需要补充的文件清单

**App 固件（11个文件）:**

| 优先级 | 文件 | 行数 | 当前状态 | 说明 |
|---|---|---|---|---|
| H | `app/projects/.../Source/touch_config.c` | 1440 | ❌ NONE | 触摸配置，大量宏和结构体 |
| H | `app/midware/touch/touch_haldispatch.c` | 1575 | ⚠️ 1个 | 触摸调度核心，复杂状态机 |
| H | `app/projects/.../Source/custom_diagnosticIII.c` | 1722 | ⚠️ 2个 | UDS诊断定制处理器 |
| H | `app/midware/touch/touch_halnode.c` | 574 | ⚠️ 1个 | 触摸节点处理 |
| M | `app/projects/.../Source/app.c` | 402 | ⚠️ 1个 | 主入口，任务创建 |
| M | `app/projects/.../Source/lin_frame.c` | 301 | ⚠️ 1个 | LIN帧处理 |
| M | `app/projects/.../lin_cfg/lin_cfg.c` | 313 | ⚠️ 1个 | LIN配置 |
| M | `app/platform/pal/pal_pmu/pal_pmu.c` | 135 | ⚠️ 4个 | 电源管理 |
| M | `app/midware/touch/si_touch_port.c` | 317 | ⚠️ 1个 | 触摸平台接口 |
| M | `app/midware/touch/touch_tool.c` | 162 | ⚠️ 1个 | 触摸工具函数 |
| L | `app/device/common/src/system_tcae10.c` | 381 | ⚠️ 1个 | 系统初始化 |

**Boot 固件（11个文件）:**

| 优先级 | 文件 | 行数 | 当前状态 | 说明 |
|---|---|---|---|---|
| H | `boot/.../source/aes_cmac.c` | 1024 | ⚠️ 1个 | AES-CMAC安全认证 |
| H | `boot/.../source/tcpl03x_ll_flash_cus.c` | 589 | ⚠️ 1个 | Flash驱动（定制版） |
| M | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sci.c` | 1179 | ⚠️ 1个 | SCI/LIN驱动 |
| M | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_gpio.c` | 335 | ⚠️ 1个 | GPIO驱动 |
| M | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sys.c` | 227 | ⚠️ 1个 | 系统驱动 |
| M | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_wdg.c` | 155 | ⚠️ 1个 | 看门狗驱动 |
| M | `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_cortex.c` | 73 | ⚠️ 1个 | Cortex核心驱动 |
| L | `boot/platform/devices/tcpl03x/system_tcpl03x.c` | 370 | ⚠️ 1个 | 系统初始化 |
| L | `boot/.../source/app.c` | 57 | ⚠️ 1个 | Boot主入口 |
| L | `boot/middleware/log_manager/logging.c` | 52 | ⚠️ 3个 | 日志管理 |
| L | `boot/platform/pal/pal_log/pal_log.c` | 58 | ⚠️ 4个 | PAL日志 |
| L | `boot/platform/pal/pal_wdg/pal_wdg.c` | 75 | ⚠️ 4个 | PAL看门狗 |

**核验目标（已注释但需检查一致性和质量）:**

| 检查项 | 文件范围 |
|---|---|
| App LL驱动层 | 15个文件，检查注释质量 |
| App middleware | 8个文件（lin_manager, diag_sid, store_manager, tclog） |
| App PAL层 | 3个文件（pal_lin_comm, pal_store, utilities） |
| App RTOS(litetask) | 2个文件 |
| Boot PAL层 | 已注释的文件 |
| Boot DFU管理器 | dfu_uds_manager.c（已验证41个@brief） |

### UDS精确注释范围
- `custom_diagnosticIII.c` — UDS诊断逻辑定制
- `app/midware/lin_manager/diagnosticIII.c` — 诊断核心NRC/会话、安全等级
- `*sid_0x*.c` — 各SID处理器（共14个），变量/宏/枚举
- `boot/.../source/dfu_uds_manager.c` — DFU升级流程中的UDS变量
- `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sci.c` — LIN UART寄存器

---

## Work Objectives

### Core Objective
为编译清单中所有文件补充完善的函数头注释和内联注释，确保UDS相关变量/宏/枚举有精确注释，并进行全面核验。

### Concrete Deliverables
- App 固件 11 个文件完成函数级 @brief 注释
- Boot 固件 12 个文件完成函数级 @brief 注释
- 所有 UDS 变量/宏/枚举增加精确注释
- 复杂函数实现部分增加内联注释
- 已有注释的核验与修复

### Must Have
- Doxygen @brief 格式（非 \brief），中文描述
- 每个 @param 独立一行，不用分号合并
- `@note` 说明重要实现细节
- `@retval` 注明每个返回值含义
- UDS 相关变量/宏必须有精确注释（包括 NRC 码、会话状态、安全等级等）
- 不改任何代码逻辑

### Must NOT Have
- 不修改代码逻辑或结构
- 不删除 PRQA 注释
- 不创建重复的函数声明
- 不添加多余的格式化注释（如过长的分隔线）

---

## Verification Strategy

> **ZERO HUMAN INTERVENTION** - ALL verification is agent-executed.

### QA Policy
- 每个文件注释完毕后，通过 `Select-String -Pattern "@brief"` 检查覆盖率
- 检查无残留 `\\brief` 旧样式
- 检查无重复函数声明
- 对于 UDS 相关文件，抽查关键变量/宏的注释精确度
- 最终验证：对所有已注释文件进行抽样，确认格式一致性

### Verification Commands
```powershell
# 检查旧样式残留
Select-String -Path "<file>" -Pattern "\\\\brief"

# 统计@brief数量
(Select-String -Path "<file>" -Pattern "@brief" | Measure-Object).Count

# 检查行数变化
(Get-Content "<file>" | Measure-Object).Count
```

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Touch层 + 设备初始化 - 独立并行):
├── T1: touch_config.c (1440行, ❌NONE)
├── T2: touch_haldispatch.c (1575行, ⚠️1个)
├── T3: touch_halnode.c (574行, ⚠️1个)
├── T4: si_touch_port.c (317行, ⚠️1个)
├── T5: touch_tool.c (162行, ⚠️1个)
├── T6: system_tcae10.c (381行, ⚠️1个)
└── T7: system_tcpl03x.c (370行, ⚠️1个)

Wave 2 (Boot驱动层 - 完全独立并行):
├── T8: tcpl03x_ll_sci.c (1179行, ⚠️1个)
├── T9: tcpl03x_ll_gpio.c (335行, ⚠️1个)
├── T10: tcpl03x_ll_sys.c (227行, ⚠️1个)
├── T11: tcpl03x_ll_wdg.c (155行, ⚠️1个)
├── T12: tcpl03x_ll_cortex.c (73行, ⚠️1个)
├── T13: tcpl03x_ll_flash_cus.c (589行, ⚠️1个)
└── T14: aes_cmac.c (1024行, ⚠️1个)

Wave 3 (App项目本地源码 - 独立并行):
├── T15: custom_diagnosticIII.c (1722行, ⚠️2个)
├── T16: app.c (402行, ⚠️1个)
├── T17: lin_frame.c (301行, ⚠️1个)
├── T18: lin_cfg.c (313行, ⚠️1个)
├── T19: pal_pmu.c (135行, ⚠️4个)

Wave 4 (Boot剩余文件 - 独立并行):
├── T20: boot/app.c (57行, ⚠️1个)
├── T21: logging.c (52行, ⚠️3个)
├── T22: pal_log.c (58行, ⚠️4个)
├── T23: pal_wdg.c (75行, ⚠️4个)

Wave 5 (UDS精确注释专项 - 跨文件并行):
├── T24: UDS变量/宏/枚举精确注释 (diag_sid目录14个文件)
├── T25: UDS诊断核心精确定义注释 (diagnosticIII.c, custom_diagnosticIII.c)
├── T26: DFU管理器UDS变量注释 (dfu_uds_manager.c)
└── T27: LIN SCI驱动寄存器注释 (pal_lin_comm.c, tcpl03x_ll_sci.c)

Wave 6 (内联注释 + 已有注释核验):
├── T28: 复杂函数实现内联注释(跨文件)
├── T29: 已有注释格式一致性核验(跨文件)
└── T30: PRQA注释保留检查

Wave FINAL:
├── F1: 计划合规审计 (oracle)
├── F2: 注释完整性最终验证(unspecified-high)
└── F3: UDS注释精确度专项审查(unspecified-high)
```

---

## TODOs

- [ ] 0. **初始化：创建类型映射模板文件**

  **What to do**:
  - 从 `app/driver/inc/` 和 `app/midware/lin_manager/inc/` 中提取UDS核心类型定义、枚举（NRC码、会话类型、安全等级）到一个注释参考模板
  - 目的是统一UDS相关注释的术语表述，确保同一概念在不同文件的注释中表述一致

  **Must NOT do**:
  - 不要修改任何源文件，仅创建参考模板

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (prep-only)
  - **Blocks**: T24, T25, T26
  - **Blocked By**: None

- [ ] 1. `app/projects/.../Source/touch_config.c` — 触摸配置注释

  **What to do**:
  - 1440行，当前0个@brief（❌ NONE）
  - 为所有函数添加Doxygen @brief注释
  - 为所有宏定义（触摸通道配置、阈值、时序等）添加精确注释
  - 为结构体成员添加逐字段注释

  **Must NOT do**: 不修改任何配置值或代码逻辑

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

- [ ] 2. `app/midware/touch/touch_haldispatch.c` — 触摸调度核心注释

  **What to do**:
  - 1575行，当前1个@brief（仅文件头）
  - 添加所有函数Doxygen注释
  - 触摸状态机转换逻辑添加内联注释
  - 通道扫描调度算法添加注释

  **Must NOT do**: 勿改触摸算法逻辑

  **Recommended Agent Profile**:
  - **Category**: `deep`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

- [ ] 3. `app/midware/touch/touch_halnode.c` — 触摸节点处理注释

  **What to do**:
  - 574行，当前1个@brief
  - 添加所有函数Doxygen注释
  - 节点状态管理逻辑添加内联注释

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

- [ ] 4. `app/midware/touch/si_touch_port.c` — 触摸平台接口注释

  **What to do**:
  - 317行，当前1个@brief
  - 添加所有函数Doxygen注释
  - I2C/SPI通信接口时序添加注释

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

- [ ] 5. `app/midware/touch/touch_tool.c` — 触摸工具函数注释

  **What to do**:
  - 162行，当前1个@brief
  - 添加所有函数Doxygen注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

- [ ] 6. `app/device/common/src/system_tcae10.c` — TCAE10系统初始化注释

  **What to do**:
  - 381行，当前1个@brief
  - 添加所有函数Doxygen注释
  - 时钟配置、中断向量设置等硬件初始化步骤添加注释

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

- [ ] 7. `boot/platform/devices/tcpl03x/system_tcpl03x.c` — TCPL03X系统初始化注释

  **What to do**:
  - 370行，当前1个@brief
  - 添加所有函数Doxygen注释
  - 时钟配置、PLL、系统时钟树添加注释

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

---

**Wave 2 Final**: After ALL Wave 2 tasks
- **Blocks**: Wave 3
- **Blocked By**: Wave 1

---

- [ ] 8. `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sci.c` — SCI/LIN驱动注释

  **What to do**:
  - 1179行，当前1个@brief（仅文件头）
  - 添加所有函数Doxygen注释
  - SCI寄存器操作（波特率配置、帧格式、中断控制）添加详细注释
  - LIN总线UART相关操作（同步间隔场、校验计算）添加注释

  **Must NOT do**: 不修改任何寄存器配置值

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2
  - **Blocks**: None
  - **Blocked By**: Wave 1

- [ ] 9. `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_gpio.c` — GPIO驱动注释

  **What to do**:
  - 335行，当前1个@brief
  - 添加所有函数Doxygen注释
  - GPIO配置模式（输入/输出/复用/中断）添加注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2
  - **Blocks**: None
  - **Blocked By**: Wave 1

- [ ] 10. `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sys.c` — 系统驱动注释

  **What to do**:
  - 227行，当前1个@brief
  - 添加所有函数Doxygen注释
  - 系统控制寄存器操作添加注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2
  - **Blocks**: None
  - **Blocked By**: Wave 1

- [ ] 11. `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_wdg.c` — 看门狗驱动注释

  **What to do**:
  - 155行，当前1个@brief
  - 添加所有函数Doxygen注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2
  - **Blocks**: None
  - **Blocked By**: Wave 1

- [ ] 12. `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_cortex.c` — Cortex核心驱动注释

  **What to do**:
  - 73行，当前1个@brief
  - 添加所有函数Doxygen注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2
  - **Blocks**: None
  - **Blocked By**: Wave 1

- [ ] 13. `boot/examples/tcpl03x/demo_apps/bootloader_seres/source/tcpl03x_ll_flash_cus.c` — Flash驱动定制注释

  **What to do**:
  - 589行，当前1个@brief
  - 添加所有函数Doxygen注释
  - Flash擦除/编程时序、加解密操作添加注释

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2
  - **Blocks**: None
  - **Blocked By**: Wave 1

- [ ] 14. `boot/examples/tcpl03x/demo_apps/bootloader_seres/source/aes_cmac.c` — AES-CMAC认证注释

  **What to do**:
  - 1024行，当前1个@brief（仅文件头）
  - 添加所有函数Doxygen注释
  - AES-CMAC算法步骤添加内联注释（子密钥生成、加密、MAC计算）
  - 安全认证流程中的密钥/种子使用添加精确注释

  **Recommended Agent Profile**:
  - **Category**: `deep`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2
  - **Blocks**: None
  - **Blocked By**: Wave 1

---

**Wave 3 Final**: After ALL Wave 3 tasks
- **Blocks**: Wave 4
- **Blocked By**: Wave 2

---

- [ ] 15. `app/projects/.../Source/custom_diagnosticIII.c` — UDS诊断定制注释

  **What to do**:
  - 1722行，当前2个@brief
  - 添加所有函数Doxygen注释
  - UDS诊断流程（会话切换、安全访问、DID读写、例程控制）添加详细注释
  - **UDS精确注释重点**: 所有NRC码、会话状态机、安全访问等级、DID标识符添加精确宏/枚举注释
  - 诊断请求分发逻辑添加内联注释
  - `uds_request_info` 结构体成员添加逐字段注释

  **Must NOT do**: 不修改任何诊断逻辑或NRC响应

  **Recommended Agent Profile**:
  - **Category**: `deep`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3
  - **Blocks**: T24, T25 (UDS precision pass partially)
  - **Blocked By**: Wave 2

- [ ] 16. `app/projects/.../Source/app.c` — App主入口注释

  **What to do**:
  - 402行，当前1个@brief
  - 添加所有函数Doxygen注释
  - 系统初始化流程、任务创建逻辑添加注释

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3
  - **Blocks**: None
  - **Blocked By**: Wave 2

- [ ] 17. `app/projects/.../Source/lin_frame.c` — LIN帧处理注释

  **What to do**:
  - 301行，当前1个@brief
  - 添加所有函数Doxygen注释
  - LIN帧结构（PID、数据字节、校验和）添加注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3
  - **Blocks**: None
  - **Blocked By**: Wave 2

- [ ] 18. `app/projects/.../lin_cfg/lin_cfg.c` — LIN配置注释

  **What to do**:
  - 313行，当前1个@brief
  - 添加所有函数Doxygen注释
  - LIN通信参数（波特率、帧ID映射、NAD分配）添加注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3
  - **Blocks**: None
  - **Blocked By**: Wave 2

- [ ] 19. `app/platform/pal/pal_pmu/pal_pmu.c` — PAL电源管理注释

  **What to do**:
  - 135行，当前4个@brief
  - 检查现有注释，补充缺失的函数注释
  - 睡眠模式切换、GPIO低功耗配置添加注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3
  - **Blocks**: None
  - **Blocked By**: Wave 2

---

**Wave 4 Final**: After ALL Wave 4 tasks
- **Blocks**: Wave 5
- **Blocked By**: Wave 3

---

- [ ] 20. `boot/examples/.../source/app.c` — Boot主入口注释

  **What to do**:
  - 57行，当前1个@brief
  - 添加主函数和初始化流程注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4
  - **Blocks**: None
  - **Blocked By**: Wave 3

- [ ] 21. `boot/middleware/log_manager/logging.c` — Boot日志管理注释

  **What to do**:
  - 52行，当前3个@brief
  - 补充缺失的函数注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4
  - **Blocks**: None
  - **Blocked By**: Wave 3

- [ ] 22. `boot/platform/pal/pal_log/pal_log.c` — PAL日志注释

  **What to do**:
  - 58行，当前4个@brief
  - 检查并补充注释

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4
  - **Blocks**: None
  - **Blocked By**: Wave 3

- [ ] 23. `boot/platform/pal/pal_wdg/pal_wdg.c` — PAL看门狗注释

  **What to do**:
  - 75行，当前4个@brief
  - 检查并补充注释（注意：之前的agent在此文件发现了重复函数声明，已修复，需验证）

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4
  - **Blocks**: None
  - **Blocked By**: Wave 3

---

**Wave 5 Final**: After ALL Wave 5 tasks
- **Blocks**: Wave 6
- **Blocked By**: Wave 4
- **Dependency**: T0 (类型映射模板)

---

- [ ] 24. **UDS变量/宏/枚举精确注释 — diag_sid目录14个文件**

  **What to do**:
  - 范围：`app/midware/lin_manager/diag_sid/sid_0x*.c` 共19个文件
  - 对以下内容添加/补充精确注释：
    - UDS服务ID宏（SID值 + 服务名称
    - NRC码枚举或宏定义（每个NRC的触发条件）
    - 会话状态变量（默认/扩展/编程会话切换条件）
    - 安全访问等级和种子/密钥相关变量
    - DID标识符（数据标识符含义和格式）
    - 子功能参数定义
  - 参考T0（类型映射模板）统一术语表述
  - 确保同一概念在不同SID文件中注释一致

  **Must NOT do**: 不修改UDS服务处理逻辑

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low` (with T0 reference)
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES (各SID文件独立)
  - **Parallel Group**: Wave 5
  - **Blocks**: None
  - **Blocked By**: T0, Wave 4

- [ ] 25. **UDS核心变量精确注释 — diagnosticIII.c**

  **What to do**:
  - 范围：`app/midware/lin_manager/diagnosticIII.c`, `app/projects/.../Source/custom_diagnosticIII.c`
  - 对以下核心变量添加精确注释：
    - `uds_request_info` 结构体每个成员
    - 会话超时计数器变量
    - 安全访问状态变量
    - P2/P2E超时参数
    - UDS请求/响应缓冲区成员
  - 参考T0模板统一术语

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES (与T24并行)
  - **Parallel Group**: Wave 5
  - **Blocks**: None
  - **Blocked By**: T0, Wave 4

- [ ] 26. **DFU管理器UDS变量注释 — dfu_uds_manager.c**

  **What to do**:
  - 范围：`boot/examples/.../source/dfu_uds_manager.c`（已验证41个@brief）
  - 对UDS相关变量添加精确注释：
    - `dfu_ctx` 结构体所有成员的操作码和状态
    - DFU流程状态机变量
    - 安全解锁相关变量（`seed_cmac_succ`, `unlock_failed_store_flag`等）
    - 各服务ID宏定义注释
  - 参考T0模板统一术语

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES (与T24、T25并行)
  - **Parallel Group**: Wave 5
  - **Blocks**: None
  - **Blocked By**: T0, Wave 4

- [ ] 27. **LIN SCI驱动寄存器注释 — 跨文件精确注释**

  **What to do**:
  - 范围：`app/platform/pal/pal_lin/pal_lin_comm.c`, `boot/platform/drivers/tcpl03x_ll_driver/tcpl03x_ll_sci.c`
  - 对以下内容添加精确注释：
    - LIN SCI寄存器配置位含义（波特率、帧格式、中断使能）
    - LIN总线状态寄存器枚举值
    - 同步间隔场/同步场/标识符场的处理逻辑
    - 校验计算相关的位移和掩码操作

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES (与T24-26并行)
  - **Parallel Group**: Wave 5
  - **Blocks**: None
  - **Blocked By**: Wave 4

---

**Wave 6 Final**: After ALL Wave 6 tasks
- **Blocks**: Final Verification Wave
- **Blocked By**: Wave 5

---

- [ ] 28. **复杂函数实现内联注释 — 跨文件补充**

  **What to do**:
  - 扫描所有已注释文件，对以下难以理解的地方添加行内注释（`/**< ... */` 或 `// ...`）：
    - 位运算操作（尤其是寄存器掩码计算）
    - 状态机转换条件
    - 数学计算逻辑（CRC/校验和/CMAC算法）
    - 条件编译分支（#ifdef/#endif的作用说明）
    - 时序相关逻辑（延时计算、超时检查）
    - 指针/回调函数使用（函数指针转换）
  - 重点检查：LL驱动层中的寄存器位操作、UDS诊断中的NRC判断链、AES-CMAC算法步骤

  **Must NOT do**: 不添加无意义的注释（如 `i++; // 递增i`）

  **Recommended Agent Profile**:
  - **Category**: `deep`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 6
  - **Blocks**: None
  - **Blocked By**: Wave 5

- [ ] 29. **已有注释格式一致性核验 — 跨文件检查**

  **What to do**:
  - 对已注释但尚未核验的文件进行格式一致性检查：
    - 确认使用 `@brief` 而非旧式 `\brief`（`Select-String -Pattern "\\\\brief"`）
    - 确认 `@param` 每行一个参数，无分号合并
    - 确认 `@retval` 对应实际返回类型
    - 确认无重复函数声明（检查上一个agent的遗留问题）
    - 确认 PRQA 注释未被意外删除
  - 重点检查：App LL 驱动层15个文件、App middleware 8个文件

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 6
  - **Blocks**: None
  - **Blocked By**: Wave 5

- [ ] 30. **汇编启动文件注释 — startup_tcae10.s + startup_tcpl03x.s**

  **What to do**:
  - 280行 + 汇编文件
  - 为汇编文件添加注释（`;` 格式）：
    - 中断向量表每个入口的功能说明
    - 堆栈初始化过程
    - 启动流程（进入main前的准备）
  - 使用 `; @brief` 格式保持一致性

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 6
  - **Blocks**: None
  - **Blocked By**: Wave 5

---

## Final Verification Wave

- [ ] F1. **计划合规审计** — `oracle`
  读取本计划并逐条核实。验证每个文件是否都已处理，输出合规率。

- [ ] F2. **注释完整性最终验证** — `unspecified-high`
  对所有目标文件执行：
  - `Select-String -Pattern "\\\\brief"` → 应为0结果
  - `Select-String -Pattern "@brief"` → 每个文件≥5
  - 随机抽样5个文件，人工检查注释质量（中文描述是否通顺，参数是否正确）
  - 检查 `pal_wdg.c` 和 `pal_systick.c` 确保之前发现的重复声明问题已永久修复

- [ ] F3. **UDS注释精确度专项审查** — `unspecified-high`
  审查以下文件的UDS注释精确度：
  - NRC码注释是否写明触发条件
  - DID注释是否写明数据格式和范围
  - 安全访问注释是否写明等级要求
  - 会话状态转换注释是否写明转换条件

---

## Commit Strategy

所有修改为纯注释，建议统一提交：
- **Message**: `docs: 补充编译源文件注释（Touch驱动/LL驱动/UDS变量/内联注释）`
- **Scope**: 约25个文件
- **Pre-commit**: `Select-String -Pattern "\\\\brief" -Path app,boot -Recurse -Include "*.c","*.s"`（确认0残留）

---

## Success Criteria

### Verification Commands
```powershell
# 1. 无旧样式残留
$old = Select-String -Path "app" -Recurse -Include "*.c","*.s" -Pattern "\\\\brief"
$old.Count  # 应为0
$old = Select-String -Path "boot" -Recurse -Include "*.c","*.s" -Pattern "\\\\brief"
$old.Count  # 应为0

# 2. 各文件 @brief 覆盖
Get-ChildItem -Path "app","boot" -Recurse -Include "*.c","*.s" | Where-Object {
  $c = (Select-String -Path $_ -Pattern "\\\\brief" | Measure-Object).Count
  $c -gt 0
} | ForEach-Object { Write-Host "$($_.Name): $c" }

# 3. 无重复函数声明
Select-String -Path "app","boot" -Recurse -Include "*.c" -Pattern "^void \w+\(|^static void \w+\(|^uint8_t \w+\(|^static uint8_t \w+\("
```

### Final Checklist
- [ ] 零旧样式 `\brief` 残留
- [ ] 所有编译文件有 ≥5 个 @brief 注释（小型文件除外）
- [ ] UDS变量/宏/枚举有精确注释
- [ ] 复杂函数实现有内联注释
- [ ] PRQA注释未被删除
- [ ] 无重复函数声明
