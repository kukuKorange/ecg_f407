/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   中断服务函数
  * @note    TIM3和USART2中断在各自驱动文件中实现(timer.c, usart2.c)
  ******************************************************************************
  */

#include "stm32f4xx_it.h"

void NMI_Handler(void) {}

void HardFault_Handler(void)
{
    while (1);
}

void MemManage_Handler(void)
{
    while (1);
}

void BusFault_Handler(void)
{
    while (1);
}

void UsageFault_Handler(void)
{
    while (1);
}

void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}

extern void TimingDelay_Decrement(void);

void SysTick_Handler(void)
{
    TimingDelay_Decrement();
}
