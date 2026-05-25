# UDS + LIN + OTA 工作流程文档

> **项目**: V32 电容式外把手  
> **总线**: LIN 2.x / J2602  
> **协议**: UDS on LIN (ISO 14229-1)  
> **版本**: V1.0  
> **日期**: 2026-05-25

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

| 会话 | 值 | 说明 | 允许的服务 |
|------|-----|------|-----------|
| DefaultSession (默认会话) | `0x01` | 上电后默认会话，可执行基础诊断服务 | $22, $2E, $2F, $3E, 供应商自定义 |
| ProgrammingSession (编程会话) | `0x02` | 用于固件更新 (Bootloader 侧) | $27, $2E, $31, $34, $36, $37, $87 |
| ExtendedSession (扩展会话) | `0x03` | App 侧扩展诊断功能 | $22, $2E, $2F, 供应商自定义 |

### 1.2 App 侧会话处理

App 侧的会话管理在 LIN 中间件 `lin_manager/diagnosticIII.c` 中通过 `lin_diag_services_supported[]` 和 `lin_diag_services_flag[]` 控制。  
App 层使用 `lin_custom_diag_service_handle()` 弱函数来扩展供应商自定义服务（如 LED 控制、数据读取等 0xBA~0xBD 系列服务）。

会话切换主要在 Bootloader 侧实现。

### 1.3 Boot 侧会话处理

Bootloader 通过 `session_control_handle()` 函数处理 $10 服务：

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
    │     └── led_indicate_init()  // LED 指示进入升级模式
    │
    ├── param[1] == 0x03 → ExtendedSession (仅返回 Boot 版本信息)
    │     ├── uds_request_info.sessionMode = PROGRAM_SESSION
    │     └── 返回 boot_version[4] 数据
    │
    └── 其他 → NRC 0x12 (SUBFUNCTION_NOT_SUPPORTED)
```

### 1.4 会话与安全等级关系

| 会话 | 安全等级 | 说明 |
|------|---------|------|
| DefaultSession | SECURITY_LEVEL0 | 未安全解锁 |
| ProgrammingSession | SECURITY_LEVEL0 | 刚进入编程会话，未解锁 |
| ProgrammingSession + 安全访问通过 | SECURITY_LEVEL1 | 已解锁，可执行 $2E/$31/$34/$36/$37 |

---

## 2. 安全访问流程

### 2.1 SID $27 服务

安全访问用于在编程会话中解锁 ECU，允许执行固件擦写操作。

```
Tester → ECU:  $27 $01      (请求种子 - RequestSeed)
    │
    ▼
security_access_handle()
    │
    ├── subfunc = 0x01
    │     └── 直接返回 security_code[4] 作为 "种子"
    │         (注: 本实现为简化方案，种子 = 密钥)
    │
Tester ← ECU:  $67 $01 <4字节种子>
    │
    ▼
Tester → ECU:  $27 $02 <4字节密钥>  (发送密钥 - SendKey)
    │
    ▼
security_access_handle()
    │
    ├── subfunc = 0x02 / 0x04
    │     ├── 比较 param[2..5] 与 security_code[0..3]
    │     ├── 匹配 → uds_request_info.securityLevel = SECURITY_LEVEL1
    │     │        响应: $67 $02 (正响应)
    │     └── 不匹配 → NRC 0x35 (INVALID_KEY)
    │                 响应: $7F $27 $35
    │
    └── 其他 subfunc → NRC 0x12
```

### 2.2 安全机制说明

| 安全概念 | 本实现 | 说明 |
|---------|-------|------|
| 种子算法 | 固定密钥直接作为种子 | 简化设计，种子=密钥 |
| 密钥计算 | 直接比较 | 无加密算法 |
| 安全等级 | SECURITY_LEVEL0 / SECURITY_LEVEL1 | 2 级安全模型 |
| 失败处理 | NRC 0x35 | 无次数限制 (本实现) |
| 安全代码 | `{0x41, 0x31, 0x12, 0x01/0x02}` | 不同芯片略有差异 |

> **注意**: 当前使用的安全方案为简化设计。实际产品级实现建议引入 Seed&Key 算法（如 AES-128 或自定义 XOR 算法）。

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
    │  ─────────────────────────────────►    │ 数据: 32字节
    │  ◄─────────────────────────────────    │ $6E (正响应) + BootVer + BlockLen + Time
    │                                       │ op_code = SYNC_INFO
    │                                       │
    │  5. 例程控制 - 擦除Flash ($31 $02 0xFF00)│
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $71 $02 (正响应)
    │                                       │ op_code = FLASH_ERASE
    │                                       │ [内部擦除 Flash 耗时 ~800ms]
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
    │  ── 重复块 ──                          │
    │                                       │
    │  ...                                   │
    │                                       │
    │  9. 例程控制 - CRC校验 ($31 $01 0x0202) │ (最后一块完成后)
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $71 $01 (正响应/负响应)
    │                                       │ 写入 DFU 信息到 Flash
    │                                       │
    │  10. 例程控制 - 复位 ($31 $01 0x7221)   │
    │  ─────────────────────────────────►    │
    │  ◄─────────────────────────────────    │ $71 $01 (正响应)
    │  [NVIC_SystemReset()]                  │
    │                                       │
    │  (系统复位，Bootloader 启动)             │
    │  (检测到 DFU 信息有效 → JumpToApp())     │
    │                                       │
    │  新固件 App 启动                        │
```

