;
;*****************************************************************************
; @brief   startup_tcpl01x.s  file.
;
; @file    startup_tcpl01x.s
; @author  AE/FAE team
; @date    2024.01.01
;******************************************************************************
;
; THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
; WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
; TIME. AS A RESULT, TINYCHIP SHALL NOT BE HELD LIABLE FOR ANY
; DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
; FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
; CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
;
; <b>&copy; Copyright (c) 2024 Tinychip Microelectronics Co.,Ltd.</b>
;
; *****************************************************************************

; Amount of memory (in bytes) allocated for Stack
; Tailor this value to your application needs
; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>
    IF	:DEF:STACK_SIZE
Stack_Size      EQU     STACK_SIZE
    ELSE
Stack_Size      EQU     0x400
    ENDIF

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size      EQU     0x00

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp                   ; @brief 初始堆栈指针（栈顶地址）
                DCD     Reset_Handler             ; @brief 复位中断，系统上电或复位后入口
                DCD     NMI_Handler               ; @brief NMI不可屏蔽中断
                DCD     HardFault_Handler         ; @brief HardFault硬件错误异常
                DCD     0                         ; @brief 保留（未使用）
                DCD     0                         ; @brief 保留（未使用）
                DCD     0                         ; @brief 保留（未使用）
                DCD     0                         ; @brief 保留（未使用）
                DCD     0                         ; @brief 保留（未使用）
                DCD     0                         ; @brief 保留（未使用）
                DCD     0                         ; @brief 保留（未使用）
                DCD     SVC_Handler               ; @brief SVC系统服务调用异常
                DCD     0                         ; @brief 保留（未使用）
                DCD     0                         ; @brief 保留（未使用）
                DCD     PendSV_Handler            ; @brief PendSV可挂起系统服务异常
                DCD     SysTick_Handler           ; @brief SysTick系统节拍定时器中断
                ; @brief 外部中断向量表
                DCD     FLASH_IRQHandler          ; @brief FLASH闪存控制器中断
                DCD     ADC_IRQHandler            ; @brief ADC模数转换完成中断
                DCD     PWM_IRQHandler            ; @brief PWM脉冲宽度调制中断
                DCD     TIMER_IRQHandler          ; @brief TIMER通用定时器中断
                DCD     IWDG_IRQHandler           ; @brief IWDG独立看门狗中断
                DCD     SCI_IRQHandler            ; @brief SCI串行通信接口中断
                DCD     AON_IRQHandler            ; @brief AON常开模块（Always-On）中断
                DCD     GPIO_IRQHandler           ; @brief GPIO通用输入输出外部中断
                DCD     RESERVED_S_IRQHandler     ; @brief 保留中断S（未使用）
                DCD     RESERVED_T_IRQHandler     ; @brief 保留中断T（未使用）
                DCD     UART_IRQHandler           ; @brief UART通用异步收发器中断
                SPACE    ( 21 * 4)                ; @brief 预留中断11~31空间（共21项）

__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; @brief 复位中断处理程序，系统上电或复位后首先执行
;        执行流程：关闭中断 → 关闭测试模式 → BSS段清零
;        → 调用SystemInit系统初始化 → 跳转至主函数TcMain
Reset_Handler    PROC
                 EXPORT  Reset_Handler                 [WEAK]
                 IMPORT  TcMain
                 IMPORT  SystemInit
                 IMPORT  |Image$$RW_IRAM1$$ZI$$Base|
                 IMPORT  |Image$$RW_IRAM1$$ZI$$Limit|
                 ; @brief 关闭全局中断（PRIMASK置位），确保初始化过程不被中断干扰
                 CPSID   I
                 ; @brief 写入解锁序列，关闭芯片测试模式（Test Mode）
                 LDR     R0, =0x4000F000
                 LDR     R1, =0xFEDCBA98
                 STR     R1, [R0]
                 ; @brief BSS段（未初始化全局变量区）清零，从ZI Base到ZI Limit逐字写入0
                 MOVS    R0, #0
                 LDR     R1, =|Image$$RW_IRAM1$$ZI$$Base|
                 LDR     R2, =|Image$$RW_IRAM1$$ZI$$Limit|
FillZero
                 STR     R0, [R1, #0]
                 ADDS    R1, R1, #4
                 CMP     R1, R2
                 BCC     FillZero

                 ; @brief 调用SystemInit，配置系统时钟及相关外设初始化
                 LDR     R0, =SystemInit
                 BLX     R0
                 ; @brief 跳转到主应用程序入口TcMain，不再返回
                 LDR     R0, =TcMain
                 BX      R0
                 ENDP

; @brief 默认异常处理程序（弱定义，用户可重新实现覆盖）
;        均为无限循环，便于调试器捕获异常现场

; @brief NMI不可屏蔽中断处理程序，发生不可屏蔽中断时进入此循环
NMI_Handler     PROC
                EXPORT  NMI_Handler                    [WEAK]
                B       .
                ENDP
; @brief HardFault硬件错误处理程序，发生硬件错误（如总线错误、用法错误等）时进入此循环
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler              [WEAK]
                B       .
                ENDP
; @brief SVC系统服务调用处理程序，执行SVC指令触发
SVC_Handler     PROC
                EXPORT  SVC_Handler                    [WEAK]
                B       .
                ENDP
; @brief PendSV可挂起系统服务调用处理程序，用于上下文切换（RTOS使用）
PendSV_Handler  PROC
                EXPORT  PendSV_Handler                 [WEAK]
                B       .
                ENDP
; @brief SysTick系统节拍定时器中断处理程序，定时溢出触发
SysTick_Handler PROC
                EXPORT  SysTick_Handler                [WEAK]
                B       .
                ENDP
; @brief 默认中断处理程序（弱定义），所有外部中断的默认入口
;        用户需在应用程序中重新实现各中断处理函数以覆盖此弱定义
Default_Handler PROC
                EXPORT  FLASH_IRQHandler               [WEAK]
                EXPORT  ADC_IRQHandler                  [WEAK]
                EXPORT  PWM_IRQHandler                  [WEAK]
                EXPORT  TIMER_IRQHandler                [WEAK]
                EXPORT  IWDG_IRQHandler                 [WEAK]
                EXPORT  SCI_IRQHandler                  [WEAK]
                EXPORT  AON_IRQHandler                  [WEAK]
                EXPORT  GPIO_IRQHandler                 [WEAK]
                EXPORT  RESERVED_S_IRQHandler           [WEAK]
                EXPORT  RESERVED_T_IRQHandler           [WEAK]
                EXPORT  UART_IRQHandler                 [WEAK]

FLASH_IRQHandler
ADC_IRQHandler
PWM_IRQHandler
TIMER_IRQHandler
IWDG_IRQHandler
SCI_IRQHandler
AON_IRQHandler
GPIO_IRQHandler
RESERVED_S_IRQHandler
RESERVED_T_IRQHandler
UART_IRQHandler
                ; @brief 默认中断处理入口，无限循环等待调试器介入
                B       .
                ENDP

                ALIGN

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                 IF      :DEF:__MICROLIB

                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit

                 ELSE

                 IMPORT  __use_two_region_memory
                 EXPORT  __user_initial_stackheap

__user_initial_stackheap

                 LDR     R0, =  Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

                 ENDIF

                 END

;************************ (C) COPYRIGHT Tinychip Microelectronics Co.,Ltd*******
