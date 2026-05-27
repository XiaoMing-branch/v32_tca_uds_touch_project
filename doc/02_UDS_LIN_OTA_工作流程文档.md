# UDS + LIN + OTA 工作流程文档

> **项目**: V32 电容式外把手
> **芯片**: App — TCAE10 (Cortex-M0+); Boot — TCPL03X (Cortex-M0+)
> **总线**: LIN 2.x / J2602 (19200 bps)
> **协议**: UDS on LIN (ISO 14229-1 / LIN 诊断规范)
> **版本**: V2.0 (基于编译源文件修正)
> **日期**: 2026-05-27

---

## 目录

1. [UDS 会话模式管理](#1-uds-会话模式管理)
2. [安全访问流程](#2-安全访问流程)
3. [固件下载流程 (OTA)](#3-固件下载流程-ota)
4. [固件校验、跳转与激活](#4-固件校验跳转与激活)
5. [LIN 总线 SID/服务映射](#5-lin-总线-sid服务映射)
6. [完整 OTA 状态机](#6-完整-ota-状态机)
7. [LIN 诊断应答码定义](#7-lin-诊断应答码定义)
8. [UDS 时序与超时管理](#8-uds-时序与超时管理)

---

## 1. UDS 会话模式管理

### 1.1 会话模式定义 (SID $10)

| 会话 | 值 | 说明 | App 侧行为 | Boot 侧行为 |
|------|-----|------|-----------|------------|
| DefaultSession (默认会话) | `0x01` | 上电后默认会话 | 执行基础诊断 ($22, $2E, $2F, $3E, $BA~$BD) | 基础诊断, 42ms 等待窗口后跳转 |
| ProgrammingSession (编程会话) | `0x02` | 用于固件更新 | 重启进入 Bootloader | $27, $2E, $31, $34, $36, $37 |
| ExtendedSession (扩展会话) | `0x03` | App 侧扩展诊断功能 | $22, $2E, $2F, 供应商自定义 | 仅返回 Boot 版本信息 |

### 1.2 App 侧会话处理

App 侧的 UDS 服务采用**双层分发架构** (`diagnosticIII.c` + `custom_diagnosticIII.c`):

- **Layer 1 (LIN 协议分发)**: `diagnosticIII.c` 的 `lin_diag_service_handle()` 遍历 `lin_diag_services_supported[27]` 数组，匹配 LIN 标准配置服务 (0xB0~0xB7, 0x2F, 0x32, 0xA0) 并分发给 `sid_0x*.c` 处理器。
- **Layer 2 (客户 UDS 分发)**: 未在 Layer 1 匹配的服务通过 `default:` 下沉至 `lin_custom_diag_service_handle()` (弱符号，在 `custom_diagnosticIII.c` 中覆盖)，经 `lin_handle_uds()` 分发至 UDS 应用层处理器。

会话控制 (`$10`) 通过 Layer 2 下沉由 `custom_diagnosticIII.c` 中的 `uds_diagnostic_session_control()` 处理。
在 App 侧收到 `$10 $02` (编程会话) 后，直接触发 `NVIC_SystemReset()` 进入 Boot。

### 1.3 Boot 侧会话处理

Bootloader 通过 `dfu_uds_manager.c` 中的 `session_control_handle()` 函数处理 $10 服务：

```
Tester → Bootloader:  $10 $02  (请求编程会话)
    │
    ▼
session_control_handle()
    │
    ├── param[1] == 0x01 → DefaultSession
    │     └── uds_request_info.sessionMode = DEFALUT_SESSION
    │
    ├── param[1] == 0x02 → ProgrammingSession
    │     ├── uds_request_info.sessionMode = PROGRAM_SESSION
    │     ├── dfu_ctx.boot_state = BOOT_STATE_UPGRADE
    │     └── LED 指示进入升级模式
    │
    ├── param[1] == 0x03 → ExtendedSession
    │     ├── uds_request_info.sessionMode = PROGRAM_SESSION
    │     └── 返回 boot_version[4] 固件版本信息
    │
    └── 其他 → NRC 0x12 (SFNS)
```

### 1.4 会话与安全等级关系

| 会话 | 安全等级 | 说明 |
|------|---------|------|
| DefaultSession | SECURITY_LEVEL0 | 未安全解锁 |
| ProgrammingSession | SECURITY_LEVEL0 | 刚进入编程会话，未解锁 |
| ProgrammingSession + 安全访问通过 | SECURITY_LEVEL1 | 已解锁，可执行 $2E/$31/$34/$36/$37 |

---

## 2. 安全访问流程

### 2.1 SID $27 服务 (Boot 侧)

安全访问采用 **AES-CMAC** 算法，通过 `aes_cmac.c` 实现密钥派生和校验。

```
Tester → ECU:  $27 $01      (请求种子 - RequestSeed)
    │
    ▼
security_access_handle() (位于 dfu_uds_manager.c)
    │
    ├── subfunc = 0x01
    │     └── 返回 security_seed[4] 作为种子
    │
Tester ← ECU:  $67 $01 <4字节种子>
    │
    ▼
Tester → ECU:  $27 $02 <4字节密钥>  (发送密钥 - SendKey)
    │
    ▼
security_access_handle()
    │
    ├── subfunc = 0x02
    │     ├── 使用 AES-CMAC 校验密钥
    │     ├── 匹配 → uds_request_info.securityLevel = SECURITY_LEVEL1
    │     │        响应: $67 $02 (正响应)
    │     └── 不匹配 → NRC 0x35 (INVALID_KEY)
    │
    ├── 连续 3 次失败 → NRC 0x36 (EXCEED_NUMBER_OF_ATTEMPTS)
    │     └── 锁定时间: os_task_update() 中每约 10s 释放一次锁定
    │
    └── 其他 subfunc → NRC 0x12
```

### 2.2 安全机制说明

| 安全概念 | 本实现 | 说明 |
|---------|-------|------|
| 种子算法 | 动态种子 | 每请求生成新种子，`lin_update_random_value()` 周期性刷新 |
| 密钥校验 | AES-CMAC | 使用 `aes_cmac.c` 计算消息认证码 |
| 安全等级 | SECURITY_LEVEL0 / SECURITY_LEVEL1 | 2 级安全模型 |
| 失败处理 | NRC 0x35 → 0x36 (3次) | 锁定后自动释放 |
| 期间保持 | NRC 0x78 (ResponsePending) | CMAC 计算期间发送，约 1.8s 间隔 |

### 2.3 安全级别矩阵

```
服务             所需安全级别
─────────────────────────────────
$10 (会话控制)    SECURITY_LEVEL0
$11 (ECU复位)     SECURITY_LEVEL0
$27 (安全访问)    SECURITY_LEVEL0
$2E (写DID)       SECURITY_LEVEL1  ← 需解锁
$31 (例程控制)    SECURITY_LEVEL1  ← 需解锁
$34 (请求下载)    SECURITY_LEVEL1  ← 需解锁
$36 (传输数据)    SECURITY_LEVEL1  ← 需解锁
$37 (传输退出)    SECURITY_LEVEL1  ← 需解锁
```

---

## 3. 固件下载流程 (OTA)

### 3.1 完整 OTA 时序

```
Tester (BCM)                          ECU (Bootloader)
    │                                       │
    │  1. 诊断会话控制 ($10 $02)             │
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $50 $02 (正响应)
    │                                       │ boot_state = UPGRADE
    │                                       │
    │  2. 安全访问 ($27 $01)                 │
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $67 $01 + seed[4]
    │                                       │
    │  3. 安全访问 ($27 $02 + key[4])        │
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $67 $02 (正响应)
    │                                       │ security = LEVEL1
    │                                       │
    │  4. 写入DID - 固件信息同步 ($2E)        │
    │  ─────────────────────────────────►    │ 数据: 32 字节
    │  ◄─────────────────────────────────    │ $6E + BootVer[4] + BlockLen[2] + Time[3]
    │                                       │ op_code = SYNC_INFO
    │                                       │
    │  5. 例程控制 - 擦除Flash ($31 $02 $FF00)│
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $71 $02 (正响应)
    │                                       │ op_code = FLASH_ERASE
    │                                       │ [内部擦除 ~800ms]
    │                                       │
    │  ── 重复块 (每块 512 字节) ──           │
    │                                       │
    │  6. 请求下载 ($34 $01)                 │
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $74 $01 (正响应)
    │                                       │ op_code = TRANFER_START
    │                                       │
    │  7. 传输数据 ($36 + 包序号 + 数据+CRC)  │
    │  ─────────────────────────────────►    │ × N 包
    │  ◄─────────────────────────────────    │ $76 (正响应) 或 $7F
    │                                       │
    │  8. 请求传输退出 ($37)                 │
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $77 (正响应)
    │                                       │ op_code = TRANFER_STOP
    │                                       │
    │  ── 下一个块 ──                         │
    │                                       │
    │  9. 例程控制 - CRC校验 ($31 $01 $0202)  │ (最后一块完成后)
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $71 $01 (正响应/负响应)
    │                                       │ 写入 DFU 信息到 Flash
    │                                       │
    │  10. 例程控制 - 复位 ($31 $01 $7221)    │
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $71 $01 (正响应)
    │  [NVIC_SystemReset()]                  │
    │                                       │
    │  (系统复位，Bootloader 启动)             │
    │  (检测 DFU 信息有效 → JumpToApp())      │
    │                                       │
    │  新固件 App 启动                        │
```

### 3.2 数据包格式 ($36 传输数据)

```
┌──────┬──────┬──────────────────────────┬────────────┐
│ SID  │ Index│       DATA (512 字节)     │ CRC32 (4B) │
│ 0x36 │  1B  │      512 bytes            │   4 bytes  │
└──────┴──────┴──────────────────────────┴────────────┘

DFU_PACKET_HEAD_CRC_LENGTH = 6    (SID + Index + CRC 佔位)
DFU_PROGRAM_LENGTH = 512          (每包有效数据长度, DFU_PACKET_BLOCK_LENGTH - 6)
DFU_PACKET_BLOCK_LENGTH = 518     (SID + Index + CRC + DATA)
DFU_PROGRAM_WORDS = 128           (512 / 4 = 128 words)
```

### 3.3 数据包缓冲区管理 (双缓冲队列)

```
queue_list_t {
    packet_unit_t packet[2];     // 双缓冲区 Ping-Pong
    uint8_t head;                // 写入指针 (待编程)
    uint8_t tail;                // 接收指针 (接收完成)
}

工作流程:
1. Tester 发送 $36 数据包 → 写入 packet[tail]
2. 每包接收完成 → tail 递增
3. 当接收数据 >= DFU_PROGRAM_LENGTH 或接收完成 →
    触发 dfu_image_program():
    - 从 packet[head] 读取数据
    - 写入 Flash (512 字节对齐)
    - 计算实时 CRC32
    - head 递增
4. Ping-Pong 交替: 接收第 N+1 包的同时编程第 N 包
```

### 3.4 CRC32 校验机制

```c
// 写入时的实时 CRC 计算 (dfu_image_program 中):
for (uint32_t i = 0; i < DFU_PROGRAM_WORDS; i++, addr += 4)
{
    pal_store_read(FLASH_TYPE_NVM, addr, (uint8_t *)&val, sizeof(uint32_t));
    dfu_ctx.write_crc = crc32_calculate_func(dfu_ctx.write_crc,
                                              (uint8_t *)&val, sizeof(uint32_t));
}

// 每包数据的 CRC 验证:
if (packet->crc32 != dfu_ctx.write_crc)
{
    // CRC 不匹配 → 擦除该扇区 → 返回错误
}

// 最终全镜像 CRC 校验 (例程控制 $31 $01 $0202):
if (dfu_ctx.dfu_info.image_crc != dfu_ctx.write_crc)
{
    // 全镜像 CRC 不匹配 → NRC GENERAL_PROGRAM_FAILURE
}
```

---

## 4. 固件校验、跳转与激活

### 4.1 校验流程

```
最后一块数据传输完成
    │
    ▼
Tester → $31 $01 $0202 (检查编程完整性)
    │
    ▼
routine_control_handle() - RoutineID = 0x0202
    │
    ├── dfu_ctx.resp_value == DFU_MSG_SUCCESS ?
    │     └── NO → NRC GENERAL_PROGRAM_FAILURE
    │
    ├── dfu_ctx.dfu_info.image_crc == dfu_ctx.write_crc ?
    │     │
    │     ├── YES → 校验通过
    │     │     ├── dfu_process_exit(DFU_MSG_SUCCESS)
    │     │     │     ├── 写入 DFU 信息 (magic=0xDEADBEEF, reason=SUCCESS)
    │     │     │     ├── 固件版本/大小/CRC/编译时间戳
    │     │     │     └── deinit LED
    │     │     │
    │     │     └── 响应: 正响应
    │     │
    │     └── NO → 校验失败
    │           └── NRC GENERAL_PROGRAM_FAILURE
    │
    ▼
Tester → $31 $01 $7221 (复位 ECU)
    │
    ▼
NVIC_SystemReset()
    │
    ▼
Bootloader 重启 → 检测 DFU 信息有效 → JumpToApp()
```

### 4.2 跳转流程 (JumpToApp)

```c
STATIC void JumpToApp(void)
{
    // 1. 关闭所有中断
    NVIC_DisableIRQ(TIMER_IRQn);
    NVIC_DisableIRQ(LINSCI_IRQn);      // 或 AFE_INT_IRQn
    NVIC_DisableIRQ(SysTick_IRQn);

    // 2. 反初始化 LIN 外设
    pal_lin_deinit(LIN_BUS_0);

    // 3. 从 App 向量表取出栈指针和入口地址
    //    跳过 0x100 字节用户数据头:
    uint32_t base = dfu_ctx.dfu_info.image_addr + 0x100u;
    uint32_t msp = *(uint32_t *)base;
    uint32_t entry = *(uint32_t *)(base + 4);

    // 4. 设置主栈指针
    __set_MSP(msp);

    // 5. 跳转 (函数指针调用)
    ((void (*)(void))entry)();
}
```

### 4.3 DFU 信息结构

```c
typedef struct {
    uint32_t magic;                  // 魔数 0xDEADBEEF (有效性标记)
    uint32_t image_size;             // 固件镜像大小
    uint32_t image_crc;              // 固件 CRC32 校验值
    uint32_t written_image_length;   // 已写入长度 (未使用)
    uint32_t written_image_crc;      // 已写入 CRC (未使用)
    uint32_t reason;                 // 升级结果 (SUCCESS/ERROR/...)
    uint32_t version;                // 固件版本号
    uint32_t image_addr;             // 固件存储起始地址
    uint32_t time[3];                // 编译时间戳
} last_dfu_info_t;

// 存储位置: FLASH_DFU_INFO_ADDR (Flash 中 Boot 之后)
// 大小: 1 个 Sector (512 字节)
```

---

## 5. LIN 总线 SID/服务映射

### 5.1 服务分发架构概述

UDS 服务在 **App** 和 **Boot** 两侧有不同的实现：

- **App 侧**: 双层分发 (L1 协议分发 + L2 客户 UDS 分发)，见 §1.2
- **Boot 侧**: 单层分发，由 `dfu_uds_manager.c` 中的 `lin_diag_service_handle()` 集中处理

以下 SID 表中，**编译源**列标注了提供该 SID 处理函数的编译单元。

### 5.2 Bootloader 侧 UDS 服务表

Boot 侧编译文件: `dfu_uds_manager.c` (主调度器), `aes_cmac.c` (安全校验)。

| SID | 服务名称 | 处理函数 | 会话限制 | 安全要求 |
|-----|---------|---------|---------|---------|
| `0x10` | DiagnosticSessionControl | `session_control_handle()` | Default/Prog/Ext | Level0 |
| `0x11` | ECUReset | 直接复位 | Default/Prog | Level0 |
| `0x27` | SecurityAccess | `security_access_handle()` | 仅 Prog | Level0 |
| `0x2E` | WriteDataByIdentifier | `firmware_info_sync_handle()` | 仅 Prog | Level1 |
| `0x31` | RoutineControl | `routine_control_handle()` | 仅 Prog | Level1 |
| `0x34` | RequestDownload | `request_download_handle()` | 仅 Prog | Level1 |
| `0x36` | TransferData | `transfer_data_handle()` | 仅 Prog | Level1 |
| `0x37` | RequestTransferExit | `request_transfer_exit_handle()` | 仅 Prog | Level1 |
| `0x87` | LinkControl | `link_control_handle()` | Default/Prog/Ext | Level0 |

> 上述所有服务均在 `dfu_uds_manager.c` 中实现，无独立的 `sid_0x*.c` 文件编译进 Boot。

### 5.3 App 侧 UDS 服务表 (按分发层)

#### Layer 1: LIN 协议分发 (diagnosticIII.c → sid_0x*.c)

这些 SID 由 LIN 协议层 `lin_diag_service_handle()` 直接分发。

| SID | 服务名称 | 处理器函数 | 编译源 |
|-----|---------|-----------|--------|
| `0x2F` | IOControlByIdentifier | `lin_diag_io_control_by_identifier()` | `sid_0x2f.c` |
| `0x32` | GetTraceabilityMsg | `lin_diag_get_traceability_msg()` | `sid_0x32.c` |
| `0xB0` | AssignNAD | `lin_diagservice_assign_nad()` | `sid_0xb0.c` |
| `0xB1` | AssignFrameIdentifier | `lin_diag_assign_frame_identifier()` | `sid_0xb1.c` |
| `0xB2` | ReadByID | `lin_diagservice_read_by_identifier()` | `custom_diagnosticIII.c` |
| `0xB3` | ConditionalChangeNAD | `lin_diag_conditional_change_nad()` | `sid_0xb3.c` |
| `0xB4` | DataDumpControl | 设置 `g_bUDSDataDumpFlag` | `diagnosticIII.c` (内联) |
| `0xB5` | SNPDSlaveNodePosition | `lin_diag_target_reset()` | `sid_0xb5.c` |
| `0xB6` | SaveConfiguration | `lin_diag_save_configuration()` | `sid_0xb6.c` |
| `0xB7` | AssignFrameIDRange | `lin_diag_assign_frame_id_range()` | `sid_0xb7.c` |
| `0xBC` | SocRegRead | `soc_reg_read()` | `sid_0xbc.c` |
| `0xBD` | SocRegWrite | `soc_reg_write()` | `sid_0xbd.c` |
| `0xA0` | ReadLogInfo | 设置 `g_bUDSReadLogInfo` | `diagnosticIII.c` (内联) |
| `0xAD/0xAE/0xAF` | 一致性测试 | — | `sid_0xct.c` (linlib) |

> 编译源列中的 `diagnosticIII.c` (内联) 表示该 SID 直接在分发器中处理，无需独立 sid 文件。

#### Layer 2: 客户 UDS 分发 (custom_diagnosticIII.c)

这些 SID 由 `lin_custom_diag_service_handle()` (弱符号覆盖) 下沉至 `custom_diagnosticIII.c` 处理。

| SID | 服务名称 | 处理器函数 | 说明 |
|-----|---------|-----------|------|
| `0x10` | DiagnosticSessionControl | `uds_diagnostic_session_control()` | 会话切换,$10 $02 → 触发复位进 Boot |
| `0x11` | ECUReset | `uds_diagnostic_rest()` | Hard/Soft/KeyOffOn 复位 |
| `0x14` | ClearDiagnosticInformation | `uds_diagnostic_clear_dtc_info()` | 清除 DTC 信息 |
| `0x22` | ReadDataByIdentifier | `uds_diagnostic_readdata_by_id()` | 读取 DID (11个支持的 DID) |
| `0x28` | CommunicationControl | `uds_communction_control()` | Rx 静音控制 |
| `0x2E` | WriteDataByIdentifier | NRC 0x33 | 拒绝写入 (需安全解锁) |
| `0x31` | RoutineControl | `uds_diagnostic_route_control()` | 例程控制 (RID 0x0203 等) |
| `0x3E` | TesterPresent | `uds_tester_present_control()` | 保持诊断会话活跃 |
| `0x85` | DTCControl | `uds_diagnostic_dtc_control()` | DTC 存储启/禁 |
| `0xB5` | SNPD AssignNAD | `uds_diagnostic_assign_NAD()` | 生产配置字分配 |
| `0x27/0x34/0x36/0x37` | — | NRC 0x7F | 需要编程会话 |

> **注**: `custom_diagnosticIII.c` 中无独立的 `sid_0x*.c` 文件 — 所有上述 SID 的处理器函数直接在 `custom_diagnosticIII.c` 中实现。`sid_0x22.c` **不存在**。

#### 预构建库处理 (linlib_seres.lib)

以下 SID 在 LIN 协议栈库 `linlib_seres.lib` 中处理，源文件存在于 `diag_sid/` 但**未编译进项目**:

| SID | 服务名称 | 源文件存在 | 状态 |
|-----|---------|-----------|------|
| `0xBA` | CustomerLEDConfigGet | `sid_0xba.c` | 未编译, 由 linlib 处理 |
| `0xBB` | CustomerLEDConfigSet | `sid_0xbb.c` | 未编译, 由 linlib 处理 |

> **sid_0x14.c / sid_0x2e.c / sid_0x3e.c / sid_0x11.c 说明**:  
> 这些文件存在于 `app/midware/lin_manager/diag_sid/` 目录并提供 `clear_dtc_info_handle()` 等函数，但**不被当前分发器调用**。对应的 SID (0x14, 0x2E, 0x3E, 0x11) 通过 `default:` 下沉至 `custom_diagnosticIII.c` 处理。这些 sid 文件可视为未使用的参考存根。

### 5.4 App 侧已注册服务表 (lin_diag_services_supported[])

定义于 `lin_cfg.c`，作为 LIN 协议栈的 SID 许可列表:

```c
const l_u8 lin_diag_services_supported[27] = {
    0x10, 0x11, 0x14, 0x19, 0x22, 0x27, 0x28,
    0x2E, 0x2F, 0x31, 0x3E, 0x34, 0x36, 0x37,
    0x85, 0xA0,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xBA, 0xBB, 0xBC
};
```

> 对应标志数组 `lin_diag_services_flag[27]` 由 LIN 传输层在收到请求时设置。

### 5.5 SID 分发路径总览图

```
LIN 帧到达 → PID 匹配 NAD
    │
    ▼
lin_diag_service_handle()        ← diagnosticIII.c
    │
    ├── 遍历 lin_diag_services_supported[27]
    │     └── 匹配 SID → lin_diag_services_flag[i] = 1
    │
    ├── SWITCH on SID:
    │     │
    │     ├── 0xB0 → sid_0xb0.c      (LIN 配置)
    │     ├── 0xB1 → sid_0xb1.c
    │     ├── 0xB2 → custom_diag.c   (注: 特异于项目)
    │     ├── 0xB3 → sid_0xb3.c
    │     ├── 0xB5 → sid_0xb5.c
    │     ├── 0xB6 → sid_0xb6.c
    │     ├── 0xB7 → sid_0xb7.c
    │     ├── 0x2F → sid_0x2f.c
    │     ├── 0x32 → sid_0x32.c
    │     ├── 0xB4 → flags 设置
    │     ├── 0xA0 → flags 设置
    │     ├── 0xAD/0xAE/0xAF → sid_0xct.c
    │     │
    │     └── default → ★ 下沉至 Layer 2 ★
    │           │
    │           ▼
    │     lin_custom_diag_service_handle()  ← custom_diagnosticIII.c
    │           │
    │           ├── 复制请求数据
    │           └── lin_handle_uds()
    │                 ├── 0x10 → session control
    │                 ├── 0x11 → ECU reset
    │                 ├── 0x14 → clear DTC
    │                 ├── 0x22 → read by ID
    │                 ├── 0x28 → comm control
    │                 ├── 0x2E → NRC 0x33
    │                 ├── 0x31 → routine control
    │                 ├── 0x3E → tester present
    │                 ├── 0x85 → DTC control
    │                 ├── 0xB5 → assign NAD
    │                 ├── 0x27/0x34/0x36/0x37 → NRC 0x7F
    │                 └── 其他 → NRC 0x11
    │
    └── 响应发送 (正响应 SID+0x40 或 0x7F+NRC)
```

### 5.6 0xBA/0xBB 二级命令映射 (供应商自定义)

LED 配置服务 (0xBA GET / 0xBB SET) 由预构建库 `linlib_seres.lib` 内部处理。此处列出供参考:

#### 0xBA (GET) 子命令

| 命令值 | 数据长度 | 描述 |
|-------|---------|------|
| `0x0000` | 9 | Get LED PN Voltage |
| `0x0003` | 8 | Get LED Typical PN Volt |
| `0x0002` | 21 | Get LED RGB Parameters |
| `0x0004` | 10 | Get White Point Config |
| `0x0013` | 3 | Get LED RGB Current |
| `0x0007` | 6 | Get PWM Parameters |
| `0x0016` | 5 | Get RGBL Parameters |
| `0x0020` | 4 | Get Relative Factor |
| `0x0022` | 16 | Get Version Info (App/Boot/RTC) |
| `0x0023` | 12 | Get UUID |
| `0x0024` | 1 | Get Static PN Sample Status |
| `0x0028` | 20 | Get LED Vendor Info |
| `0x0029` | 2 | Get LED Status (Fault) |
| `0x0031` | 4 | Get Register Config |
| `0x0032` | sizeof(analog_signal_t) | Get Test Values |

#### 0xBB (SET) 子命令

| 命令值 | 数据长度 | 描述 |
|-------|---------|------|
| `0x0001` | 24 | Set LED RGB Parameters |
| `0x0004` | 10 | Set White Point Config |
| `0x0012` | — | Set Temperature Adjust |
| `0x0005` | — | Set LED RGB Current |
| `0x0011` | — | Set LED PWM Lighting (XY) |
| `0x0014` | 11 | Set LED LUV Lighting |
| `0x0015` | 10 | Set LED RGBL Lighting |
| `0x0019` | 8 | Set Relative Factor |
| `0x0021` | — | Set Static PN Sample |
| `0x0025` | — | Set LED CXY Lighting |
| `0x0026` | — | Set WhiteTest Lighting |
| `0x0027` | — | Reset LED RGB Params + NVIC Reset |
| `0x0030` | — | Set Register Config |

---

## 6. 完整 OTA 状态机

### 6.1 全局状态图

```
                          ┌───────────────────────────┐
                          │       POWER_ON_RESET       │
                          └─────────────┬─────────────┘
                                        │
                                        ▼
                          ┌───────────────────────────┐
                          │   BOOT_STATE_IDLE          │
                          │   (等待 ~50ms, 监听诊断)    │
                          └──────┬─────────────────┬──┘
                                 │                 │
                    ┌────────────┘                 └────────────┐
                    │ 收到 $10 $02                              │ 计时到期
                    ▼                                           ▼
          ┌───────────────────┐                 ┌───────────────────┐
          │BOOT_STATE_UPGRADE │                 │BOOT_STATE_USER_APP│
          │(编程模式)          │                 │(跳转至APP)        │
          └────────┬──────────┘                 └───────────────────┘
                   │                                        │
                   ▼                                        ▼
          ┌───────────────────┐                      [App 应用程序]
          │ OTA 状态机        │
          │ (见下方子状态)     │
          └───────────────────┘
```

### 6.2 OTA 子状态机 (DFU Operation State)

```
                              ┌───────────────────────────┐
                              │      DFU_CMD_SYNC_INFO     │ ← 收到 $2E
                              │  (同步固件信息: 地址/大小/CRC)│
                              └─────────────┬─────────────┘
                                            │ info 有效
                                            ▼
                              ┌───────────────────────────┐
                              │     DFU_CMD_FLASH_ERASE    │ ← 收到 $31 $02 $FF00
                              │  (擦除 Flash 扇区)         │
                              └─────────────┬─────────────┘
                                            │ 擦除完成
                                            ▼
                              ┌───────────────────────────┐
                              │   DFU_CMD_TRANFER_START    │ ← 收到 $34 $01
                              │  (请求下载, 开始传输)       │
                              └─────────────┬─────────────┘
                                            │
                                            ▼
                         ┌─────────────────────────────────────┐
                         │        DATA TRANSFER (接收数据)      │
                         │  ┌─────┐    ┌─────┐    ┌─────┐     │
                         │  │$36#1│──►│$36#2│──►│...  │     │
                         │  └─────┘    └─────┘    └─────┘     │
                         │         │ 双缓冲 Ping-Pong          │
                         │         ▼                           │
                         │  ┌─────────────────────┐            │
                         │  │  dfu_image_program() │ 写入 Flash │
                         │  └─────────────────────┘            │
                         └─────────────────────────────────────┘
                                            │
                                            │ 块内所有数据传输完成
                                            ▼
                              ┌───────────────────────────┐
                              │    DFU_CMD_TRANFER_STOP    │ ← 收到 $37
                              │  (请求传输退出)             │
                              └─────────────┬─────────────┘
                                            │
                                            ▼
                              ┌───────────────────────────┐
                              │   CRC 校验 (Routine 0x0202)│
                              │  image_crc == write_crc ? │
                              ├──────────┬────────────────┤
                              │ 成功     │ 失败            │
                              ▼          ▼                │
                     ┌────────────┐  ┌────────────┐       │
                     │写DFU信息   │  │NRC返回     │       │
                     │(SUCCESS)   │  │(GPF)       │       │
                     └─────┬──────┘  └────────────┘       │
                           │                               │
                           ▼                               │
                     ┌────────────────┐                    │
                     │$31 $01 $7221   │                    │
                     │复位ECU         │                    │
                     └───────┬────────┘                    │
                             │                             │
                             ▼                             │
                     ┌────────────────┐                    │
                     │ NVIC_SystemReset()                  │
                     │ → 重新回到 POWER_ON_RESET           │
                     └────────────────┘                    │
                                                           │
                           ┌───────────────────────────────┘
                           │ 超时 (>3s 无 UDS 请求)
                           ▼
                     ┌───────────────────────────┐
                     │  dfu_process_exit(TIMEOUT) │
                     │  ctx清零, 保持 BOOT_STATE  │
                     └───────────────────────────┘
```

### 6.3 Bootloader 主循环伪代码

```c
void main_loops(void)
{
    static uint32_t LoopCnt = 0;

    // 每次循环处理收到的 LIN UDS 诊断请求
    lin_diag_service_handle();

    if (boot_state == BOOT_STATE_IDLE)
    {
        delay_ms(1);
        if ((++LoopCnt) > 50)         // 约 50ms
        {
            LoopCnt = 0;
            if (last_dfu_info_get() == SUCCESS)
                boot_state = BOOT_STATE_USER_APP;   // 跳转 App
            else
                boot_state = BOOT_STATE_UPGRADE;     // 等待 OTA
        }
    }
    else if (boot_state == BOOT_STATE_USER_APP)
    {
        JumpToApp();
    }
    // boot_state == BOOT_STATE_UPGRADE: 继续循环, 等待 UDS 请求
    // UDS 请求由 lin_diag_service_handle() 中的 SID 分发器处理
    // 相应处理函数设置 op_code → 触发 OTA 状态机
}

void os_task_update(void)             // 覆盖弱符号, 由 SysTick_Handler 每 1ms 调用
{
    dfu_timeout_handle();             // 检查 UDS 超时
    // CMAC 计算期间 NRC 0x78 保持发送
    // 锁定计数递减
}

void dfu_timeout_handle(void)
{
    if (op_code != 0)                 // 正在 OTA 过程中
    {
        uds_timeout++;
        if (uds_timeout > LIN_UDS_TIMEOUT)    // 3000ms
        {
            dfu_process_exit(DFU_MSG_TIMEOUT);
        }
    }
    else
    {
        uds_timeout = 0;              // 空闲时清零
    }
}
```

---

## 7. LIN 诊断应答码定义

### 7.1 负响应码 (NRC)

| 宏名 | 值 | 描述 | ISO 14229 定义 |
|------|-----|------|---------------|
| `NRC_SNS` | `0x11` | Service Not Supported | 服务不支持 |
| `NRC_SFNS` | `0x12` | Sub-Function Not Supported | 子功能不支持 |
| `NRC_IMLOIF` | `0x13` | Incorrect Message Length Or Invalid Format | 报文长度/格式错误 |
| `NRC_RTL` | `0x14` | Response Too Long | 响应过长 |
| `NRC_BRR` | `0x21` | Busy Repeat Request | 忙碌重试 |
| `NRC_CNC` | `0x22` | Conditions Not Correct | 条件不满足 |
| `NRC_RSE` | `0x24` | Request Sequence Error | 请求序列错误 |
| `NRC_NRFSC` | `0x25` | No Response From Subnet Component | 子网无响应 |
| `NRC_FPEORA` | `0x26` | Failure Prevents Execution Of Requested Action | 无法执行请求 |
| `NRC_ROOR` | `0x31` | Request Out Of Range | 请求超出范围 |
| `NRC_SAD` | `0x33` | Security Access Denied | 安全访问被拒 |
| `NRC_IK` | `0x35` | Invalid Key | 密钥无效 |
| `NRC_ENOA` | `0x36` | Exceed Number Of Attempts | 超过尝试次数 |
| `NRC_RCRRP` | `0x78` | Request Correctly Received - Response Pending | 正确接收,正处理 |

### 7.2 响应报文格式

```
正响应:
┌─────────┬──────────────────┐
│ SID+0x40 │   数据 (可选)    │
│ (1 字节)  │   (N 字节)      │
└─────────┴──────────────────┘

负响应:
┌──────┬──────┬──────┐
│ 0x7F │ SID  │ NRC  │
│  1B  │  1B  │  1B  │
└──────┴──────┴──────┘
```

---

## 8. UDS 时序与超时管理

### 8.1 超时参数

| 参数 | 值 | 说明 | 管理位置 |
|------|-----|------|---------|
| `LIN_UDS_TIMEOUT` | 3000 ms | OTA 过程 UDS 请求超时 | `dfu_uds_manager.c` |
| `N_As` timeout | 传输层确认超时 | LIN 协议栈管理 | `linlib_seres.lib` |
| `N_Cr` timeout | 多帧传输流控超时 | LIN 协议栈管理 | `linlib_seres.lib` |
| SysTick period | 1 ms | 系统时基 | `pal_systick.c` / `tc_port.c` |
| Boot IDLE 等待 | ~50 ms (50 loops) | 升级指令窗口 | `main_loops()` |

### 8.2 LIN 传输层时序

```
单帧传输:
Tester → Slave:  [PID] [SID] [数据...] [Checksum]
    │                    │
    ▼                    ▼
Slave 在下一帧 PID 匹配时发送响应

多帧传输 (第一帧 FF / 流控制 FC / 后续帧 CF):
Tester → Slave:  FF (第一帧, 8 字节)
Tester ← Slave:  FC (流控制, 允许发送/等待)
Tester → Slave:  CF (连续帧)
... (重复直到传输完成)
```

---

## 附录 A: 诊断服务完整调用验证矩阵

| 步骤 | SID | 子功能/参数 | 前置条件 | 期望响应 |
|------|-----|------------|---------|---------|
| 1 | `$10` | `$02` | 无 | `$50 $02` + session=Pgm |
| 2 | `$27` | `$01` | PgmSession | `$67 $01` + seed[4] |
| 3 | `$27` | `$02` + key | 已请求种子 | `$67 $02` |
| 4 | `$2E` | DID + info[32] | SecurityLv1 | `$6E` + bootVer[4] + blockLen[2] + time[3] |
| 5 | `$31` | `$02 $FF00` | Sync完成 | `$71 $02` + Flash擦除 |
| 6 | `$34` | `$01` | Erase完成 | `$74 $01` |
| 7 | `$36` | seq# + data[512] + CRC[4] | XferStart | `$76` + seq# |
| 8 | `$37` | — | 所有数据传完 | `$77` |
| 9 | `$31` | `$01 $0202` | XferExit | `$71 $01` + CRC验证 |
| 10 | `$31` | `$01 $7221` | CRC通过 | `$71 $01` + 复位 |

---

## 附录 B: App/Boot 侧 SID 覆盖范围对比

```
SID     App Layer 1 (sid_0x*.c)  App Layer 2 (custom_diag)   Boot (dfu_uds_manager)
───     ───────────────────────  ──────────────────────────  ──────────────────────
0x10    —                        ✅ session control          ✅ session control
0x11    —                        ✅ ECU reset                ✅ direct reset
0x14    —                        ✅ clear DTC                —
0x22    —                        ✅ read by ID               —
0x27    —                        ❌ NRC 0x7F                 ✅ security access
0x28    —                        ✅ comm control             —
0x2E    —                        ❌ NRC 0x33                 ✅ firmware info sync
0x2F    ✅ IO control            —                           —
0x31    —                        ✅ routine control          ✅ routine control
0x32    ✅ traceability          —                           —
0x3E    —                        ✅ tester present           —
0x34    —                        ❌ NRC 0x7F                 ✅ request download
0x36    —                        ❌ NRC 0x7F                 ✅ transfer data
0x37    —                        ❌ NRC 0x7F                 ✅ transfer exit
0x85    —                        ✅ DTC control              —
0x87    —                        —                           ✅ link control
0xB0    ✅ assign NAD            —                           —
0xB1    ✅ assign frame ID       —                           —
0xB2    ✅ read by ID (custom)   —                           —
0xB3    ✅ conditional change    —                           —
0xB4    ✅ data dump flag        —                           —
0xB5    ✅ target reset          ✅ assign NAD (override)    —
0xB6    ✅ save config           —                           —
0xB7    ✅ assign frame ID range —                           —
0xBA    linlib (prebuilt)        —                           —
0xBB    linlib (prebuilt)        —                           —
0xBC    ✅ SoC reg read          —                           —
0xBD    ✅ SoC reg write         —                           —
```
