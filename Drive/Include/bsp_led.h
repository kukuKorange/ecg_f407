/**
  ******************************************************************************
  * @file    bsp_led.h
  * @brief   LED驱动头文件 (STM32F407)
  * @note    LED = PF4 (低电平点亮)
  ******************************************************************************
  */

#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stm32f4xx.h"
#include "kconfig.h"

#define ECG_LED_PIN    GPIO_Pin_4
#define ECG_LED_PORT   GPIOF
#define ECG_LED_CLK    RCC_AHB1Periph_GPIOF

#define ECG_LED_ON     GPIO_ResetBits(ECG_LED_PORT, ECG_LED_PIN)
#define ECG_LED_OFF    GPIO_SetBits(ECG_LED_PORT, ECG_LED_PIN)
#define ECG_LED_TOGGLE ECG_LED_PORT->ODR ^= ECG_LED_PIN

void LED_RGB_Config(void);

#ifdef ENABLE_LED_INDICATOR
void LED_StatusUpdate(void);
#endif

#endif
