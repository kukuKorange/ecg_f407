/**
  ******************************************************************************
  * @file    bsp_led.h
  * @brief   RGB LED驱动头文件 (STM32F407)
  * @note    LED_R=PE2, LED_G=PE3, LED_B=PE4 (低电平点亮)
  ******************************************************************************
  */

#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stm32f4xx.h"
#include "kconfig.h"

#define LED1_PIN    GPIO_Pin_2
#define LED1_PORT   GPIOE
#define LED2_PIN    GPIO_Pin_3
#define LED2_PORT   GPIOE
#define LED3_PIN    GPIO_Pin_4
#define LED3_PORT   GPIOE

#define LED1_ON     GPIO_ResetBits(LED1_PORT, LED1_PIN)
#define LED1_OFF    GPIO_SetBits(LED1_PORT, LED1_PIN)
#define LED1_TOGGLE LED1_PORT->ODR ^= LED1_PIN

#define LED2_ON     GPIO_ResetBits(LED2_PORT, LED2_PIN)
#define LED2_OFF    GPIO_SetBits(LED2_PORT, LED2_PIN)
#define LED2_TOGGLE LED2_PORT->ODR ^= LED2_PIN

#define LED3_ON     GPIO_ResetBits(LED3_PORT, LED3_PIN)
#define LED3_OFF    GPIO_SetBits(LED3_PORT, LED3_PIN)
#define LED3_TOGGLE LED3_PORT->ODR ^= LED3_PIN

#define LED_RGBOFF  do { LED1_OFF; LED2_OFF; LED3_OFF; } while(0)

void LED_RGB_Config(void);

#ifdef ENABLE_LED_INDICATOR
void LED_StatusUpdate(void);
#endif

#endif
