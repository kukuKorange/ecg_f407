/**
  ******************************************************************************
  * @file    beep.h
  * @brief   蜂鸣器驱动头文件 (STM32F407, PF5)
  ******************************************************************************
  */

#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx.h"

#define BEEP_PIN    GPIO_Pin_5
#define BEEP_PORT   GPIOF
#define BEEP_CLK    RCC_AHB1Periph_GPIOF

#define BEEP_ON     GPIO_SetBits(BEEP_PORT, BEEP_PIN)
#define BEEP_OFF    GPIO_ResetBits(BEEP_PORT, BEEP_PIN)
#define BEEP_TOGGLE BEEP_PORT->ODR ^= BEEP_PIN

void Beep_Init(void);
void Beep_Alarm(uint16_t duration_ms);

#endif
