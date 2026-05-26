
  .syntax unified
  .cpu cortex-m0
  .fpu softvfp
  .thumb

.global  g_pfnVectors
.global  Default_Handler

/* start address for the initialization values of the .data section.
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss

    .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
/* @brief 复位中断处理程序，系统上电或复位后首先执行
 *        执行流程：初始化堆栈指针 → 复制data段到SRAM → BSS段清零
 *        → 调用SystemInit系统初始化 → 调用C++静态构造函数
 *        → 跳转至主函数TcMain */
Reset_Handler:
   @ @brief 从链接器符号获取栈顶地址，初始化堆栈指针SP
   ldr   r0, =_estack
   mov   sp, r0          /* set stack pointer */

/* @brief 将.data段（已初始化全局变量）从Flash复制到SRAM运行区 */
  ldr r0, =_sdata      @ @brief data段起始地址（SRAM）
  ldr r1, =_edata      @ @brief data段结束地址（SRAM）
  ldr r2, =_sidata     @ @brief data段加载地址（Flash）
  movs r3, #0          @ @brief 偏移量初始为0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]     @ @brief 从Flash读取数据
  str r4, [r0, r3]     @ @brief 写入SRAM
  adds r3, r3, #4      @ @brief 偏移量+4

LoopCopyDataInit:
  adds r4, r0, r3      @ @brief 当前SRAM写入地址
  cmp r4, r1           @ @brief 比较是否到达段尾
  bcc CopyDataInit     @ @brief 未完成则继续复制

/* @brief BSS段（未初始化全局变量区）清零 */
  ldr r2, =_sbss       @ @brief BSS段起始地址
  ldr r4, =_ebss       @ @brief BSS段结束地址
  movs r3, #0          @ @brief 写入值0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]        @ @brief 写入0
  adds r2, r2, #4      @ @brief 地址+4

LoopFillZerobss:
  cmp r2, r4           @ @brief 比较是否到达段尾
  bcc FillZerobss      @ @brief 未完成则继续清零

/* @brief 调用SystemInit，配置系统时钟及相关外设初始化 */
  bl  SystemInit
/* @brief 调用C++静态构造函数（__libc_init_array） */
    bl __libc_init_array
/* @brief 跳转到主应用程序入口TcMain */
  bl  TcMain

LoopForever:
    b LoopForever


.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
/* @brief 默认中断处理程序（弱定义），所有未实现的中断默认跳转到此
 *        无限循环等待调试器介入，便于捕获未预期中断 */
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop    @ @brief 无限循环
  .size  Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex M0.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors


g_pfnVectors:
  .word  _estack        @ @brief 初始堆栈指针（栈顶地址）
  .word  Reset_Handler  @ @brief 复位中断，系统上电或复位后入口
  .word  NMI_Handler    @ @brief NMI不可屏蔽中断
  .word  HardFault_Handler @ @brief HardFault硬件错误异常
  .word  0              @ @brief 保留（未使用）
  .word  0              @ @brief 保留（未使用）
  .word  0              @ @brief 保留（未使用）
  .word  0              @ @brief 保留（未使用）
  .word  0              @ @brief 保留（未使用）
  .word  0              @ @brief 保留（未使用）
  .word  0              @ @brief 保留（未使用）
  .word  SVC_Handler    @ @brief SVC系统服务调用异常
  .word  0              @ @brief 保留（未使用）
  .word  0              @ @brief 保留（未使用）
  .word  PendSV_Handler @ @brief PendSV可挂起系统服务异常
  .word  SysTick_Handler @ @brief SysTick系统节拍定时器中断
  .word     FLASH_IRQHandler  @ @brief FLASH闪存控制器中断
  .word     ADC_IRQHandler    @ @brief ADC模数转换完成中断
  .word     PWM_IRQHandler    @ @brief PWM脉冲宽度调制中断
  .word     TIMER_IRQHandler  @ @brief TIMER通用定时器中断
  .word     IWDG_IRQHandler   @ @brief IWDG独立看门狗中断
  .word     SCI_IRQHandler    @ @brief SCI串行通信接口中断
  .word     AON_IRQHandler    @ @brief AON常开模块（Always-On）中断
  .word     GPIO_IRQHandler   @ @brief GPIO通用输入输出外部中断
  .word     SPI_IRQHandler    @ @brief SPI串行外设接口中断
  .word     RESERVED_T_IRQHandler @ @brief 保留中断T（未使用）
  .word     UART_IRQHandler   @ @brief UART通用异步收发器中断
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）
  .word     0              @ @brief 保留（未使用）

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler

   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler

   .weak      FLASH_IRQHandler
   .thumb_set FLASH_IRQHandler,Default_Handler

   .weak      ADC_IRQHandler
   .thumb_set ADC_IRQHandler,Default_Handler

   .weak      PWM_IRQHandler
   .thumb_set PWM_IRQHandler,Default_Handler

   .weak      TIMER_IRQHandler
   .thumb_set TIMER_IRQHandler,Default_Handler

   .weak      IWDG_IRQHandler
   .thumb_set IWDG_IRQHandler,Default_Handler

   .weak      SCI_IRQHandler
   .thumb_set SCI_IRQHandler,Default_Handler

   .weak      AON_IRQHandler
   .thumb_set AON_IRQHandler,Default_Handler

   .weak      GPIO_IRQHandler
   .thumb_set GPIO_IRQHandler,Default_Handler

   .weak      SPI_IRQHandler
   .thumb_set SPI_IRQHandler,Default_Handler

   .weak      RESERVED_T_IRQHandler
   .thumb_set RESERVED_T_IRQHandler,Default_Handler

   .weak      UART_IRQHandler
   .thumb_set UART_IRQHandler,Default_Handler