### 3.2 数据包格式 ($36 传输数据)

```
┌──────┬──────┬──────────────────────────┬────────────┐
│ SID  │ Index│       DATA (512 字节)     │ CRC32 (4B) │
│ 0x36 │  1B  │      512 bytes            │   4 bytes  │
└──────┴──────┴──────────────────────────┴────────────┘
              └───────── DFU_PROGRAM_LENGTH ──────────┘
              └──────── DFU_PACKET_BLOCK_LENGTH = 6 + 512 ──────┘

DFU_PACKET_HEAD_CRC_LENGTH = 6  (SID + Index + CRC占位)
DFU_PROGRAM_LENGTH = 512          (每包有效数据长度)
DFU_PROGRAM_WORDS = 128           (512/4 = 128 words)
```

### 3.3 数据包缓冲区管理 (双缓冲队列)

```
queue_list_t {
    packet_unit_t packet[2];    // 双缓冲区 Ping-Pong
    uint8_t head;               // 写入指针 (待编程)
    uint8_t tail;               // 接收指针 (接收完成)
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
4. Ping-Pong 交替，实现"接收-编程"流水线
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

// 最终全镜像 CRC 校验 (例程控制 0x0202):
if (dfu_ctx.dfu_info.image_crc != dfu_ctx.write_crc)
{
    // 全镜像 CRC 不匹配 → NRC
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
routine_control_handle() - RoutineID=0x0202
    │
    ├── dfu_ctx.resp_value == DFU_MSG_SUCCESS ?
    │     └── NO → NRC GENERAL_PROGRAM_FAILURE
    │
    ├── dfu_ctx.dfu_info.image_crc == dfu_ctx.write_crc ?
    │     │
    │     ├── YES → 校验通过
    │     │     ├── dfu_process_exit(DFU_MSG_SUCCESS)
    │     │     │     ├── 写入 DFU 信息 (magic=0xDEADBEEF, reason=SUCCESS)
    │     │     │     │     ├── 固件版本
    │     │     │     │     ├── 固件大小
    │     │     │     │     ├── 固件 CRC
    │     │     │     │     └── 编译时间戳
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
void JumpToApp(void)
{
    // 1. 关闭所有中断
    NVIC_DisableIRQ(TIMER_IRQn);
    NVIC_DisableIRQ(LINSCI_IRQn);   // 或 AFE_INT_IRQn
    NVIC_DisableIRQ(SysTick_IRQn);

    // 2. 反初始化 LIN 外设
    pal_lin_deinit(LIN_BUS_0);

    // 3. 从 App 向量表取出栈指针和入口地址
    //    (App 向量表在 FLASH_APP_ADDR 处)
    uint32_t msp = *(uint32_t *)dfu_ctx.dfu_info.image_addr;
    uint32_t entry = *(uint32_t *)(dfu_ctx.dfu_info.image_addr + 4);

    // 4. 设置主栈指针
    __set_MSP(msp);

    // 5. 跳转 (函数指针调用)
    ((void (*)(void))entry)();
}
```

### 4.3 DFU 信息结构

```c
typedef struct {
    uint32_t magic;                 // 魔数 0xDEADBEEF (有效性标记)
    uint32_t image_size;            // 固件镜像大小
    uint32_t image_crc;             // 固件 CRC32 校验值
    uint32_t written_image_length;  // 已写入长度 (未使用)
    uint32_t written_image_crc;     // 已写入 CRC (未使用)
    uint32_t reason;                // 升级结果 (SUCCESS/ERROR/...)
    uint32_t version;               // 固件版本号
    uint32_t image_addr;            // 固件存储起始地址 (⚠️ 不可移动字段位置)
    uint32_t time[3];               // 编译时间戳
} last_dfu_info_t;

// 存储位置: FLASH_DFU_INFO_ADDR (Flash 中 Boot 之后)
// 大小: 1 个 Sector (512 字节)
```

