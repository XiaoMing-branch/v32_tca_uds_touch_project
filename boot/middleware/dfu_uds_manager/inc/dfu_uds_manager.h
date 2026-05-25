/**
 *****************************************************************************
 * @brief   dfu_uds header file.
 *
 * @file    dfu_uds_manager.c
 * @author  AE/FAE team
 * @date    2024.01.01
 *****************************************************************************
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
 *****************************************************************************
 */

#ifndef __DFU_UDS_MANAGER_H__
#define __DFU_UDS_MANAGER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup UDS_SID UDS服务标识符(SID)定义
 * @brief 根据ISO 14229-1标准定义的UDS诊断服务ID，用于LIN总线OTA固件升级
 * @note 所有服务通过LIN UDS协议传输，响应SID = 请求SID + 0x40
 * @{
 */
#define SERVICE_SESSION_CONTROL             0x10      /**< @brief $10 诊断会话控制 - 切换默认/编程/扩展会话模式 */
#define SERVICE_ECU_RESET                   0x11      /**< @brief $11 ECU复位 - 执行硬件复位或保持唤醒 */
#define SERVICE_SECURITY_ACCESS             0x27      /**< @brief $27 安全访问 - 种子密钥验证，解锁安全级别 */
#define SERVICE_FIRMWARE_INFO_SYNC          0x2E      /**< @brief $2E 写入数据(固件信息同步) - 接收固件地址/大小/CRC */
#define SERVICE_ROUTINE_CONTROL             0x31      /**< @brief $31 例程控制 - CRC校验/Flash编程/系统复位 */
#define SERVICE_REQUEST_DOWNLOAD            0x34      /**< @brief $34 请求下载 - 确认传输开始，准备接收数据 */
#define SERVICE_TRANSFER_DATA               0x36      /**< @brief $36 传输数据 - 接收固件数据包，双缓冲队列管理 */
#define SERVICE_REQUEST_TRANSFER_EXIT       0x37      /**< @brief $37 请求传输退出 - 结束数据传输阶段 */
#define SERVICE_LINK_CONTROL                0x87      /**< @brief $87 链路控制 - 动态切换LIN波特率(9600/19200/115200) */
/** @} */

/**
 * @defgroup UDS_ADDR_MODE UDS寻址模式定义
 * @brief LIN总线诊断寻址类型，决定ECU是否响应请求
 * @{
 */
#define REQUEST_ID_ERROR         0           /**< @brief 无效寻址模式，用于初始化状态 */
#define PHYSICAL_ADDR            1           /**< @brief 物理寻址 - 点对点，仅目标NAD响应 */
#define FUNCTION_ADDR            2           /**< @brief 功能寻址 - 广播，总线上所有节点响应 */
/** @} */

/**
 * @defgroup UDS_SESSION_MODE UDS会话模式位掩码
 * @brief 用于ServiceUDS权限矩阵，按位掩码组合判断当前会话是否允许某服务
 * @{
 */
#define DEFALUT_SESSION          1           /**< @brief 默认会话(bit0) - 上电初始状态，仅支持基础服务 */
#define PROGRAM_SESSION          2           /**< @brief 编程会话(bit1) - OTA升级模式，支持固件下载相关服务 */
#define EXTEND_SESSION           4           /**< @brief 扩展会话(bit2) - 扩展诊断模式，支持特殊功能 */
/** @} */

/**
 * @defgroup UDS_SECURITY_LEVEL UDS安全等级定义
 * @brief 安全访问的层级逐级递增，高等级包含低等级权限
 * @note 在serviceUDS权限矩阵中，当前securityLevel必须 >= 服务要求的安全等级
 * @{
 */
#define SECURITY_LEVEL0          1           /**< @brief 安全等级0 - 无安全访问，允许基础服务(会话控制/链路控制/复位) */
#define SECURITY_LEVEL1          3           /**< @brief 安全等级1 - 解锁后级别，允许固件下载/编程/CRC校验 */
#define SECURITY_LEVEL2          7           /**< @brief 安全等级2 - 最高级别，预留 */
/** @} */

#if defined (__TCPL01X__)
#define CFG_SUPPORT_DFU_V3      (0)
#elif defined (__TCPL03X__)
#define CFG_SUPPORT_DFU_V3      (1)
#else
#define CFG_SUPPORT_DFU_V3      (1)
#endif

/**
 * @defgroup DFU_PROTOCOL DFU协议参数定义
 * @brief OTA固件升级过程中的时序、数据包结构等参数
 * @{
 */
#define DFU_INFO_MAGIC                  (0xDEADBEEFUL)  /**< @brief DFU信息块魔数，用于校验Flash中DFU记录的有效性 */
#define LIN_UDS_TIMEOUT                 (3000)          /**< @brief UDS诊断超时时间，单位ms(3秒)，OTA过程中无UDS请求则超时退出 */
#define DFU_ERASE_WAITTIME              (800)           /**< @brief Flash擦除等待时间，单位ms，通知Tester擦除耗时 */
#define DFU_PROGRAM_WAITTIME            (60)            /**< @brief Flash编程等待时间，单位ms，通知Tester单包编程耗时 */
#define DFU_PROGRAM_LENGTH              (512)           /**< @brief 单次编程数据长度(字节)，每个数据包负载大小 */
#define DFU_PACKET_HEAD_CRC_LENGTH      (6)             /**< @brief 数据包头部+CRC长度：SID(1B)+块索引(1B)+CRC32(4B) */
#define DFU_PACKET_BLOCK_LENGTH         (DFU_PROGRAM_LENGTH + DFU_PACKET_HEAD_CRC_LENGTH) /**< @brief 完整UDS数据帧长度 = 负载 + 头尾 */
#define DFU_PROGRAM_WORDS               (DFU_PROGRAM_LENGTH >> 2) /**< @brief 编程数据按32位字对齐的字数(512/4=128字) */
/** @} */

/**
 * @defgroup FLASH_LAYOUT Flash存储器布局定义
 * @brief 芯片内部Flash分区映射，Bootloader/DFU信息/应用程序/参数区
 * @note Flash总容量64KB(0x00000000 ~ 0x00010000)
 * @{
 */
#define FLASH_SECTOR_SIZE               (512)           /**< @brief Flash扇区大小(0.5KB)，擦除的最小单位 */
#define FLASH_BASE_ADDR                 (0x00000000UL)  /**< @brief Flash起始地址 */
#define FLASH_END_ADDR                  (0x00010000UL)  /**< @brief Flash结束地址(64KB) */

#if 1 == CFG_SUPPORT_DFU_V3
/**
 * @brief V3版本Flash布局(TC PL03X):
 *        0x0000 ~ 0x1DFF: Bootloader (7.5KB)
 *        0x1E00 ~ 0x1FFF: DFU信息区 (0.5KB)
 *        0x2000 ~ 0xF9FF: 应用程序区 (54.5KB)
 *        0xFA00 ~ 0xFFFF: 应用参数区 (1.5KB)
 */
#define FLASH_BOOT_SIZE                 (0x00001E00UL)  /**< @brief Bootloader固件大小(7.5KB)，位于Flash起始区域 */
#define FLASH_DFU_INFO_ADDR             (FLASH_BASE_ADDR + FLASH_BOOT_SIZE) /**< @brief DFU升级记录信息地址(0x1E00)，紧接Bootloader之后 */
#define FLASH_DFU_INFO_SIZE             (FLASH_SECTOR_SIZE) /**< @brief DFU信息区大小(0.5KB)，存储升级完成状态记录 */
#define FLASH_APP_ADDR                  (FLASH_DFU_INFO_ADDR + FLASH_DFU_INFO_SIZE) /**< @brief 应用程序固件起始地址(0x2000) */
#if defined (__TCPL01X__)
#define FLASH_APP_PARAM_SIZE            (0)             /**< @brief TCPL01X无独立参数区 */
#elif defined (__TCPL03X__)
#define FLASH_APP_PARAM_SIZE            (0x00000800UL)  /**< @brief TCPL03X应用参数区大小(2KB:0xFA00~0xFFFF) */
#endif
#else
/**
 * @brief V2版本Flash布局(TC PL01X):
 *        0x0000 ~ 0x2DFF: Bootloader (11.5KB)
 *        0x2E00 ~ 0x2FFF: DFU信息区 (0.5KB)
 *        0x3000 ~ 0xFFFF: 应用程序区 (52KB)
 */
#define FLASH_BOOT_SIZE                 (0x00002E00UL)  /**< @brief Bootloader固件大小(11.5KB) */
#define FLASH_DFU_INFO_ADDR             (FLASH_BASE_ADDR + FLASH_BOOT_SIZE) /**< @brief DFU信息区地址(0x2E00) */
#define FLASH_DFU_INFO_SIZE             (FLASH_SECTOR_SIZE) /**< @brief DFU信息区大小(0.5KB) */
#define FLASH_APP_ADDR                  (FLASH_DFU_INFO_ADDR + FLASH_DFU_INFO_SIZE) /**< @brief 应用程序起始地址(0x3000) */
#if defined (__TCPL01X__)
#define FLASH_APP_PARAM_SIZE            (0)
#elif defined (__TCPL03X__)
#define FLASH_APP_PARAM_SIZE            (0x00000800UL)
#endif
#endif
#define FLASH_APP_IMAGE_SIZE            (FLASH_END_ADDR - FLASH_BASE_ADDR - FLASH_APP_ADDR - FLASH_APP_PARAM_SIZE) /**< @brief 应用程序固件最大尺寸(V3约54KB/V2约52KB) */
#define FLASH_APP_END_ADDR              (FLASH_END_ADDR - FLASH_APP_PARAM_SIZE) /**< @brief 应用程序区结束地址(不含参数区) */
/** @} */

#define  QUEUE_LIN_LEN          (2)     /**< @brief LIN接收双缓冲队列深度，乒乓缓冲交替接收与编程 */

/**
 * @defgroup LIN_BAUDRATE LIN波特率标识定义
 * @brief 用于链路控制服务($87)切换波特率的枚举值
 * @{
 */
#define LIN_BRUAD_9600          (1)     /**< @brief LIN波特率9600bps标识 */
#define LIN_BRUAD_19200         (2)     /**< @brief LIN波特率19200bps标识(默认) */
#define LIN_BRUAD_115200        (5)     /**< @brief LIN波特率115200bps标识(高速传输用) */
/** @} */

/**
 * @enum dfu_msg_error_code_e
 * @brief DFU升级过程错误码枚举，记录OTA各阶段的执行结果
 * @note 该错误码贯穿整个升级流程：固件信息同步 → Flash擦除 → 请求下载 → 数据传输 → CRC校验 → 退出
 */
typedef enum
{
    DFU_MSG_SUCCESS = 0,                /**< @brief 操作成功，无错误 */
    DFU_MSG_ERROR,                      /**< @brief 通用错误 */
    DFU_MSG_ERASE_ERROR,                /**< @brief Flash擦除失败，pal_store_erase返回0 */
    DFU_MSG_SYNC_ERROR,                 /**< @brief 固件信息同步失败(参数越界或长度不足) */
    DFU_MSG_TRANFER_REQUEST_ERROR,      /**< @brief 请求下载时序错误(op_code非FLASH_ERASE) */
    DFU_MSG_TRANFER_EXIT_ERROR,         /**< @brief 传输退出错误 */
    DFU_MSG_SEQ_ERROR,                  /**< @brief 包序号错误 */
    DFU_MSG_PACKET_LEN_ERROR,           /**< @brief 数据包长度错误(≤DFU_PACKET_HEAD_CRC_LENGTH) */
    DFU_MSG_INDEX_ERROR,                /**< @brief 块索引不匹配(接收索引与期望索引不符) */
    DFU_MSG_PROGRA_ERROR,               /**< @brief Flash编程错误(地址越界或写后CRC校验失败) */
    DFU_MSG_CRC_ERROR,                  /**< @brief 整体CRC校验错误(计算的CRC与Tester下发的image_crc不匹配) */
    DFU_MSG_TIMEOUT,                    /**< @brief UDS诊断超时(3秒内无有效UDS请求) */
    DFU_MSG_MAX,                        /**< @brief 错误码最大值，用于边界检查 */
} dfu_msg_error_code_e;

/**
 * @enum dfu_cmd_e
 * @brief DFU操作流程状态机枚举，标识当前升级阶段
 * @note 升级流程：SYNC_INFO → FLASH_ERASE → TRANFER_START → [数据传输循环] → TRANFER_STOP
 */
typedef enum
{
    DFU_CMD_SYNC_INFO = 0,              /**< @brief 固件信息同步阶段，等待$2E服务下发固件参数 */
    DFU_CMD_FLASH_ERASE,                /**< @brief Flash擦除阶段，$31 0xFF00例程触发擦除操作 */
    DFU_CMD_TRANFER_START,              /**< @brief 数据传输开始，$34确认后进入接收数据状态 */
    DFU_CMD_TRANFER_STOP,               /**< @brief 数据传输结束，$37退出后进入CRC验证/复位阶段 */
    DFU_CMD_MAX,                        /**< @brief 状态最大值 */
} dfu_cmd_e;

/**
 * @enum boot_state_e
 * @brief Bootloader主循环状态枚举，决定上电后的行为
 * @note 上电后首先进入IDLE等待42ms窗口，之后根据DFU信息决定进入APP或UPGRADE
 */
typedef enum
{
    BOOT_STATE_IDLE = 0,                /**< @brief 空闲等待态，上电后42ms窗口期内等待UDS请求，超时后根据DFU记录决定下一步 */
    BOOT_STATE_USER_APP,                /**< @brief 跳转应用程序态，DFU记录有效时跳转至用户固件运行 */
    BOOT_STATE_UPGRADE,                 /**< @brief 固件升级态，DFU记录无效或收到编程会话请求时进入OTA升级模式 */
} boot_state_e;

/**
 * @struct packet_unit_t
 * @brief UDS数据包单元结构体(4字节对齐)，用于接收和暂存Tester下发的固件数据块
 * @note 每个packet单元承载DFU_PROGRAM_LENGTH(512)字节的固件数据，
 *       配合双缓冲队列实现"接收-编程"流水线操作
 */
typedef struct
{
    uint32_t head;                              /**< @brief 包头信息(预留)，当前未使用 */
    uint32_t data[DFU_PROGRAM_WORDS];           /**< @brief 固件数据负载(128个32位字 = 512字节)，对齐Flash编程粒度 */
    uint32_t crc32;                             /**< @brief 该数据块的CRC32校验值，编程后与Flash回读CRC比对验证 */
} __attribute__((aligned(4))) packet_unit_t;

/**
 * @struct queue_list_t
 * @brief LIN接收双缓冲队列结构体，实现乒乓缓冲机制
 * @note 队列深度为2(packet[0]和packet[1])，head指向待编程包，tail指向待填充包。
 *       当tail包填满后尾指针前移，主循环检测到队列非空时从head取包写入Flash。
 *       这种设计允许LIN接收与Flash编程并行工作，提高OTA传输效率。
 */
typedef struct
{
    packet_unit_t packet[QUEUE_LIN_LEN];        /**< @brief 数据包缓冲数组[2]，乒乓操作，一个接收一个写入 */
    uint8_t head;                               /**< @brief 读指针(出队)，指向待写入Flash的数据包 */
    uint8_t tail;                               /**< @brief 写指针(入队)，指向接收Tester数据的空闲缓冲区 */
} queue_list_t;

/**
 * @struct ServiceUDS_TypeDef
 * @brief UDS服务权限描述符，定义每个诊断服务所需的会话模式、寻址方式和安全等级
 * @note 用于lin_diag_service_handle中的权限检查矩阵，任何UDS请求都必须通过此处的权限校验
 */
typedef struct
{
    uint8_t sid;                /**< @brief 服务标识符(SID)，如0x10/0x27/0x31等 */
    uint8_t sessionMode;        /**< @brief 允许的会话模式位掩码(可组合：DEFALUT_SESSION|PROGRAM_SESSION|EXTEND_SESSION) */
    uint8_t requsetMode;        /**< @brief 允许的寻址模式位掩码(可组合：PHYSICAL_ADDR|FUNCTION_ADDR) */
    uint8_t securityLevel;      /**< @brief 所需的最低安全等级(SECURITY_LEVEL0/1/2)，当前等级必须 ≥ 此值 */
} ServiceUDS_TypeDef;

/**
 * @struct lin_baudrate_config_t
 * @brief LIN波特率配置结构体，用于链路控制服务($87)动态切换波特率
 * @note OTA升级时可通过$87服务将波特率从19200切换到115200以加速数据传输，
 *       编程完成后切换回19200恢复正常通信
 */
typedef struct
{
    uint8_t update_flag;            /**< @brief 波特率更新标志，置1表示需要在主循环中重新初始化LIN(设置后由lin_sci_baudrate_update清零) */
    uint32_t baudrate;              /**< @brief 目标波特率值，支持9600/19200/115200 */
} lin_baudrate_config_t;

/**
 * @struct last_dfu_info_t
 * @brief 上次DFU升级完成记录结构体，存储在Flash DFU信息区(FLASH_DFU_INFO_ADDR)
 * @note 该记录在升级成功时写入Flash，Bootloader上电时读取判断是否跳转APP。
 *       魔数(DFU_INFO_MAGIC=0xDEADBEEF)和reason=SUCCESS两者同时满足才认为升级有效。
 * @warning image_addr字段位置固定不可移动，JumpToApp函数直接通过固定偏移访问此字段
 */
typedef struct
{
    uint32_t magic;                         /**< @brief 魔数标记(0xDEADBEEF)，用于标识DFU记录有效性 */
    uint32_t image_size;                    /**< @brief 固件总大小(字节)，由$2E服务下发的image_size参数 */
    uint32_t image_crc;                     /**< @brief 固件整体CRC32校验值，由$2E服务下发，退出前与计算CRC比对 */
    uint32_t written_image_length;          /**< @brief 实际写入Flash的固件长度(已验证写入的字节数) */
    uint32_t written_image_crc;             /**< @brief 实际写入固件的CRC32值(从Flash回读计算) */
    uint32_t reason;                        /**< @brief DFU退出原因码，DFU_MSG_SUCCESS表示升级成功完成 */
    uint32_t version;                       /**< @brief Bootloader版本号(boot_version数组拷贝到此) */
    uint32_t image_addr;                    /* Waring Don't move */ /**< @brief 应用程序固件存放地址(FLASH_APP_ADDR)，JumpToApp通过此字段获取入口地址，不可移动位置 */
    uint32_t time[3];                       /**< @brief 升级完成时间戳(字符串"HH:MM:SS"拷贝到此) */
} last_dfu_info_t;

/**
 * @struct dfu_manager_context_t
 * @brief DFU管理器主上下文结构体，贯穿整个OTA升级流程的全局状态
 * @note 包含升级所需的全部运行时状态：目标地址/长度/CRC、接收进度、超时监控、
 *       LIN配置、DFU记录、双缓冲队列等。作为静态全局变量dfu_ctx存在。
 */
typedef struct
{
    /** @name 编程控制参数(由$2E服务初始化) */
    /** @{ */
    uint32_t write_addr;                    /**< @brief 当前写入Flash的目标地址(初始值为FLASH_APP_ADDR，每包递增DFU_PROGRAM_LENGTH) */
    uint32_t write_length;                  /**< @brief 已写入Flash的累计字节数 */
    uint32_t write_index;                   /**< @brief 已写入Flash的块索引(当前等于recevice_index) */
    uint32_t write_crc;                     /**< @brief 实时累计CRC32值，每写入一包后从Flash回读计算，初始值0xFFFFFFFF */
    /** @} */

    /** @name 接收控制参数(Tester下发数据的管理) */
    /** @{ */
    uint32_t receive_length;                /**< @brief 已接收的累计字节数(不含包头CRC) */
    uint32_t recevice_index;                /**< @brief 期望接收的下一个数据块索引(从1开始递增) */
    /** @} */

    /** @name 超时与状态控制 */
    /** @{ */
    uint32_t uds_timeout;                   /**< @brief UDS超时计数器(单位ms)，每次UDS请求后清零，超时阈值LIN_UDS_TIMEOUT(3000ms) */
    uint8_t op_code;                        /**< @brief 当前操作码(dfu_cmd_e枚举)，标识升级阶段：SYNC_INFO/FLASH_ERASE/TRANFER_START/STOP */
    uint8_t resp_value;                     /**< @brief 最近一次操作的结果码(dfu_msg_error_code_e值)，用于决定后续流程是否继续 */
    uint8_t program_flag;                   /**< @brief 编程进行中标志，$31 0xFF01例程中置1指示正在执行Flash写入 */
    uint8_t boot_state;                     /**< @brief Bootloader当前状态(boot_state_e枚举)：IDLE/USER_APP/UPGRADE */
    /** @} */

    /** @name 外设与存储配置 */
    /** @{ */
    lin_baudrate_config_t lin_config;       /**< @brief LIN波特率配置(动态切换，OTA高速传输后恢复) */
    last_dfu_info_t dfu_info;               /**< @brief DFU升级完成记录(上电读取决定跳转，升级成功写入) */
    queue_list_t queue_list;                /**< @brief LIN双缓冲接收队列(乒乓缓冲，深度2) */
    /** @} */
} dfu_manager_context_t;

/**
 * @struct dfu_process_context_t
 * @brief UDS服务调度表条目，将SID与对应的处理函数关联
 * @note 构成dfu_process_ctx[]调度表，lin_diag_service_handle根据SID索引到对应handler
 */
typedef void (*dfu_func)(uint8_t *, uint16_t);     /**< @brief UDS服务处理函数类型，参数为数据缓冲区和长度 */
typedef struct
{
    uint8_t sid;                    /**< @brief 服务标识符，与UDS请求的SID匹配 */
    dfu_func func;                  /**< @brief 该SID对应的处理函数指针(允许为NULL，如ECU复位未实现具体处理) */
} dfu_process_context_t;

/**
 * @brief Bootloader初始化入口，加载NAD配置并初始化LIN从机
 */
void dfu_manager_init(void);

/**
 * @brief LIN波特率动态更新，在主循环中检测标志并重新初始化LIN
 */
void lin_sci_baudrate_update(void);

/**
 * @brief UDS超时监控定时处理，3秒无服务请求则退出DFU模式
 */
void dfu_timeout_handle(void);

/**
 * @brief Bootloader主循环：UDS接收分发、42ms等待窗口、跳转APP或进入升级模式
 */
void main_loops(void);

#ifdef __cplusplus
}
#endif
#endif /* __DFU_UDS_MANAGER_H__ */
