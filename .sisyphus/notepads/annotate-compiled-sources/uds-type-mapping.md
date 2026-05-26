# UDS 类型映射注释参考模板

> 本文档提供 UDS（ISO 14229）诊断协议在项目中的类型/常量/结构体注释规范。
> 所有模板均从项目实际源代码（`diagnosticIII.h`、`dfu_uds_manager.h`、`custom_diagnosticIII.c` 等）中提取。
> 供 Wave 5（UDS 精确注释专项）注释编写时参考。

---

## 目录

1. [NRC 否定响应码枚举（0x10~0x78）](#1-nrc-否定响应码枚举0x100x78)
2. [会话类型定义（默认/扩展/编程）](#2-会话类型定义默认扩展编程)
3. [安全访问等级定义](#3-安全访问等级定义)
4. [DID 数据标识符注释模板](#4-did-数据标识符注释模板)
5. [SID 服务标识符列表](#5-sid-服务标识符列表)
6. [P2/P2E 超时参数说明](#6-p2p2e-超时参数说明)
7. [uds_request_info 结构体成员注释](#7-uds_request_info-结构体成员注释)

---

## 1. NRC 否定响应码枚举（0x10~0x78）

**来源**: `diagnosticIII.h` + `custom_diagnosticIII.c`

项目实际使用的 NRC 宏定义及其触发条件注释模板：

### 模板示例

```c
/* ────────────────────────────────────────── *
 *  NRC 否定响应码定义                          *
 *  协议标准: ISO 14229-1:2020                *
 *  响应格式: [0x7F][SID][NRC] (固定3字节)     *
 * ────────────────────────────────────────── */

/** @defgroup NRC_Codes 否定响应码(NRC)枚举
 *  @brief 当诊断服务请求被拒绝时，ECU返回0x7F+SID+NRC
 *  @note 否定响应格式: 首字节0x7F(否定响应标识)，第2字节为原始SID，第3字节为NRC错误码
 *  @{
 */

/** @brief NRC 0x11 - 服务不支持 (serviceNotSupported)
 *  @trigger ECU不支持该SID，或该SID在当前会话/寻址模式下不可用
 *  @note ISO 14229-1: 请求的诊断服务ID未被ECU实现 */
// #define UDS_SERVICE_NOT_SUPPORTED_11       0x11u

/** @brief NRC 0x12 - 子功能不支持 (sub-functionNotSupported)
 *  @trigger SID支持，但请求的子功能sub-function未被ECU实现 */
// #define SFNS                              0x12

/** @brief NRC 0x13 - 报文长度错误或格式无效 (incorrectMessageLengthOrInvalidFormat)
 *  @trigger 请求报文长度与SID期望长度不匹配，或参数格式错误 */
// #define IMLOIF                            0x13

/** @brief NRC 0x14 - 响应过长 (responseTooLong)
 *  @trigger ECU生成的响应数据超过了传输层最大报文长度限制 */
// #define RTL                               0x14

/** @brief NRC 0x21 - 忙/请重试 (busyRepeatRequest)
 *  @trigger ECU正忙（如正在执行Flash编程），无法处理当前请求，Tester应稍后重试 */
// #define BRR                               0x21

/** @brief NRC 0x22 - 条件不满足 (conditionsNotCorrect)
 *  @trigger 当前条件不满足执行该操作（如安全访问未解锁、车速条件不符合等）
 *  @note 项目中用于：车速≥0x36km/h时禁止IO控制 */
// #define CNC                               0x22
// #define UDS_COND_NOT_CORRECT_22           0x22u

/** @brief NRC 0x24 - 请求序列错误 (requestSequenceError)
 *  @trigger 服务请求顺序错误（如未请求种子直接发送密钥） */
// #define RSE                               0x24

/** @brief NRC 0x25 - 子网组件无响应 (noResponseFromSubnetComponent)
 *  @trigger 网关ECU转发请求后，目标子网节点未在超时内响应 */
// #define NRFSC                             0x25

/** @brief NRC 0x26 - 失败阻止请求执行 (FailurePreventsExecutionOfRequestedAction)
 *  @trigger 因内部故障（硬件/软件错误）导致无法执行请求操作 */
// #define FPEORA                            0x26

/** @brief NRC 0x31 - 请求越界 (requestOutOfRange)
 *  @trigger 请求参数超出有效范围（如DID不存在、数据格式无效等）
 *  @note 项目中用于：DID检查失败、无效参数、IO控制ID不匹配 */
// #define ROOR                              0x31
// #define UDS_REQUEST_OUT_OF_RANGE_31       0x31u

/** @brief NRC 0x33 - 安全访问被拒绝 (securityAccessDenied)
 *  @trigger 安全访问未解锁，或当前安全等级不足以执行该服务
 *  @note ISO 14229-1: 解锁前尝试访问受保护功能时返回 */
// #define SAD                               0x33
// #define UDS_DID_SEC_ERR_33                0x33u

/** @brief NRC 0x35 - 密钥无效 (invalidKey)
 *  @trigger 安全访问$27服务中发送的密钥与ECU计算值不匹配 */
// #define IK                                0x35

/** @brief NRC 0x36 - 超过尝试次数 (exceedNumberOfAttempts)
 *  @trigger 安全访问密钥验证失败次数超过ECU允许的最大重试上限 */
// #define ENOA                              0x36

/** @brief NRC 0x72 - 一般编程失败 (generalProgrammingFailure)
 *  @trigger Flash编程/擦除操作失败（如写入后CRC校验不通过、地址越界等）
 *  @note 项目中用于：OTA固件更新时的编程/擦除错误 */
// #define UDS_GENERAL_PROGRAMMING_FAIL_72   0x72u

/** @brief NRC 0x78 - 正确接收但响应挂起 (requestCorrectlyReceived-ResponsePending)
 *  @trigger ECU已正确接收请求，但需要额外时间处理（如Flash擦除中），
 *          在此期间持续发送此NRC阻止Tester超时
 *  @note 应用在: Flash擦除(800ms)、Flash编程(60ms)等耗时操作中 */
// #define RCRRP                             0x78
// #define UDS_REQ_CORR_REC_RESP_PEND_78     0x78u

/** @brief NRC 0x7E - 条件不支持 (conditionNotSupported)
 *  @trigger 当前会话模式下不支持此子功能 */
// #define UDS_COND_NOT_SUPPORT_7E           0x7Eu

/** @brief NRC 0x7F - 当前会话不支持该服务 (serviceNotSupportedInActiveSession)
 *  @trigger 该SID在当前的会话模式下不被允许（如在默认会话中请求编程服务）
 *  @note 项目中用于: 非扩展会话下禁止DID读取、非编程会话下禁止OTA操作 */
// #define UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F  0x7Fu

/** @} */
```

### 项目实际宏定义映射

| NRC | 宏名 | 文件 | 触发条件简述 |
|-----|------|------|-------------|
| 0x11 | `SNS` / `UDS_SERVICE_NOT_SUPPORTED_11` | diagnosticIII.h / custom_diagnosticIII.c | 服务ID不被ECU支持 |
| 0x12 | `SFNS` / `UDS_SUBFUNC_NOT_SUPP_12` | diagnosticIII.h / custom_diagnosticIII.c | 子功能不支持 |
| 0x13 | `IMLOIF` / `UDS_INCOR_LEN_INVALID_FORMAT_13` | diagnosticIII.h / custom_diagnosticIII.c | 报文长度/格式错误 |
| 0x14 | `RTL` / `UDS_RESPONSE_TOO_LONG_14` | diagnosticIII.h / custom_diagnosticIII.c | 响应数据过长 |
| 0x21 | `BRR` / `UDS_BUSY_REPEAT_REQUEST_21` | diagnosticIII.h / custom_diagnosticIII.c | ECU忙需重试 |
| 0x22 | `CNC` / `UDS_COND_NOT_CORRECT_22` | diagnosticIII.h / custom_diagnosticIII.c | 条件不满足(如车速过高) |
| 0x24 | `RSE` / `UDS_REQU_SEQU_ERROR_24` | diagnosticIII.h / custom_diagnosticIII.c | 请求序列错误 |
| 0x25 | `NRFSC` | diagnosticIII.h | 子网组件无响应 |
| 0x26 | `FPEORA` | diagnosticIII.h | 内部故障阻止执行 |
| 0x31 | `ROOR` / `UDS_REQUEST_OUT_OF_RANGE_31` | diagnosticIII.h / custom_diagnosticIII.c | 参数越界/无效DID |
| 0x33 | `SAD` / `UDS_DID_SEC_ERR_33` | diagnosticIII.h / custom_diagnosticIII.c | 安全访问拒绝 |
| 0x35 | `IK` | diagnosticIII.h | 安全密钥无效 |
| 0x36 | `ENOA` | diagnosticIII.h | 超出发送尝试次数 |
| 0x72 | `UDS_GENERAL_PROGRAMMING_FAIL_72` | custom_diagnosticIII.c | Flash编程失败 |
| 0x78 | `RCRRP` / `UDS_REQ_CORR_REC_RESP_PEND_78` | diagnosticIII.h / custom_diagnosticIII.c | 响应挂起 |
| 0x7E | `UDS_COND_NOT_SUPPORT_7E` | custom_diagnosticIII.c | 条件不支持 |
| 0x7F | `UDS_SERVICE_NOT_SUPPORTED_INACTIVE_SESSION_7F` | custom_diagnosticIII.c | 当前会话不支持 |

---

## 2. 会话类型定义（默认/扩展/编程）

**来源**: `lin.h` (定义) + `custom_diagnosticIII.c` (使用)

### 模板示例

```c
/** @defgroup UDS_SessionMode UDS 诊断会话类型
 *  @brief 诊断会话模式决定了ECU当前可用的服务集合和权限范围
 *  @note 上电后自动进入默认会话(0x01)，通过$10服务切换会话类型
 *  @{
 */

/** @brief 默认会话 - SESSION_MODE_DEFAULT (0x01)
 *  @scope 上电初始状态，支持基础诊断服务（$10会话控制、$14清除DTC、$19读取DTC等）
 *  @note 无诊断活动超时后自动从扩展会话回退至此模式 */
// #define SESSION_MODE_DEFAULT                0x01

/** @brief 编程会话 - SESSION_MODE_PROGRAM (0x02)
 *  @scope OTA固件升级模式，允许$27安全访问、$2E写入数据、$34请求下载、$36传输数据等
 *  @note 项目中用于: Bootloader DFU 固件更新流程
 *  @warning 进入编程会话后非易失性存储器写入将被启用 */
// #define SESSION_MODE_PROGRAM                0x02

/** @brief 扩展会话 - SESSION_MODE_EXTEND (0x03)
 *  @scope 扩展诊断模式，允许特殊功能（如IO控制、DID读取/写入等非标准诊断操作）
 *  @note 项目中用于: 门把手触摸功能IO控制、读取/写入配置参数
 *  @caution 进入扩展会话需满足车速条件（车速 < 0x36km/h 或车速信号无效） */
// #define SESSION_MODE_EXTEND                 0x03

/** @} */
```

### 会话模式位掩码 (Bootloader 权限矩阵使用)

```c
/** @brief 会话模式位掩码，用于ServiceUDS权限矩阵 */
// #define DEFALUT_SESSION          1  /**< bit0 默认会话 */
// #define PROGRAM_SESSION          2  /**< bit1 编程会话 */
// #define EXTEND_SESSION           4  /**< bit2 扩展会话 */
```

---

## 3. 安全访问等级定义

**来源**: `dfu_uds_manager.h`

### 模板示例

```c
/** @defgroup UDS_SecurityLevel UDS 安全等级定义
 *  @brief 安全等级逐级递增，高等级包含低等级权限
 *  @note 当前securityLevel必须 ≥ 服务要求的安全等级，请求才被允许
 *        等级通过$27服务完成种子密钥验证后提升
 *  @{
 */

/** @brief 安全等级0 - SECURITY_LEVEL0 (1)
 *  @permit 基础服务，如$10会话控制、$11ECU复位、$87链路控制
 *  @note 上电默认等级，无需安全访问解锁 */
// #define SECURITY_LEVEL0          1

/** @brief 安全等级1 - SECURITY_LEVEL1 (3)
 *  @permit 固件下载相关服务，如$2E固件信息同步、$31例程控制、$34请求下载、$36传输数据、$37传输退出
 *  @note 需要通过$27服务发送正确密钥后从等级0升级至此等级
 *  @warning 等级1允许修改非易失性存储器内容 */
// #define SECURITY_LEVEL1          3

/** @brief 安全等级2 - SECURITY_LEVEL2 (7)
 *  @permit 最高权限，预留未使用
 *  @note 当前项目中未启用，作为扩展预留 */
// #define SECURITY_LEVEL2          7

/** @} */
```

### 安全访问密钥 (项目中实际使用)

```c
/** @brief 安全访问密钥数组 (种子=密钥，简化实现)
 *  @note TCPL01X: {0x41, 0x31, 0x12, 0x01}
 *        TCPL03X (TCAE10X): {0x40, 0x31, 0x12, 0x02}
 *        TCPL03X (其他): {0x41, 0x31, 0x12, 0x02}
 *  @see $27 安全访问服务 */
// const uint8_t security_code[4] = { ... };
```

---

## 4. DID 数据标识符注释模板

**来源**: `custom_diagnosticIII.c`

### 项目实际 DID 列表

| DID | 名称 | 数据长度 | 描述 |
|-----|------|---------|------|
| 0xF187 | DID_PART_NUMBER | 12 bytes | 零件号 (Seres part num: 4280310-RW02) |
| 0xF18A | DID_SUPPLIER_CODE | 10 bytes | 供应商代码 (Seres: "3197") |
| 0xF197 | DID_ECU_NAME | 10 bytes | ECU名称 (如 "EHIS_FL" / "EHIS_RL" / "EHIS_FR" / "EHIS_RR") |
| 0xF189 | DID_SW_VERSION | 24 bytes | 软件版本号 (序列号格式: "SW:1.01.B_260525_3197_06") |
| 0xF089 | DID_APP_SW_VERSION | 21 bytes | 应用软件版本 (如 "SW:EHISFL.1.3C.05") |
| 0xF180 | DID_HARDWARE_VERSION | 8 bytes | 硬件版本号 |
| 0xF184 | DID_BOOT_VERSION | 8 bytes | Bootloader版本号 |
| 0xF190 | DID_DFU_FINGERPRINT | 10 bytes | DFU指纹 (OTA记录中的fingerprint) |
| 0x0216 | DID_VIN | - | 车辆识别码 (预留) |
| 0xF0FA | DID_ACTUATOR_TEST | - | 执行器测试 (预留) |
| 0x0001 | DID_PRODUCT_ID | - | 产品标识 (0xF3u) |

### DID 注释模板

```c
/** @defgroup UDS_DID UDS 数据标识符 (DID)
 *  @brief 通过$22(读取数据)和$2E(写入数据)服务访问的数据标识符
 *  @note DID为2字节(uint16_t)，高字节在前
 *  @{
 */

/** @brief DID 0xF187 - 零件号 (Part Number)
 *  @service $22 读取数据标识符
 *  @length 12 bytes (ASCII字符串)
 *  @example "4280310-RW02"
 *  @note 仅扩展会话(SESSION_MODE_EXTEND)下可读 */
// #define DID_PART_NUMBER                     0xF187

/** @brief DID 0xF18A - 供应商代码 (Supplier Code)
 *  @service $22 读取数据标识符
 *  @length 10 bytes (ASCII字符串)
 *  @example "3197" (右补空格)
 *  @note 仅扩展会话下可读 */
// #define DID_SUPPLIER_CODE                   0xF18A

/** @brief DID 0xF197 - ECU名称 (ECU Name)
 *  @service $22 读取数据标识符
 *  @length 10 bytes (ASCII字符串)
 *  @example "EHIS_FL" (前排左/右门分别标为FL/FR, 后排为RL/RR)
 *  @note 仅扩展会话下可读，标识安装位置 */
// #define DID_ECU_NAME                        0xF197

/** @brief DID 0xF189 - 软件版本 (Software Version / Sequence Number)
 *  @service $22 读取数据标识符
 *  @length 24 bytes (ASCII字符串)
 *  @example "SW:1.01.B_260525_3197_06"
 *  @note 存储在固定Flash地址(0x00004000)，包含生成日期和序列号 */
// #define DID_SW_VERSION                      0xF189

/** @brief DID 0xF089 - 应用软件版本 (Application Software Version)
 *  @service $22 读取数据标识符
 *  @length 21 bytes
 *  @example "SW:EHISFL.1.3C.05"
 *  @note 存储在固定Flash地址(0x00004018)，包含供应商和版本标识 */
// #define DID_APP_SW_VERSION                  0xF089

/** @brief DID 0xF180 - 硬件版本 (Hardware Version)
 *  @service $22 读取数据标识符
 *  @length 8 bytes
 *  @note 从Flash指定地址(FLASH_HW_VERSION_ADDR)读取 */
// #define DID_HARDWARE_VERSION                0xF180

/** @brief DID 0xF184 - Bootloader版本 (Bootloader Version)
 *  @service $22 读取数据标识符
 *  @length 8 bytes
 *  @note 从Flash指定地址(FLASH_BOOT_VERSION_ADDR)读取 */
// #define DID_BOOT_VERSION                    0xF184

/** @brief DID 0xF190 - DFU指纹 (DFU Fingerprint)
 *  @service $22 读取数据标识符
 *  @length 10 bytes
 *  @note 返回最近一次OTA固件升级记录中的fingerprint信息 */
// #define DID_DFU_FINGERPRINT                 0xF190

/** @} */
```

### DID 权限检查函数注释模板

```c
/** @brief 检查DID是否在支持的DID列表中
 *  @param  ucSess  要检查的DID值 (uint16_t)
 *  @return UDS_TRUE(0x01) - DID有效; UDS_FALSE(0x00) - DID不支持
 *  @note 如果DID不在以下列表中，返回NRC 0x31 (requestOutOfRange)
 *  支持列表: 0xF187, 0xF18A, 0xF197, 0xF189, 0xF089, 0xF180, 0xF184,
 *           0xF190, 0x0216, 0xF0FA, 0x0001
 *  @see $22 读取数据标识符服务 */
// STATIC uint8_t uds_diag_DID_chk(uint16_t ucSess);
```

---

## 5. SID 服务标识符列表

**来源**: `diagnosticIII.h` (函数声明) + `dfu_uds_manager.h` (宏定义) + `dfu_uds_manager.c` (权限矩阵)

### 完整 SID 表 (项目中使用)

| 服务ID | 宏名 | 服务名称 | 功能 | 源文件 |
|--------|------|---------|------|--------|
| $10 | `SERVICE_SESSION_CONTROL` | 诊断会话控制 | 切换默认/编程/扩展会话 | dfu_uds_manager.h |
| $11 | - | ECU复位 | 执行硬件复位或保持唤醒 | dfu_uds_manager.c |
| $22 | `SERVICE_READ_BY_IDENTIFY` | 读取数据标识符 | 通过DID读取ECU数据 | diagnosticIII.h + custom |
| $27 | `SERVICE_SECURITY_ACCESS` | 安全访问 | 种子-密钥验证，解锁安全等级 | dfu_uds_manager.h |
| $28 | - | 通信控制 | (项目中未启用) | dfu_uds_manager.c |
| $2E | `SERVICE_FIRMWARE_INFO_SYNC` | 写入数据(固件信息同步) | 接收固件地址/大小/CRC | dfu_uds_manager.h |
| $31 | `SERVICE_ROUTINE_CONTROL` | 例程控制 | CRC校验/Flash编程/系统复位 | dfu_uds_manager.h |
| $34 | `SERVICE_REQUEST_DOWNLOAD` | 请求下载 | 确认传输开始，准备接收数据 | dfu_uds_manager.h |
| $36 | `SERVICE_TRANSFER_DATA` | 传输数据 | 接收固件数据包，双缓冲队列管理 | dfu_uds_manager.h |
| $37 | `SERVICE_REQUEST_TRANSFER_EXIT` | 请求传输退出 | 结束数据传输阶段 | dfu_uds_manager.h |
| $87 | `SERVICE_LINK_CONTROL` | 链路控制 | 动态切换LIN波特率 | dfu_uds_manager.h |
| $A0 | - | 读取日志信息 | 通过LIN读取内部日志(条件编译) | diagnosticIII.c |

### SID 注释模板

```c
/** @defgroup UDS_SID UDS 服务标识符 (SID) 定义
 *  @brief 根据ISO 14229-1标准 + 项目自定义扩展
 *  @note 响应SID = 请求SID + 0x40（肯定响应时）
 *  @{
 */

/** @brief $10 - 诊断会话控制 (DiagnosticSessionControl)
 *  @func   切换ECU诊断会话模式: 默认(0x01) / 编程(0x02) / 扩展(0x03)
 *  @access 所有会话模式、物理/功能寻址、安全等级0
 *  @resp   [SID+0x40][sessionMode][P2_MSB][P2_LSB][P2E_MSB][P2E_LSB] */
// #define SERVICE_SESSION_CONTROL             0x10

/** @brief $11 - ECU复位 (ECUReset)
 *  @func   请求ECU执行硬件复位或保持唤醒
 *  @access 默认/编程会话、物理/功能寻址、安全等级0
 *  @subfunc 0x01 = 硬复位, 0x02 = 保持唤醒 */
// (项目中无独立宏定义，在ServiceUDS数组中直接使用0x11)

/** @brief $22 - 读取数据标识符 (ReadDataByIdentifier)
 *  @func   通过2字节DID读取ECU内部数据(零件号/版本号/供应商代码等)
 *  @access 扩展会话下可用
 *  @format  请求: [0x22][DID_MSB][DID_LSB]
 *          响应: [0x62][DID_MSB][DID_LSB][data...] */
// #define SERVICE_READ_BY_IDENTIFY            0x22

/** @brief $27 - 安全访问 (SecurityAccess)
 *  @func   安全种子-密钥验证，用于解锁更高安全等级
 *  @access 仅编程会话、物理寻址、安全等级0
 *  @subfunc 0x01 = 请求种子, 0x02 = 发送密钥 */
// #define SERVICE_SECURITY_ACCESS             0x27

/** @brief $2E - 写入数据/固件信息同步 (WriteDataByIdentifier)
 *  @func   接收OTA固件升级参数(目标地址/固件大小/CRC校验值)
 *  @access 仅编程会话、物理寻址、安全等级1
 *  @note   DFU流程起始服务，初始化dfu_ctx编程参数 */
// #define SERVICE_FIRMWARE_INFO_SYNC          0x2E

/** @brief $31 - 例程控制 (RoutineControl)
 *  @func   执行预设例程: 0xFF00擦除Flash / 0xFF01编程触发
 *  @access 仅编程会话、物理寻址、安全等级1
 *  @note   DFU关键服务，执行耗时操作时返回NRC 0x78 */
// #define SERVICE_ROUTINE_CONTROL             0x31

/** @brief $34 - 请求下载 (RequestDownload)
 *  @func   确认传输开始，Tester→ECU握手，准备接收固件数据
 *  @access 仅编程会话、物理寻址、安全等级1
 *  @note   此服务后进入数据传输阶段 */
// #define SERVICE_REQUEST_DOWNLOAD            0x34

/** @brief $36 - 传输数据 (TransferData)
 *  @func   接收固件数据包，每个包512字节+CRC32校验
 *  @access 仅编程会话、物理寻址、安全等级1
 *  @note   双缓冲乒乓操作，流水线处理"接收-编程" */
// #define SERVICE_TRANSFER_DATA               0x36

/** @brief $37 - 请求传输退出 (RequestTransferExit)
 *  @func   结束数据传输阶段，触发CRC验证和复位前准备
 *  @access 仅编程会话、物理寻址、安全等级1 */
// #define SERVICE_REQUEST_TRANSFER_EXIT       0x37

/** @brief $87 - 链路控制 (LinkControl)
 *  @func   动态切换LIN通信波特率: 9600/19200/115200
 *  @access 所有会话、物理/功能寻址、安全等级0
 *  @note   数据传输阶段切换到115200以加速OTA */
// #define SERVICE_LINK_CONTROL                0x87

/** @} */
```

### 自定义/私有SID

```c
/** @brief $B0 - 自定义: 读取日志信息
 *  @func   通过LIN接口读取ECU内部日志缓冲 (条件编译 LOG_INTERFACE_LIN)
 *  @note   仅在 LOG_INTERFACE_TYPE == LOG_INTERFACE_LIN 时启用 */
// (diagnosticIII.c 中 switch-case 直接使用 0xA0)
```

---

## 6. P2/P2E 超时参数说明

**来源**: `custom_diagnosticIII.c` (应用层) + `dfu_uds_manager.h` (Bootloader层)

### P2/P2E 定义

```c
/** @defgroup UDS_Timing UDS 诊断时序参数
 *  @brief P2和P2E超时决定ECU响应窗口和挂起行为
 *  @{
 */

/** @brief P2_Server_Max (500ms)
 *  @scope 应用层诊断服务响应超时
 *  @desc  ECU从收到请求到发送响应(或NRC 0x78)的最大允许时间
 *  @note  超过此时间Tester认为通信超时
 *  @unit  milliseconds */
// #define P2_SERVER_MAX                     (500)

/** @brief P2E_Server_Max (200ms)
 *  @scope 应用层诊断扩展响应超时(发出NRC 0x78后的间隔)
 *  @desc  ECU在发送NRC 0x78(响应挂起)后，发送下一个NRC 0x78或最终响应的间隔
 *  @note  用于长时间操作（如Flash擦除800ms、编程60ms）时周期性通知Tester
 *  @unit  milliseconds */
// #define P2E_SERVER_MAX                    (200)

/** @brief UDS诊断总超时 (3000ms) - Bootloader DFU使用
 *  @desc  从收到上一个UDS请求开始计时，3秒内无下一个UDS请求则退出DFU模式
 *  @note  超时后dfu_process_exit(DFU_MSG_TIMEOUT)，恢复到空闲状态
 *  @unit  milliseconds */
// #define LIN_UDS_TIMEOUT                   (3000)

/** @} */
```

### P2/P2E 在响应中的编码格式

```c
/** @brief 诊断会话控制($10)肯定响应格式 (6字节)
 *  字节0: [SID+0x40] = 0x50 (肯定响应)
 *  字节1: [sessionMode] = 当前激活的会话模式
 *  字节2-3: [P2_MSB][P2_LSB] = P2_Server_Max (500ms = 0x01F4)
 *  字节4-5: [P2E_MSB][P2E_LSB] = P2E_Server_Max (200ms = 0x00C8)
 */
// P2编码示例: diagnosticTxBuffer[2] = (uint8_t)(P2_SERVER_MAX >> 8);    // 0x01
//             diagnosticTxBuffer[3] = (uint8_t)P2_SERVER_MAX;            // 0xF4
// P2E编码示例: diagnosticTxBuffer[4] = (uint8_t)(P2E_SERVER_MAX >> 8);   // 0x00
//              diagnosticTxBuffer[5] = (uint8_t)P2E_SERVER_MAX;           // 0xC8
```

---

## 7. uds_request_info 结构体成员注释

**来源**: `dfu_uds_manager.h` (类型定义) + `dfu_uds_manager.c` (实例化)

### 结构体定义模板

```c
/** @struct ServiceUDS_TypeDef
 *  @brief  UDS 服务权限描述符
 *  @note   定义每个诊断服务所需的会话模式、寻址方式和安全等级
 *          用于 lin_diag_service_handle 中的三层权限校验矩阵
 */
typedef struct
{
    /** @brief 服务标识符 (SID)
     *  @range 0x00~0xFF
     *  @note  与UDS请求报文首字节匹配
     *  @example 0x10(会话控制), 0x27(安全访问), 0x31(例程控制) */
    uint8_t sid;

    /** @brief 允许的会话模式位掩码
     *  @bitfield 可组合: DEFALUT_SESSION(1) | PROGRAM_SESSION(2) | EXTEND_SESSION(4)
     *  @note     当前会话模式必须命中此掩码中的至少一个位
     *  @example  DEFALUT_SESSION | PROGRAM_SESSION (0x03) 表示默认或编程会话均可 */
    uint8_t sessionMode;

    /** @brief 允许的寻址模式位掩码
     *  @bitfield 可组合: PHYSICAL_ADDR(1) | FUNCTION_ADDR(2)
     *  @note     请求的寻址类型必须命中此掩码
     *  @example  PHYSICAL_ADDR (0x01) 表示仅接受物理寻址请求 */
    uint8_t requsetMode;

    /** @brief 所需的最低安全等级
     *  @range SECURITY_LEVEL0(1) < SECURITY_LEVEL1(3) < SECURITY_LEVEL2(7)
     *  @note   当前安全等级必须 ≥ 此值才能执行
     *  @warning 安全等级逐级包含: level2包含level1和level0的权限 */
    uint8_t securityLevel;

} ServiceUDS_TypeDef;
```

### 全局实例注释模板

```c
/** @brief 当前UDS请求会话信息实例
 *  @note  记录当前活动的会话模式、寻址类型和安全等级
 *         每次UDS请求时以此与ServiceUDS[]权限矩阵比较
 *  @field sessionMode   = 当前会话模式(初始DEFALUT_SESSION)
 *  @field requsetMode   = 当前请求的寻址模式(初始REQUEST_ID_ERROR=无效)
 *  @field securityLevel = 当前安全等级(初始SECURITY_LEVEL0=未解锁)
 *  @warning 会话控制($10)成功后更新sessionMode;
 *           安全访问($27)成功后更新securityLevel;
 *           每次收到请求时更新requsetMode */
// STATIC ServiceUDS_TypeDef uds_request_info =
// {
//     .sessionMode    = DEFALUT_SESSION,     /* 初始为默认会话 */
//     .requsetMode    = REQUEST_ID_ERROR,    /* 初始为无效寻址 */
//     .securityLevel  = SECURITY_LEVEL0      /* 初始安全等级0(未解锁) */
// };
```

### 服务权限矩阵表注释模板 (ServiceUDS[])

```c
/** @brief UDS 服务权限矩阵表
 *  @note  lin_diag_service_handle 中据此进行三层权限校验:
 *         1. 寻址模式检查 (requsetMode)
 *         2. 会话模式检查 (sessionMode)
 *         3. 安全等级检查 (securityLevel)
 *         三项全部通过才允许执行对应 handler
 *  @see   ServiceUDS_TypeDef
 */
// STATIC const ServiceUDS_TypeDef ServiceUDS[] =
// {
//     /* SID    sessionModeMask              requestModeMask          securityLevel   Description    */
//     { 0x11u,  DEFALUT_SESSION|PROGRAM_SESSION, PHYSICAL_ADDR|FUNCTION_ADDR, SECURITY_LEVEL0, }, /* $11 ECU复位 */
//     { 0x10u,  DEFALUT_SESSION|PROGRAM_SESSION|EXTEND_SESSION, PHYSICAL_ADDR|FUNCTION_ADDR, SECURITY_LEVEL0, }, /* $10 会话控制 */
//     { 0x87u,  DEFALUT_SESSION|PROGRAM_SESSION|EXTEND_SESSION, PHYSICAL_ADDR|FUNCTION_ADDR, SECURITY_LEVEL0, }, /* $87 链路控制 */
//     { 0x27u,  PROGRAM_SESSION,              PHYSICAL_ADDR,          SECURITY_LEVEL0, }, /* $27 安全访问 */
//     { 0x2Eu,  PROGRAM_SESSION,              PHYSICAL_ADDR,          SECURITY_LEVEL1, }, /* $2E 固件信息同步 */
//     { 0x31u,  PROGRAM_SESSION,              PHYSICAL_ADDR,          SECURITY_LEVEL1, }, /* $31 例程控制 */
//     { 0x34u,  PROGRAM_SESSION,              PHYSICAL_ADDR,          SECURITY_LEVEL1, }, /* $34 请求下载 */
//     { 0x36u,  PROGRAM_SESSION,              PHYSICAL_ADDR,          SECURITY_LEVEL1, }, /* $36 传输数据 */
//     { 0x37u,  PROGRAM_SESSION,              PHYSICAL_ADDR,          SECURITY_LEVEL1, }, /* $37 请求传输退出 */
// };
```

### dfu_process_context_t 调度表注释模板

```c
/** @struct dfu_process_context_t
 *  @brief  UDS 服务调度表条目 (SID→handler映射)
 *  @note   构成dfu_process_ctx[]调度表，lin_diag_service_handle根据SID索引到对应handler
 */
// typedef struct
// {
//     uint8_t sid;         /**< @brief 服务标识符，与UDS请求的SID匹配 */
//     dfu_func func;       /**< @brief 该SID对应的处理函数指针 (允许NULL，如ECU复位未实现) */
// } dfu_process_context_t;

/** @brief SID→Handler 调度表实例
 *  @note 服务分发根据此表查找匹配的SID并调用对应的handler
 */
// STATIC const dfu_process_context_t dfu_process_ctx[] =
// {
//     { SERVICE_SESSION_CONTROL,   session_control_handle   }, /* $10 - 会话控制 */
//     { SERVICE_ECU_RESET,         ecu_reset_handle         }, /* $11 - ECU复位 */
//     { SERVICE_SECURITY_ACCESS,   security_access_handle   }, /* $27 - 安全访问 */
//     { SERVICE_FIRMWARE_INFO_SYNC,firmware_sync_handle     }, /* $2E - 固件信息同步 */
//     { SERVICE_ROUTINE_CONTROL,   routine_control_handle   }, /* $31 - 例程控制 */
//     { SERVICE_REQUEST_DOWNLOAD,  request_download_handle  }, /* $34 - 请求下载 */
//     { SERVICE_TRANSFER_DATA,     transfer_data_handle     }, /* $36 - 传输数据 */
//     { SERVICE_REQUEST_TRANSFER_EXIT, transfer_exit_handle }, /* $37 - 请求传输退出 */
//     { SERVICE_LINK_CONTROL,      link_control_handle      }, /* $87 - 链路控制 */
// };
```

---

## 附录: 响应报文格式参考

### 肯定响应

```c
/** @brief 肯定响应报文格式
 *  格式: [SID+0x40][data...]
 *  @note 首字节 = 请求SID + 0x40
 *        后续字节 = 响应数据负载
 *  发送函数: lin_diag_positive_notify(sid, data, length)
 */
```

### 否定响应

```c
/** @brief 否定响应报文格式 (固定3字节)
 *  格式: [0x7F][SID][NRC]
 *  字节0: 0x7F (否定响应标识, NEGTIVE_ID)
 *  字节1: 请求的原始SID
 *  字节2: NRC错误码
 *  发送函数: lin_diag_negative_notify(sid, resp_value)
 */
// #define NEGTIVE_ID              0x7F
// #define UDS_NEG_RESP_RSID       0x7Fu
```

### 响应抑制位

```c
/** @brief 请求报文中SuppressPosRspMsgIndicationBit (bit7 of SID)
 *  @note 当SID字节的bit7=1时，ECU应抑制肯定响应，仅在异常时返回否定响应
 *  掩码: UDS_SID_MASK_WO_RESP_IND_BIT = 0x7F (去除bit7后获取真实SID)
 *  标志: UDS_SUPPRESS_POS_RESP_INDIC_BIT = 0x80
 */
```

---

## 附录: 关键源文件索引

| 文件路径 | 包含内容 |
|---------|---------|
| `app\midware\lin_manager\inc\diagnosticIII.h` | NRC宏定义, LIN诊断服务函数声明, DATA_DUMP/COMMAND定义 |
| `app\midware\lin_manager\diagnosticIII.c` | 诊断服务调度, 肯定/否定响应发送, NAD管理 |
| `boot\middleware\dfu_uds_manager\inc\dfu_uds_manager.h` | SID宏定义, 会话模式/安全等级/寻址模式, ServiceUDS_TypeDef, DFU结构体 |
| `boot\middleware\dfu_uds_manager\dfu_uds_manager.c` | 服务权限矩阵, uds_request_info实例, DFU流程实现, session_control_handle |
| `app\projects\TCAE10\Keil\customer\doorctrl_m9_duotaiji\Source\custom_diagnosticIII.c` | 会话控制实现, DID列表, NRC本地定义, P2/P2E超时, 自定义诊断服务 |
| `app\prebuild\lin_lib\inc\lin.h` | SESSION_MODE_DEFAULT/PROGRAM/EXTEND (0x01/0x02/0x03) |

---

> 编写日期: 2026-05-26
> 用途: UDS精确注释专项(Wave 5)参考模板
> 注意: 所有 `// #define` 和 `// STATIC` 注释均为模板示例，实际代码中需取消注释并适配。