---

## 5. LIN 总线 SID/服务映射

### 5.1 Bootloader 侧 UDS 服务表

| SID | 服务名称 | 处理函数 | 会话限制 | 安全要求 |
|-----|---------|---------|---------|---------|
| `0x10` | DiagnosticSessionControl | `session_control_handle()` | Default/Prog/Ext | Level0 |
| `0x11` | ECUReset | 无处理 (直接复位) | Default/Prog | Level0 |
| `0x27` | SecurityAccess | `security_access_handle()` | 仅 Prog | Level0 |
| `0x2E` | WriteDataByIdentifier | `firmware_info_sync_handle()` | 仅 Prog | Level1 |
| `0x31` | RoutineControl | `routine_control_handle()` | 仅 Prog | Level1 |
| `0x34` | RequestDownload | `request_download_handle()` | 仅 Prog | Level1 |
| `0x36` | TransferData | `transfer_data_handle()` | 仅 Prog | Level1 |
| `0x37` | RequestTransferExit | `request_transfer_exit_handle()` | 仅 Prog | Level1 |
| `0x87` | LinkControl | `link_control_handle()` | Default/Prog/Ext | Level0 |

### 5.2 App 侧 UDS 服务表 (自定义 + 标准)

| SID | 服务名称 | 处理函数 | 说明 |
|-----|---------|---------|------|
| `0x22` | ReadDataByIdentifier | `lin_diagservice_read_by_identifier()` | 读取 DID (产品ID/序列号/用户数据) |
| `0x2E` | WriteDataByIdentifier | `lin_diag_write_by_identifier()` | 写 DID，仅返回正响应 |
| `0x2F` | IOControlByIdentifier | `lin_diag_io_control_by_identifier()` | IO 控制，仅返回正响应 |
| `0x3E` | TesterPresent | `lin_diag_tester_present()` | 保持诊断会话活跃 |
| `0xB0` | AssignNAD | `lin_diagservice_assign_nad()` | LIN NAD 分配 |
| `0xB1` | AssignFrameIdentifier | `lin_diag_assign_frame_identifier()` | LIN 帧 ID 分配 |
| `0xB2` | ReadByID | `lin_diagservice_read_by_identifier()` | LIN 产品/序列号读取 |
| `0xB3` | ConditionalChangeNAD | `lin_diag_conditional_change_nad()` | 条件 NAD 变更 |
| `0xB4` | DataDumpControl | `lin_diag_data_dump_control()` | 数据转储 (ADC采样数据) |
| `0xB5` | SNPDSlaveNodePosition | `lin_diag_snpd()` | 从节点位置检测 |
| `0xB6` | SaveConfiguration | `lin_diag_save_configuration()` | 保存配置到 Flash |
| `0xB7` | AssignFrameIDRange | `lin_diag_assign_frame_id_range()` | 帧 ID 范围分配 |
| `0xBA` | CustomerLEDConfigGet | `lin_diag_led_config_get()` | 读取 LED 参数/版本/状态 |
| `0xBB` | CustomerLEDConfigSet | `lin_diag_led_config_set()` | 设置 LED RGB 参数 |
| `0xBC` | RegRead | `soc_reg_read()` | SoC 寄存器读取 |
| `0xBD` | RegWrite | `soc_reg_write()` | SoC 寄存器写入 |
| `0x11` | ECUReset | `lin_diag_ecu_reset()` | 硬件复位 |
| `0x14` | ClearDiagnosticInformation | `clear_dtc_info_handle()` | 清除 DTC 信息 |
| `0x32` | GetTraceabilityMsg | `lin_diag_get_traceability_msg()` | 追溯信息 (空实现) |

### 5.3 0xBA/0xBB 二级命令映射 (供应商自定义)

0xBA (GET) 子命令:

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

0xBB (SET) 子命令:

