/**
  ******************************************************************************
  * @file    timer.h
  * @brief   TIM3定时器驱动头文件
  ******************************************************************************
  */

#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

extern volatile uint32_t tim3_ms_counter;

void Timer3_Init(void);

#endif