| 命令值 | 数据长度 | 描述 |
|-------|---------|------|
| `0x0001` | 24 | Set LED RGB Parameters |
| `0x0004` | 10 | Set White Point Config |
| `0x0012` | - | Set Temperature Adjust |
| `0x0005` | - | Set LED RGB Current |
| `0x0011` | - | Set LED PWM Lighting (XY) |
| `0x0014` | 11 | Set LED LUV Lighting |
| `0x0015` | 10 | Set LED RGBL Lighting |
| `0x0019` | 8 | Set Relative Factor |
| `0x0021` | - | Set Static PN Sample |
| `0x0025` | - | Set LED CXY Lighting |
| `0x0026` | - | Set WhiteTest Lighting |
| `0x0027` | - | Reset LED RGB Params + NVIC Reset |
| `0x0030` | - | Set Register Config |

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
                          │   (等待 ~42ms，监听诊断)    │
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
                              │  (请求下载，开始传输)       │
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
                                            │ 所有数据传输完成
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

```
void main_loops(void)
{
    static uint32_t LoopCnt = 0;

    // 每次循环处理收到的 LIN UDS 诊断请求
    lin_diag_service_handle();

    if (boot_state == BOOT_STATE_IDLE)
    {
        delay_ms(1);
        if ((++LoopCnt) > 50)  // 约 42ms + 5ms
        {
            LoopCnt = 0;
            if (last_dfu_info_get() == SUCCESS)
                boot_state = BOOT_STATE_USER_APP;  // 跳转 App
            else
                boot_state = BOOT_STATE_UPGRADE;    // 等待 OTA
        }
    }
    else if (boot_state == BOOT_STATE_USER_APP)
    {
        JumpToApp();
    }
    // boot_state == BOOT_STATE_UPGRADE 时，主循环继续
    // 等待并处理 UDS 诊断请求
}

void os_task_update(void)
{
    dfu_timeout_handle();  // 每秒检查 UDS 超时
}

void dfu_timeout_handle(void)
{
    if (op_code != 0)            // 正在 OTA 过程中
    {
        uds_timeout++;
        if (uds_timeout > 3000)  // 3 秒超时
        {
            // 恢复 LIN 波特率
            // 退出 OTA 流程
            dfu_process_exit(DFU_MSG_TIMEOUT);
        }
    }
    else
    {
        uds_timeout = 0;         // 空闲时清零
    }
}
```

---

## 7. LIN 诊断应答码定义

### 7.1 负响应码 (NRC)

| 宏 | 值 | 描述 |
|----|-----|------|
| `SNS` | `0x11` | Service Not Supported |
| `SFNS` | `0x12` | Sub-Function Not Supported |
| `IMLOIF` | `0x13` | Incorrect Message Length Or Invalid Format |
| `RTL` | `0x14` | Response Too Long |
| `BRR` | `0x21` | Busy Repeat Request |
| `CNC` | `0x22` | Conditions Not Correct |
| `RSE` | `0x24` | Request Sequence Error |
| `NRFSC` | `0x25` | No Response From Subnet Component |
| `FPEORA` | `0x26` | Failure Prevents Execution Of Requested Action |
| `ROOR` | `0x31` | Request Out Of Range |
| `SAD` | `0x33` | Security Access Denied |
| `IK` | `0x35` | Invalid Key |
| `ENOA` | `0x36` | Exceed Number Of Attempts |
| `RCRRP` | `0x78` | Request Correctly Received - Response Pending |

### 7.2 响应报文格式

```
正响应:
┌─────────┬──────────────────┐
│ SID+0x40 │   数据 (可选)    │
│ (1 字节) │   (N 字节)      │
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

| 参数 | 值 | 说明 |
|------|-----|------|
| `LIN_UDS_TIMEOUT` | 3000 ms | OTA 过程 UDS 请求超时 |
| `N_As` timeout | 传输层确认超时 | 由 LIN 协议栈管理 |
| `N_Cr` timeout | 多帧传输流控制超时 | 由 LIN 协议栈管理 |
| `N_Ar` / `N_Br` | LIN 总线超时参数 | LIN 协议栈配置 |

### 8.2 LIN 传输层时序

```
单帧传输:
Tester → Slave:  [PID] [SID] [数据...] [Checksum]
    │                    │
    ▼                    ▼
Slave 在下一帧 PID 匹配时发送响应

多帧传输 (第一帧 FF / 流控制 FC / 后续帧 CF):
Tester → Slave:  FF (第一帧, 8字节)
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
| 8 | `$37` | - | 所有数据传完 | `$77` |
| 9 | `$31` | `$01 $0202` | XferExit | `$71 $01` + CRC验证 |
| 10 | `$31` | `$01 $7221` | CRC通过 | `$71 $01` + 复位 |
